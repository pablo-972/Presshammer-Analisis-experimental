#pragma once
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>
#include <cstring>
#include "Mapping.h"

void singleSidedPresshammer(uintptr_t target);
void doubleSidedPresshammer(uintptr_t target);