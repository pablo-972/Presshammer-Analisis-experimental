#include "presshammer.h"


int numAggrActs = 16;
int numReads = 32;


void singleSidedPresshammer(uintptr_t target){
    volatile unsigned long long *victim = (volatile unsigned long long *) target;

    for(int i = 0; i < numAggrActs; i++){
        for(int j = 0; j < numReads; j++){
            *victim;
        }

        asm volatile("clflushopt (%0)" :: "r"(victim) : "memory");
        asm volatile ("mfence");
    }
}


void doubleSidedPresshammer(uintptr_t target){
    Mapping::baseAddress = target;
    Mapping baseVictim, victim, aggr1, aggr2;

    baseVictim.decodeNewAddress(target);
    baseVictim.resetColumn();

    victim = Mapping(baseVictim);
    aggr1 = Mapping(victim); aggr1.incrementRow();
    aggr2 = Mapping(victim); aggr2.decrementRow();

    volatile unsigned long long *aggr1Addr = (volatile unsigned long long *) (aggr1.toVirt());
    volatile unsigned long long *aggr2Addr = (volatile unsigned long long *) (aggr2.toVirt());


    for(int i = 0; i < numAggrActs; i++){
        for(int j = 0; j < numReads; j++){
            *aggr1Addr;
        }

        for(int j = 0; j < numReads; j++){
            *aggr2Addr;
        }

        asm volatile("clflushopt (%0)" :: "r"(aggr1Addr) : "memory");
        asm volatile ("mfence");
        asm volatile("clflushopt (%0)" :: "r"(aggr2Addr) : "memory");
        asm volatile ("mfence");
    }
}