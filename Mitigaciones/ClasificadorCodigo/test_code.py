# --------- Test codes --------- #


CODE_1 = """
        for (int j = 0; j < x; j++) {
            *(aggrArr[j]);
            asm volatile("clflush (%0)" : : "r" (aggrArr[j]) : "memory");
        }
        """


CODE_2 = """
        for (int i = 0; i < kIterationPerVictim; i++){
        asm volatile ("lfence");


        // calculate initial time of the attack
        clock_gettime(CLOCK_REALTIME, &tglob1);
        
        // activate numAggrActs times
        for (int j = 0; j < numAggrActs; j++){
            asm volatile ("mfence");

            // for each activation read both aggressors numReads times, which will adjust the aggressor on time
            for (int k = 0; k < numReads; k++)
                *(aggrArr[k*2 + 0]); // the aggressor row 1 is read

            for (int k = 0; k < numReads; k++)
                *(aggrArr[k*2 + 1]); // the aggressor row 2 is read
            
            // flush the cache lines so that we access DRAM again in the next iteration 
            for (int k = 0; k < numReads; k++)
                asm volatile("clflushopt (%0)" : : "r" (aggrArr[k*2 + 0]) : "memory");

            for (int k = 0; k < numReads ; k++)
                asm volatile("clflushopt (%0)" : : "r" (aggrArr[k*2 + 1]) : "memory");

        """