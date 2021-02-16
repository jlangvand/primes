#ifndef PRIME_PRIMETEST_H_
#define PRIME_PRIMETEST_H_

#include <boost/random.hpp>
#include <cstdint>

namespace jlprime {
  bool primeTest(std::uint32_t n, boost::random::mt19937 rng, int passes);
}


#endif  // PRIME_PRIMETEST_H_
