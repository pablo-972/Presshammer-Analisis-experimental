#include <inttypes.h>

/**
 * Clase para decodificar direcciones
 * Código de Blacksmith: https://github.com/comsec-group/blacksmith
 */

#define CHANS(x) ((x) << (8UL * 3UL))
#define DIMMS(x) ((x) << (8UL * 2UL))
#define RANKS(x) ((x) << (8UL * 1UL))
#define BANKS(x) ((x) << (8UL * 0UL))

#define MTX_SIZE 30

struct MemConfiguration {
  int IDENTIFIER;
  int BK_SHIFT;
  int BK_MASK;
  int ROW_SHIFT;
  int ROW_MASK;
  int COL_SHIFT;
  int COL_MASK;
  int DRAM_MTX[MTX_SIZE];
  int ADDR_MTX[MTX_SIZE];
};


class Mapping
{
private:
    int bank;
    int row;
    int column;

struct MemConfiguration dual_rank = {
      .IDENTIFIER = (CHANS(1UL) | DIMMS(1UL) | RANKS(2UL) | BANKS(16UL)),
      .BK_SHIFT =  25,
      .BK_MASK =  (0b11111),
      .ROW_SHIFT =  0,
      .ROW_MASK =  (0b111111111111),
      .COL_SHIFT =  12,
      .COL_MASK =  (0b1111111111111),
      .DRAM_MTX =  {
          0b000000000000000010000001000000,
          0b000000000001000100000000000000,
          0b000000000010001000000000000000,
          0b000000000100010000000000000000,
          0b000000001000100000000000000000,
          0b000000000000000010000000000000,
          0b000000000000000001000000000000,
          0b000000000000000000100000000000,
          0b000000000000000000010000000000,
          0b000000000000000000001000000000,
          0b000000000000000000000100000000,
          0b000000000000000000000010000000,
          0b000000000000000000000000100000,
          0b000000000000000000000000010000,
          0b000000000000000000000000001000,
          0b000000000000000000000000000100,
          0b000000000000000000000000000010,
          0b000000000000000000000000000001,
          0b100000000000000000000000000000,
          0b010000000000000000000000000000,
          0b001000000000000000000000000000,
          0b000100000000000000000000000000,
          0b000010000000000000000000000000,
          0b000001000000000000000000000000,
          0b000000100000000000000000000000,
          0b000000010000000000000000000000,
          0b000000001000000000000000000000,
          0b000000000100000000000000000000,
          0b000000000010000000000000000000,
          0b000000000001000000000000000000
      },
      .ADDR_MTX =  {
          0b000000000000000000100000000000,
          0b000000000000000000010000000000,
          0b000000000000000000001000000000,
          0b000000000000000000000100000000,
          0b000000000000000000000010000000,
          0b000000000000000000000001000000,
          0b000000000000000000000000100000,
          0b000000000000000000000000010000,
          0b000000000000000000000000001000,
          0b000000000000000000000000000100,
          0b000000000000000000000000000010,
          0b000000000000000000000000000001,
          0b000010000000000000000000001000,
          0b000100000000000000000000000100,
          0b001000000000000000000000000010,
          0b010000000000000000000000000001,
          0b000001000000000000000000000000,
          0b000000100000000000000000000000,
          0b000000010000000000000000000000,
          0b000000001000000000000000000000,
          0b000000000100000000000000000000,
          0b000000000010000000000000000000,
          0b000000000001000000000000000000,
          0b100001000000000000000000000000,
          0b000000000000100000000000000000,
          0b000000000000010000000000000000,
          0b000000000000001000000000000000,
          0b000000000000000100000000000000,
          0b000000000000000010000000000000,
          0b000000000000000001000000000000
      }
  };

public:
    Mapping() {};

    // copy constructor for mapping
    Mapping (const Mapping &m)
    {
        this->bank = m.bank;
        this->row = m.row;
        this->column = m.column;
    }

    static uintptr_t base_address;
    
    int get_column();
    int get_bank();
    int get_row();

    int linearize();

    void increment_row();
    void increment_column_dw();
    void increment_column_cb();
    void reset_column();
    void increment_bank();
    void decrement_row();

    uintptr_t to_virt();

    void decode_new_address(uintptr_t addr);

};
