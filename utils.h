#ifndef _utils_h_
#define _utils_h_

#include <time.h>

inline long currentTimeMillis() { return clock() / (CLOCKS_PER_SEC / 1000); }

#endif
