#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/mman.h>
#include <linux/kernel-page-flags.h>
#include "presshammer.h"
#include "taggon.h"


namespace color {
    constexpr const char* RESET  = "\033[0m";
    constexpr const char* RED    = "\033[31m";
    constexpr const char* GREEN  = "\033[32m";
    constexpr const char* YELLOW = "\033[33m";
    constexpr const char* BLUE   = "\033[34m";
}


void printBanner() {
        std::cout << R"(
    __________                              __                                         
    \______   \_______  ____   ______ _____|  |__ _____    _____   _____   ___________      ______
    |     ___/\_  __ \_/ __ \ /  ___//  ___/  |  \\__  \  /     \ /     \_/ __ \_  __ \    |_,.,--\
    |    |     |  | \/\  ___/ \___ \ \___ \|   Y  \/ __ \|  Y Y  \  Y Y  \  ___/|  | \/       ||
    |____|     |__|    \___  >____  >____  >___|  (____  /__|_|  /__|_|  /\___  >__|          ||
                            \/     \/     \/     \/     \/      \/      \/     \/             ##
                                                                                              ##               
    )" << "\n";
}


volatile char* allocateTargetMemory(){
    std::cout << color::BLUE << "[*] " << color::RESET << "Allocating contiguous memory..." << std::endl;

    const std::string hugetlbfsMountPoint = "/mnt/huge/buff";
    FILE *fp = fopen(hugetlbfsMountPoint.c_str(), "w+");
    if (fp == nullptr){
        std::cerr << color::RED << "[!] " << color::RESET << "Could not mount superpage" << std::endl;
        exit(EXIT_FAILURE);
    }

    ulong memSize = 1ULL << 30ULL;
    volatile char *startAddress = (volatile char *) 0x2000000000;
    auto mappedTargetAddress = mmap((void *) startAddress, memSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS | MAP_HUGETLB | (30UL << MAP_HUGE_SHIFT), fileno(fp), 0);

    if (mappedTargetAddress == MAP_FAILED) {
        std::cerr << color::RED << "[!] " << color::RESET << "mmap failed" << std::endl;
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    volatile char *targetAddress = (volatile char*) mappedTargetAddress;
    std::cout << color::BLUE << "[*] " << color::RESET << "Virtual direction at: " << std::hex << (uintptr_t) targetAddress << std::dec << '\n' << std::endl;

    return targetAddress;
}



void experimentalAnalysis(bool isVerify, int numVictims){
    std::cout << color::GREEN << "[+] " << color::RESET << "STARTING ATTACK " << std::endl;

    // file to save bitflips records
    std::ofstream bitFlipsFile;
    bitFlipsFile.open("bitflips.txt");
    bitFlipsFile << "row,aggressors_activations,reads,median_of_time,bitflips" << std::endl;
    
    volatile char *targetAddress = allocateTargetMemory();

    if(isVerify){
        std::cout << color::BLUE << "[*] " << color::RESET << "STARTING VERIFY tAggON " << std::endl;
        verifytAggOn((uintptr_t) targetAddress);

    }else{
        std::cout << color::BLUE << "[*] " << color::RESET << "STARTING PRESSHAMMER " << std::endl;

        // this array determines the number of reads to the aggressors. The higher the number of reads, the longer the row is kept open.
        int numReadsArr[10] = {1, 2, 4, 8, 16, 32, 48, 64, 80, 128}; 

        // this array determines the index of numReadsArr up to which that much read count can be fit into a refresh window with the given activation count
        // e.g., for activation count 3, we can perform number of reads upto numReadsArr[experimentCounts[4-3]] = numReadsArr[9] = 80 (not including)
        int experimentCounts[4] = {7, 9, 10, 10}; 
        
        // activate aggressors this many times
        for (int i = 4; i > 0; i--){ 

            // perform this many reads after each activation 
            for (int j = 0; j < experimentCounts[4-i]; j++){ 

                std::cout << color::BLUE << "[*] " << color::RESET << "Experiment with number of activations: " << i << " and number of reads: " << numReadsArr[j] << std::endl;
                int totalBitFlips = doubleSidedAttack((uintptr_t) targetAddress, i, numReadsArr[j], numVictims, bitFlipsFile);
                std::cout << color::GREEN << "[+] " << color::RESET << " Total Bit Flips: " << totalBitFlips << std::endl;
            }
        }
    }
    
    std::cout << color::GREEN << "[+] " << color::RESET << "FINISHED!" << std::endl;

}



int main(int argc, char *argv[]){
    printBanner();

    int index;
    int arguments;

    int numVictims = 1500;  // number of victim rows to hammer
    bool isVerify = false;  // verify row access time

    
    static struct option long_options[] = {
        {"verify", no_argument, 0, 'v'}, 
        {"numVictims", required_argument, 0, 'n'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    

    while ((arguments = getopt_long(argc, argv, "vn:dt:", long_options, &index)) != -1)
    {
        switch (arguments){
            case 'n':
                numVictims = std::atoi(optarg);
                break;
            case 'v':
                isVerify = true;
                break;
            case 'h':
                std::cout << "Usage: " << argv[0] << " [--verify, -v] [--numVictims, -n N]" << std::endl;
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " [--verify, -v] [--numVictims, -n N]" << std::endl;
                exit(EXIT_FAILURE);
        }
    }

    experimentalAnalysis(isVerify, numVictims);
    exit(0);
}