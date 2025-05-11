#include "permissions.h"


namespace color {
    constexpr const char* RESET  = "\033[0m";
    constexpr const char* RED    = "\033[31m";
    constexpr const char* GREEN  = "\033[32m";
    constexpr const char* YELLOW = "\033[33m";
    constexpr const char* BLUE   = "\033[34m";
}


void createFile(const char *testFile){
    std::ofstream file (testFile);
    if (!file.is_open()){
        std::cerr << color::RED << "[!] " << color::RESET << "Error creating file: " << testFile << std::endl; 
    }
    std::cout << color::BLUE << "[*] " << color::RESET << "File created: " << testFile << std::endl;
}


int openFile(const char *testFile){
    // the file is opened with the read/write flag so that, in case of modifying any permission, it is saved in disk
    int fd = open(testFile, O_RDWR);

    if(fd == -1){
        std::cerr << color::RED << "[!] " << color::RESET << "Error opening file: " << testFile << std::endl;
        perror("open");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // perform fsync to ensure that all metadata is synchronized and loaded into memory
    if(fsync(fd) == -1){
        std::cerr << color::RED << "[!] " << color::RESET << "Error synchronizing metadata" << std::endl;
        perror("fsync");
        close(fd);
        exit(EXIT_FAILURE);
    }

    return fd;
}


void showData(const char *testFile, uintptr_t target, mode_t mode){
    std::cout << "\n" << "----------- " << color::RED << "Target file: " << testFile << color::RESET <<  " -----------" << '\n'
    << color::BLUE << "[*] " << color::RESET << "Actual permissions: " << std::oct << (mode & 0777)  << "\n" 
    << color::BLUE << "[*] " << color::RESET << "File permissions struct address: " << std::hex << "0x" << target << std::dec << '\n'
    << "--------------------------------------------------" << '\n' << std::endl;
}


void checkPermissionsFile(bool isDoubleSided){
    std::cout << color::BLUE << "[*] " << color::RESET << "CHECK PERMISSIONS FILE" << std::endl;


    //1. creates a test file
    const char* testFile = "/tmp/test.txt";
    createFile(testFile);


    //2. opens the test file
    int fd = openFile(testFile);


    //3. gets metadata copy
    struct stat sb;

    if(fstat(fd, &sb) == -1){
        std::cerr << color::RED << "[!] " << color::RESET << "Error getting metadata copy" << std::endl;
        perror("fstat");
        close(fd);
        exit(EXIT_FAILURE);
    }


    //4. gets target address
    void* stModeAddr = (void*)(&sb.st_mode);
    volatile char *targetAddress = (volatile char*) stModeAddr;
    uintptr_t target = (uintptr_t) targetAddress;


    // shows data
    showData(testFile, target, sb.st_mode);


    //5 check if can apply presshammer
    if(isDoubleSided){
        doubleSidedPresshammer(target);
    }else{
        // try single sided presshammer because mapping may be unknown
        singleSidedPresshammer(target);
    }


    /*
    it has been verified that we can access the row and apply presshammer 
    but anyway we can change the file permissions from the copy of the metadata since it is temporary and local data.
    */

    sb.st_mode |= S_IXUSR;
    showData(testFile, target, sb.st_mode);

    /*
    although the program can modify the value of sb.st_mode (e.g., adding execute permission),
    this only affects the local copy of the structure returned by fstat().
    */


    // close file
    close(fd);


    // CONCLUSION 
    /*
    in this case, applying presshammer does not make sense because we are not directly accessing
    the inode stored in RAM or disk, but just a copy of it returned to userland.

    even if we manage to flip bits via Rowhammer or Presshammer at this address,
    the changes would affect only temporary, volatile data used by the process,
    not the actual permission bits recognized by the filesystem.

    thus, changing permissions using this method is NOT viable.
    */

}