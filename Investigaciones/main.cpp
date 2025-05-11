#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>
#include <cstring>
#include <getopt.h>

#include "Mapping.h"
#include "permissions.h"
#include "content.h"
#include "elf_header.h"


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



int main(int argc, char *argv[]){
    printBanner();

    int index, arguments;

    bool isDoubleSided = false;
    bool checkPermissions = false;
    bool checkContent = false;
    bool checkElf = false;
    
    static struct option long_options[] = {
        {"permissions", no_argument, 0, 'p'},
        {"content", no_argument, 0, 'c'},
        {"elfHeader", no_argument, 0, 'e'},
        {"isDoubleSided", no_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    if (argc == 1) {  
        std::cout << "Usage: " << argv[0] << " [--permissions, -p] [--content, -c] [--elfHeader, -e] [--isDoublesSided, -d]" << std::endl;
        exit(0);
    }

    while((arguments = getopt_long(argc, argv, "pcedh", long_options, &index)) != -1){
        switch (arguments){
        case 'p':
            checkPermissions = true;
            break;
        case 'c':
            checkContent = true;
            break;
        case 'e':
            checkElf = true;
            break;
        case 'd':
            isDoubleSided = true;
            break;
        case 'h':
            std::cout << "Usage: " << argv[0] << " [--permissions, -p] [--content, -c] [--elfHeader, -e] [--isDoublesSided, -d]" << std::endl;
            exit(0);
        default:
            std::cerr << "Invalid option or missing argument. Use -h for usage information." << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    if(checkPermissions){
        checkPermissionsFile(isDoubleSided);
    }

    if(checkContent){
        checkContentFile(isDoubleSided);
    }

    if(checkElf){
        checkElfHeader(isDoubleSided);
    }

    std::cout << color::GREEN << "[+] " << color::RESET << "Finished!" << std::endl;
    exit(0);
}

