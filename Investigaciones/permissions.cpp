#include "permissions.h"


void createFile(const char *testFile){
    std::ofstream file (testFile);
    if (!file.is_open()){
        showErrorInfo("Error creating file: " + std::string(testFile));
        exit(EXIT_FAILURE);
    }
}

int openFile(const char *testFile){
    int fd = open(testFile, O_RDWR);

    if(fd == -1){
        showErrorInfo("Error opening file: " + std::string(testFile));
        perror("open");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if(fsync(fd) == -1){
        showErrorInfo("Error synchronizing metadata");
        perror("fsync");
        close(fd);
        exit(EXIT_FAILURE);
    }
    return fd;
}


void checkPermissionsFile(bool isDoubleSided){
    showInfo("CHECK PERMISSIONS FILE");

    // ========================================================================
    // STEP 1. Create a test file, open it and get a copy of the file metadata
    // ========================================================================
    showStep(1, "Create a test file, \n open it and get a copy of the file metadata");

    const char* testFile = "/tmp/test.txt";
    createFile(testFile);
    showSuccessInfo("File: " + std::string(testFile) + " created");

    int fd = openFile(testFile);
    showSuccessInfo("File: " + std::string(testFile) + " opened with the read/write flag so, in case of modifying any permission, can save it in disk");

    struct stat sb;
    if(fstat(fd, &sb) == -1){
        showErrorInfo("Error getting metadata copy");
        perror("fstat");
        close(fd);
        exit(EXIT_FAILURE);
    }
    showSuccessAddress("Metadata copy address: ", (uintptr_t) &sb);


    // ========================================================================
    // STEP 2. Get memory address of the st_mode field 
    // ========================================================================
    showStep(2, "Get memory address of the st_mode field");

    void* stModeAddr = (void*)(&sb.st_mode);
    volatile char *targetAddress = (volatile char*) stModeAddr;
    uintptr_t target = (uintptr_t) targetAddress;

    showPermissionsInfo("Actual permissions: ", sb.st_mode);
    showCriticalAddress("File permissions struct address (target): ", target);


    // ========================================================================
    // STEP 3. Check if can apply presshammer
    // ========================================================================
    showStep(3, "Check if can apply presshammer");
    
    if(isDoubleSided){
        doubleSidedPresshammer(target);
    }else{
        singleSidedPresshammer(target); // try single sided presshammer because mapping may be unknown
    }
    showSuccessAddress("Hammered target address: ", target);

    close(fd);


    // ========================================================================
    // CONCLUSION
    // ========================================================================
    showConclusion();
    std::cout << "It has been verified that we can access the row and apply presshammer, \n"
    << "but even if we modify the permissions in the local memory copy of the metadata, \n"
    << "it only affects the local, temporary data, not the actual filesystem data. \n" << std::endl;

    std::cout << "While we can modify the sb.st_mode in memory (e.g., adding execute permissions), \n"
    << "this only affects the temporary structure returned by fstat(). \n"
    << "It does NOT modify the actual permissions on the disk. \n" << std::endl;

    std::cout << "Applying presshammer here does not make sense because: \n"
    << "- We are not directly accessing the inode stored in RAM or on disk. \n"
    << "- The changes only affect the temporary, volatile copy of the metadata in memory. \n"
    << "- Even if we apply Rowhammer or Presshammer and flip bits at this address, the changes would not persist after the program ends. \n"
    << "- Modifying file permissions using this method is NOT viable for permanent changes to the filesystem. \n" << std::endl;


}