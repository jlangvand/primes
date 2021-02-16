#include <boost/multiprecision/miller_rabin.hpp>
#include <boost/random.hpp>
#include <cstdint>
#include "primetest.h"

namespace jlprime {
  namespace mp = boost::multiprecision;
  
  const int bp[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23 };
  bool primeTest(std::uint32_t n, boost::random::mt19937 rng, int passes) {
    for (int i = 0; i < 9; i++)
      if (!(n % bp[i]))
        return n == (std::uint32_t)bp[i];
    if (n < 8)
      return false;
    return mp::miller_rabin_test(n, passes, rng);
  }
}



