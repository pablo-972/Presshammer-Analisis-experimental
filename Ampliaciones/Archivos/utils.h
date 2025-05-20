#pragma once
#include <iostream>
#include <string>
#include <unistd.h>
#include <stdint.h>
#include <elf.h>

void showStep(int number, const std::string& title);

void showSuccessInfo(const std::string& info);
void showConclusion();
void showSuccessAddress(const std::string& info, uintptr_t address);
void showCriticalAddress(const std::string& info, uintptr_t address);
void showInfo(const std::string& info);
void showPermissionsInfo(const std::string& info, mode_t mode);
void showContentInfo(char *content);
void showElfHeaderInfo(Elf64_Ehdr *header);
void showErrorInfo(const std::string& info);

