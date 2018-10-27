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

#endif