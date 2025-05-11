#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>
#include <string>
#include "Mapping.h"
#include "presshammer.h"


void checkPermissionsFile(bool isDoubleSided);