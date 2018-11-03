#ifndef _UTIL_H_
#define _UTIL_H_

#define DECODE_SHORT(x) (*(x) | (*((x) + 1) << 8))
#define DECODE_SIGNED_BYTE(x) (*(reinterpret_cast<const int8_t*>(x)))

#define WRITE_BYTE(dst, value) (*(dst) = (value))
#define WRITE_SHORT(dst, value) (*reinterpret_cast<uint16_t*>(dst) = (value))
#define WRITE_LONG(dst, value) (*reinterpret_cast<uint32_t*>(dst) = (value))

#define READ_MASK(value, mask) (((value) & mask) == mask)
#define READ_BIT(value, bit) (((value) & (1 << bit)) != 0)

#define SET_MASK(value, mask) ((value) | mask)
#define SET_BIT(value, bit) ((value) | (1 << bit))

#define UNSET_MASK(value, mask) ((value) & ~mask)
#define UNSET_BIT(value, bit) ((value) & ~(1 << bit))

#define SET_MASK_IF(value, mask, condition) (condition ? SET_MASK(value, mask) : UNSET_MASK(value, mask))
#define SET_BIT_IF(value, bit, condition) (condition ? SET_BIT(value, bit) : UNSET_BIT(value, bit))

#define REINTERPRET(value, type) (*reinterpret_cast<const type*>(&value))

//#define CARRY_8(a, b, x) ((a ^ b ^ x) & 0x100)
//#define CARRY_16(a, b, x) ((a ^ b ^ x) & 0x10000)

#define OVERFLOW_8(a, b, x) (x < a)
#define OVERFLOW_16(a, b, x) (x < a)

#define UNDERFLOW_8(a, b, x) (x > a)
#define UNDERFLOW_16(a, b, x) (x > a)

#define CARRY_BIT_4(a, b, x) (((a ^ b ^ x) & 0x10) == 0x10)
#define CARRY_BIT_8(a, b, x) (((a ^ b ^ x) & 0x100) == 0x100)
#define CARRY_BIT_12(a, b, x) (((a ^ b ^ x) & 0x800) == 0x800)

#endif