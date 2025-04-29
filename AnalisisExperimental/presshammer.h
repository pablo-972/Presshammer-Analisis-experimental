#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <algorithm>
#include <ctime>
#include <utility>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <immintrin.h>
#include <inttypes.h>
#include <sched.h>
#include <bits/stdc++.h>
#include <sys/resource.h>

#include "Mapping.h"


int doubleSidedAttack(uintptr_t targetAddress, int numAggrActs, int numReads, int numVictims);
