#include <stdint.h>

/* Define a register*/
#define DEF_REG(name, base)  uint32_t * const SYSREG_##name = (uint32_t *)base 

#define SET_REG(name, val) *((uint32_t *)name) = val
#define GET_REG(name) *((uint32_t *)name)

/* Bitfield operations */
#define SET_BIT(name, pos) *((uint32_t *)name) |= (1 << pos)
#define CLR_BIT(name, pos) *((uint32_t *)name) &= (~(1 << pos))
#define TST_BIT(name, pos) (*((uint32_t *)name) & (1 << pos))?1:0

/* Field access */
struct reg_field {
    uint32_t *base;
    int pos;
    uint32_t size;
};

void reg_field_init(struct reg_field *field, uint32_t *ptr, int pos, int size);

#define WR_FIELD(name, val) *((uint32_t *)(name)->base) |= ((val & ((1<< (name)->size) - 1)) << (name)->pos)
#define RD_FIELD(name)      (*((uint32_t *)(name)->base) >> pos) & ((1 << (name)->size) - 1)


void noop(void);
