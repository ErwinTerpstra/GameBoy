#ifndef _UTIL_H_
#define _UTIL_H_

#define DECODE_SHORT(x) (*x | (*(x + 1) << 8))
#define DECODE_SIGNED_BYTE(x) (*(reinterpret_cast<const int8_t*>(x)))

#endif