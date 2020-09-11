//
// memfind
//

#include <cstddef>
#include <cstdint>

//|///////////////////// memfind ////////////////////////////////////////////
extern "C" uint64_t memfind(const void *src, int ch, size_t n)
{
  auto *s = (const unsigned char*)src;

  for(; ((uint64_t)(s) & 7) != 0 && n != 0 && *s != ch; --n)
    ++s;

  if (n != 0 && *s != ch)
  {
    const uint64_t LO = 0x0101010101010101;
    const uint64_t HI = 0x8080808080808080;

    auto k = LO * uint64_t(ch);

    for(auto w = (uint64_t const *)(s); n >= 8; n -= 8, ++w)
    {
      auto x = *w ^ k;

      if ((x - LO) & ~x & HI)
        break;

      s += 8;
    }
  }

  for(; n != 0 && *s != ch; --n)
    ++s;

  return (uint64_t)s - (uint64_t)src;
}
