//
// stringcmp
//

#include <cstddef>
#include <cstdint>

//|///////////////////// stringcmp //////////////////////////////////////////
extern "C" int32_t stringcmp(uint8_t *lhs, uint64_t lhslen, uint8_t *rhs, uint64_t rhslen)
{
  auto lhsend = lhs + lhslen;
  auto rhsend = rhs + rhslen;

  for(; rhs != rhsend; ++lhs, ++rhs)
  {
    if (lhs == lhsend)
      return -1;

    if (auto cmp = int32_t(*lhs) - int32_t(*rhs); cmp != 0)
      return cmp;
  }

  if (lhs != lhsend)
    return +1;

  return 0;
}
