#include "taggon.h"


constexpr int kNumOfRows = (1ULL << 30)/8192; // total number of rows
constexpr int kDistance = 30; // distance between aggr rows
constexpr int iterations = 100000;
std::mt19937 rng; // random variable


uintptr_t generateRandomAddress(uintptr_t targetAddress, std::uniform_int_distribution<unsigned> &dist){

    int rowOffset = dist(rng);
    return targetAddress + rowOffset * (8192);
}


void generateAggr(Mapping &aggr1, Mapping &aggr2, uintptr_t randomAddress){
    // initializes aggr1 row
    aggr1.decodeNewAddress(randomAddress);
    aggr1.resetColumn();

    // initializes aggr2 row far away from aggr1
    aggr2.decodeNewAddress(randomAddress);
    aggr2.resetColumn();

    for (int i = 0; i < kDistance; i++)
        aggr2.incrementRow();
}


void generateTemp(Mapping &temp, uintptr_t randomAddress){
    temp.decodeNewAddress(randomAddress);
    temp.resetColumn();

    for (int i = 0; i < kDistance/2; i++)
        temp.incrementRow();
}

void cleanLatencyRecord(int latencyValuesAggr1[], int latencyValuesAggr2[]){

    for (int i = 0 ; i < 128 ; i++){
        latencyValuesAggr1[i] = 0;
        latencyValuesAggr2[i] = 0;
    }
}


void saveAggrVirtualAddress(volatile unsigned long long *aggrArr1[], volatile unsigned long long *aggrArr2[], Mapping &aggr1, Mapping &aggr2){
    for (int j = 0 ; j < 128 ; j++){
        aggrArr1[j] = (volatile unsigned long long *) (aggr1.toVirt());
        aggrArr2[j] = (volatile unsigned long long *) (aggr2.toVirt());
        
        aggr1.incrementColumnCb();
        aggr2.incrementColumnCb();
    }
    
    // flush cache    
    for (int i = 0 ; i < 128 ; i++)
        asm volatile("clflushopt (%0)" : : "r" (aggrArr1[i]) : "memory"); 
    for (int i = 0 ; i < 128 ; i++)
        asm volatile("clflushopt (%0)" : : "r" (aggrArr2[i]) : "memory");
}

void saveTimeRecord(std::ofstream &latencyFile, int latencyValuesAggr1[], int latencyValuesAggr2[]){
        // the latencies of the aggressor rows are recorded
        //aggr1
        latencyFile << "aggr1 ";
        for (int i = 0 ; i < 128 ; i++)
            latencyFile << std::dec << latencyValuesAggr1[i] << " ";
        
        latencyFile << std::endl;

        // aggr2
        latencyFile << "aggr2 ";
        for (int i = 0 ; i < 128 ; i++)
            latencyFile << std::dec << latencyValuesAggr2[i] << " ";
        
        latencyFile << std::endl;

        asm volatile("mfence");
}

void accessTimeClock(volatile unsigned long long *aggrArr1[], volatile unsigned long long *aggrArr2[], volatile unsigned long long *tempAddr, std::ofstream &latencyFile, int latencyValuesAggr1[], int latencyValuesAggr2[]){
    // for record time
    struct timespec tglob1, tglob2; // clock time

    for (int i = 0; i < iterations; i++){
        // measurement for aggressor row 1
        for (int j = 0; j < 128; j++){

            asm volatile("lfence");
            clock_gettime(CLOCK_REALTIME, &tglob1); // gets the initial time
            *(aggrArr1[j]); // access to the aggressor row 1
            asm volatile("lfence"); // gets the finish time

            clock_gettime(CLOCK_REALTIME, &tglob2); // gets the finish time
            latencyValuesAggr1[j] = tglob2.tv_nsec - tglob1.tv_nsec; // saves the access time for row 1
            asm volatile("mfence");
        }

        *(tempAddr); // access the middle row to stabilize the system
        asm volatile("clflushopt (%0)" : : "r" (tempAddr) : "memory");
        asm volatile("mfence");

        // measurement for aggressor row 2
        for (int j = 0; j < 128; j++){

            asm volatile("lfence");
            clock_gettime(CLOCK_REALTIME, &tglob1); // gets the initial time

            *(aggrArr2[j]); // access to the aggressor row 1
            asm volatile("lfence"); 

            clock_gettime(CLOCK_REALTIME, &tglob2); // gets the finish time
            latencyValuesAggr2[j] = tglob2.tv_nsec - tglob1.tv_nsec; // saves the access time for row 2
            asm volatile("mfence");
        }

        // cache is flushed again to force reads to DRAM and not to cache
        for (int i = 0; i < 128; i++)
            asm volatile("clflushopt (%0)" :: "r"(aggrArr1[i]) : "memory");
    
        for (int i = 0; i < 128; i++)
            asm volatile("clflushopt (%0)" :: "r"(aggrArr2[i]) : "memory");

        asm volatile ("mfence");

        saveTimeRecord(latencyFile, latencyValuesAggr1, latencyValuesAggr2);
    }

}


void accessCycles(volatile unsigned long long *aggrArr1[], volatile unsigned long long *aggrArr2[], volatile unsigned long long *tempAddr, std::ofstream &latencyFile, int latencyValuesAggr1[], int latencyValuesAggr2[]){
    // for record time
    unsigned cycles_low, cycles_high, cycles_low1, cycles_high1; // # of cycles

    for (int i = 0; i < iterations; i++){
        // measurement for aggressor row 1
        for (int j = 0; j < 128; j++){

            asm volatile ("lfence");
            asm volatile (
                "RDTSCP\n\t" 
                "mov %%edx, %0\n\t" 
                "mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low):: 
                "%rax", "%rbx", "%rcx", "%rdx"); // gets initial cycles

            *(aggrArr1[j]); // access to the aggressor row 1
            asm volatile("lfence");

            asm volatile (
                "RDTSCP\n\t" 
                "mov %%edx, %0\n\t" 
                "mov %%eax, %1\n\t": "=r" (cycles_high1), "=r" (cycles_low1):: 
                "%rax", "%rbx", "%rcx", "%rdx"); // gets final cycles

            latencyValuesAggr1[j] = cycles_low1 - cycles_low;
            asm volatile("mfence");
        }

        *(tempAddr); // access the middle row to stabilize the system
        asm volatile("clflushopt (%0)" : : "r" (tempAddr) : "memory");
        asm volatile("mfence");

        // measurement for aggressor row 2
        for (int j = 0; j < 128; j++){

            asm volatile ("lfence");
            asm volatile (
                "RDTSCP\n\t" 
                "mov %%edx, %0\n\t" 
                "mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low):: 
                "%rax", "%rbx", "%rcx", "%rdx"); // gets initial cycles

            *(aggrArr2[j]); // access to the aggressor row 2
            asm volatile("lfence");

            asm volatile (
                "RDTSCP\n\t" 
                "mov %%edx, %0\n\t" 
                "mov %%eax, %1\n\t": "=r" (cycles_high1), "=r" (cycles_low1):: 
                "%rax", "%rbx", "%rcx", "%rdx"); // gets final cycles

            latencyValuesAggr2[j] = cycles_low1 - cycles_low;
            asm volatile("mfence");
        }

        // cache is flushed again to force reads to DRAM and not to cache
        for (int i = 0; i < 128; i++)
            asm volatile("clflushopt (%0)" :: "r"(aggrArr1[i]) : "memory");
        
        for (int i = 0; i < 128; i++)
            asm volatile("clflushopt (%0)" :: "r"(aggrArr2[i]) : "memory");
    
        asm volatile ("mfence");

        saveTimeRecord(latencyFile, latencyValuesAggr1, latencyValuesAggr2);
    }
}


__attribute__((optimize("unroll-loops")))
void verifytAggOn(uintptr_t targetAddress){

    // open latency.txt for writing latency records
    std::ofstream latencyFile;
    latencyFile.open("latency.txt");
    
    // addressinng for aggressor and temp rows
    Mapping::baseAddress = targetAddress;
    Mapping aggr1;
    Mapping aggr2;
    Mapping temp;
    
    // initialize mt PRNG
    rng.seed(1337);
    std::uniform_int_distribution<unsigned> dist(0,kNumOfRows);

    // generate a random address
    uintptr_t randomAddress = generateRandomAddress(targetAddress, dist);

    // generate two row addresses far away from each other (aggr1 and aggr2)
    generateAggr(aggr1, aggr2, targetAddress);
    
    // access temp once between aggr1 and aggr2 to try to bring
    // system state back to the initial state
    generateTemp(temp, targetAddress);


    // 2 aggressors, "numAggrActs" cache blocks accessed each & 1 temp row
    volatile unsigned long long *aggrArr1[128];
    volatile unsigned long long *aggrArr2[128];
    // save aggressors virtual addresses 
    saveAggrVirtualAddress(aggrArr1, aggrArr2, aggr1, aggr2);

    volatile unsigned long long *tempAddr;
    tempAddr = (volatile unsigned long long *) (temp.toVirt());
    
    // prepare arrays to save latency records
    int latencyValuesAggr1[128] = {0};
    int latencyValuesAggr2[128] = {0};
    
    cleanLatencyRecord(latencyValuesAggr1, latencyValuesAggr2);

    // we test the access time using the system clock and average # of cycles:

    // 1. test access time with system clock (clock_gettime(CLOCK_REALTIME, &time))
    accessTimeClock(aggrArr1, aggrArr2, tempAddr, latencyFile, latencyValuesAggr1, latencyValuesAggr2);
    cleanLatencyRecord(latencyValuesAggr1, latencyValuesAggr2);

    // 2. test access time with average # of cycles
    accessCycles(aggrArr1, aggrArr2, tempAddr, latencyFile, latencyValuesAggr1, latencyValuesAggr2);
    
    
    // file is closed
    latencyFile.close();


}