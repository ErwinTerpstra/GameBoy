#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include <stdint.h>
#include <algorithm>

#ifndef DMG_DEBUG
	#ifdef _DEBUG
		#define DMG_DEBUG
	#endif
#endif

#define DMG_INLINE inline
#define DMG_FORCE_INLINE DMG_INLINE __forceinline

#endif