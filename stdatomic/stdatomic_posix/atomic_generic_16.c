
#include "stdatomic-impl.h"

#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_16
INSTANTIATE_STUB_LF(16, __impl_uint128_t)
INSTANTIATE_SYNC(16, __impl_uint128_t)
#else
INSTANTIATE_STUB_LCM(16, __impl_uint128_t)
INSTANTIATE_SYNCM(16, __impl_uint128_t)
# if __UINT128__
INSTANTIATE_STUB_LCA(16, __impl_uint128_t)
INSTANTIATE_SYNCA(16, __impl_uint128_t)
# endif
#endif

