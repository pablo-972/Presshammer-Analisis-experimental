#include "utils.h"


namespace color {
    constexpr const char* RESET  = "\033[0m";
    constexpr const char* RED    = "\033[31m";
    constexpr const char* GREEN  = "\033[32m";
    constexpr const char* YELLOW = "\033[33m";
    constexpr const char* BLUE   = "\033[34m";
}



void showStep(int number, const std::string& title){
    std::cout << "\n\n==================================================\n";
    std::cout << " STEP " << number << ": " << title << std::endl;
    std::cout << "==================================================\n";
}


void showConclusion(){
    std::cout << "\n\n==================================================\n";
    std::cout << " CONCLUSION "  << std::endl;
    std::cout << "==================================================\n";
}


void showSuccessInfo(const std::string& info){
    std::cout << color::GREEN << "[+] " << color::RESET << info << std::endl;
}


void showSuccessAddress(const std::string& info, uintptr_t address){
    std::cout << color::GREEN << "[+] " << color::RESET << info << std::hex << address << std::dec << std::endl;
}


void showErrorInfo(const std::string& info){
    std::cout << color::RED << "[!] " << color::RESET << info << std::endl;
}


void showCriticalInfo(const std::string& info){
    std::cout << color::YELLOW << "[~] " << color::RESET << info << std::endl;
}


void showCriticalAddress(const std::string& info, uintptr_t address){
    std::cout << color::YELLOW << "[~] " << color::RESET << info << std::hex << address << std::dec << std::endl;
}


void showInfo(const std::string& info){
    std::cout << color::BLUE << "[*] " << color::RESET << info << std::endl;
}


void showPermissionsInfo(const std::string& info, mode_t mode){
    std::cout << color::BLUE << "[*] " << color::RESET << info << std::oct << (mode & 0777) << std::dec << std::endl;
}


void showContentInfo(char *content){
    std::cout << color::BLUE << "[*] " << color::RESET << "Content: " 
    << "\n" << content << std::endl;
}


void showElfHeaderInfo(Elf64_Ehdr *header){
    std::cout << color::BLUE << "[*] " << color::RESET << "ELF header memory address: " << (void*) header << "\n"
    << color::BLUE << "[*] " << color::RESET << "Entry point value: " << "0x" << header->e_entry << "\n"
    << color::YELLOW << "[~] " << color::RESET << "Entry point memory address (target): " << (void*) & (header->e_entry) << std::endl;
}