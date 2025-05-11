#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>
#include <cstring>
#include <elf.h>
#include "Mapping.h"
#include "presshammer.h"


void checkElfHeader(bool isDoubleSided);