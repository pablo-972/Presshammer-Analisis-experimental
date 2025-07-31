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


CODE_5 = """
for (int i = 0; i < 100000; i++) {
    *(aggrRow1);
    *(aggrRow2);
    asm volatile("clflush (%0)" : : "r"(aggrRow1) : "memory");
    asm volatile("clflush (%0)" : : "r"(aggrRow2) : "memory");
}
"""

CODE_6 = """
while (true) {
    for (int i = 0; i < N; i++) {
        *(aggrArr[i]);
        asm volatile("clflushopt (%0)" : : "r" (aggrArr[i]) : "memory");
    }
}
"""


CODE_7 = """
for (int i = 0; i < iterations; i++) {
    *(victim);
    asm volatile("clflush (%0)" : : "r" (victim) : "memory");
}
"""


CODE_8 = """
for (int i = 0; i < 1e5; ++i) {
    *(aggrArr1[i]);
    *(aggrArr2[i]);
    asm volatile("mfence");
    asm volatile("clflushopt (%0)" : : "r"(aggrArr1[i]) : "memory");
    asm volatile("clflushopt (%0)" : : "r"(aggrArr2[i]) : "memory");
}
"""


CODE_9 = """
for (int r = 0; r < rounds; r++) {
    flush_cache_line((void*)target);
    *(volatile char*)target;
}
"""


CODE_10 = """
for (int i = 0; i < loop_limit; i++) {
    asm volatile("mfence");
    *(agg1);
    *(agg2);
    asm volatile("clflush (%0)" : : "r"(agg1) : "memory");
    asm volatile("clflush (%0)" : : "r"(agg2) : "memory");
}
"""


CODE_11 = """
while (condition) {
    *(aggressorA);
    *(aggressorB);
    asm volatile("clflush (%0)" : : "r"(aggressorA));
    asm volatile("clflush (%0)" : : "r"(aggressorB));
}
"""

CODE_12 = """
for (int i = 0; i < 1000; ++i) {
    asm volatile("clflush (%0)" : : "r" (buffer[i % 8]) : "memory");
    benchmark_results[i] = buffer[i % 8];
}
"""

CODE_13 = """
for (int i = 0; i < 100; ++i) {
    buffer[i % 10]++;
    for (int j = 0; j < 100; j++){
        *(buffer[i % 10]);
    }
}
asm volatile("clflush (%0)" : : "r" (buffer[1]) : "memory")
asm volatile("clflush (%0)" : : "r" (buffer[0]) : "memory")
"""




# --------- Secure codes --------- #

CODE_14 = """
const int size = 100;
        
for (int i = 0; i < size; i++){
    arr[i] = i%2;
}

for (int j = 0; j < size; j++){
    std::cout << "Elemento: " << arr[j] << std::endl;
}
        
"""


CODE_15 = """
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


CODE_16 = """
const int x = 100;
int total = 0;

for (int i = 0; i < x; ++i) {
    total += i;
}

std::cout << "Total: " << total << std::endl;
"""

CODE_17 = """
int arr[100];
for (int i = 0; i < 100; i++) {
    arr[i] = i * 2;
}
std::cout << "Computation done" << std::endl;
"""


CODE_18 = """
std::vector<int> numbers = {4, 2, 7, 1};
std::sort(numbers.begin(), numbers.end());
"""


CODE_19 = """
for (int i = 0; i < N; i++) {
    processData(arr[i]);
}
"""


CODE_20 = """
int x = 10;
int y = 20;
int z = x + y;
printf("Sum: %d", z);
"""


CODE_21 = """
std::string name = "hammerbert";
std::reverse(name.begin(), name.end());
std::cout << name;
"""


CODE_22 = """
for (int i = 0; i < 50; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
"""


CODE_23 = """
std::vector<std::string> lines;
std::ifstream file("input.txt");
std::string line;
while (getline(file, line)) {
    lines.push_back(line);
}
"""


CODE_24 = """
std::array<int, 5> a = {1, 2, 3, 4, 5};
int result = std::accumulate(a.begin(), a.end(), 0);
"""


CODE_25 = """
for (int i = 0; i < list.size(); ++i) {
    std::cout << list[i] << std::endl;
}
"""

CODE_26 = """
auto now = std::chrono::system_clock::now();
auto timestamp = std::chrono::system_clock::to_time_t(now);
std::cout << std::ctime(&timestamp);
"""

CODE_27 = """
for (const auto& val : array) {
    if (val % 2 == 0) {
        std::cout << val << " is even" << std::endl;
    }
}
"""

CODE_28 = """
int temp = 0;
for (int i = 0; i < 10; ++i) {
    temp += i;
    asm volatile("clflushopt (%0)" : : "r" (&temp) : "memory");
}
"""

CODE_29 = """
const int size = 64;
for (int i = 0; i < size; ++i) {
    arr[i] = i;
}
for (int i = 0; i < size; ++i) {
    asm volatile("clflush (%0)" : : "r" (&arr[i]) : "memory");
}
std::cout << "Data flushed." << std::endl;
"""