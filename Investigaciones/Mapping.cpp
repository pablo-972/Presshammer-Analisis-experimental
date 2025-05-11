#include "Mapping.h"

uintptr_t Mapping::baseAddress;


int Mapping::getColumn(){
    return column;
}


int Mapping::getBank(){
    return bank;
}


int Mapping::getRow(){
    return row;
}


void Mapping::incrementRow(){
  this->row++;
}


void Mapping::incrementBank(){
  this->bank++;
}


void Mapping::decrementRow(){
  this->row--;
}


void Mapping::incrementColumnDw(){
  this->column+=8;
}


void Mapping::incrementColumnCb(){
  this->column+=64;
}


void Mapping::resetColumn(){
  this->column = 0;
}


int Mapping::linearize() {
  return (this->bank << dualRank.BK_SHIFT) 
        | (this->row << dualRank.ROW_SHIFT) 
        | (this->column << dualRank.COL_SHIFT);
}


uintptr_t Mapping::toVirt() {
  int result = 0;
  int linearize_value = this->linearize();

  for (unsigned long i : dualRank.ADDR_MTX) {
    result <<= 1ULL;
    result |= (int) __builtin_parityl(linearize_value & i);
  }
  return result + this->baseAddress;
}


void Mapping::decodeNewAddress(uintptr_t addr){
    int result = 0;

    for (unsigned long i : dualRank.DRAM_MTX) {
      result <<= 1ULL;
      result |= (int) __builtin_parityl(addr & i);
    }
    bank = (result >> dualRank.BK_SHIFT) & dualRank.BK_MASK;
    row = (result >> dualRank.ROW_SHIFT) & dualRank.ROW_MASK;
    column = (result >> dualRank.COL_SHIFT) & dualRank.COL_MASK;
}