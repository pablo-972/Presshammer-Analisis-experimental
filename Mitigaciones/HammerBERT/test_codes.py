# --------- Insecure codes --------- #

CODE_1 = """
for (int j = 0; j < x; j++) {
    *(aggrArr[j]);
    asm volatile("clflush (%0)" : : "r" (aggrArr[j]) : "memory");
}
"""


CODE_2 = """
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
"""


CODE_3 = """
asm volatile("lfence");

for (int i = 0; i < numReads; i++) {
    *(aggrArr[i]);
}

for (int i = 0; i < numReads; i++) {
    asm volatile("clflush (%0)" : : "r" (aggrArr[i]) : "memory");
}

std::cout << "End of flushing process." << std::endl;
"""


CODE_4 = """
asm volatile("lfence");

for (int i = 0; i < numReads; i++) {
    *(aggrArr[i]);
}

asm volatile("clflushopt (%0)" : : "r" (aggrArr) : "memory");
"""


# --------- Secure codes --------- #

CODE_5 = """
const int size = 100;
        
for (int i = 0; i < size; i++){
    arr[i] = i%2;
}

for (int j = 0; j < size; j++){
    std::cout << "Elemento: " << arr[j] << std::endl;
}
        
"""


CODE_6 = """
const int size = 50;
std::vector<int> data(size);

for (int i = 0; i < size; ++i) {
    data[i] = i * 10;
}

std::sort(data.begin(), data.end());

for (const auto& val : data) {
    std::cout << val << std::endl;
}
"""


CODE_7 = """
const int x = 100;
int total = 0;

for (int i = 0; i < x; ++i) {
    total += i;
}

std::cout << "Total: " << total << std::endl;
"""