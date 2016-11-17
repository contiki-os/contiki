#include "../ff.h"

#if _USE_LFN != 0

#if   _CODE_PAGE == 932	/* Japanese Shift_JIS */
#include "unicode/cc932.c"
#elif _CODE_PAGE == 936	/* Simplified Chinese GBK */
#include "unicode/cc936.c"
#elif _CODE_PAGE == 949	/* Korean */
#include "unicode/cc949.c"
#elif _CODE_PAGE == 950	/* Traditional Chinese Big5 */
#include "unicode/cc950.c"
#else					/* Single Byte Character-Set */
#include "unicode/ccsbcs.c"
#endif

#endif
