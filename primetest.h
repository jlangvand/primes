#ifndef PRIME_PRIMETEST_H_
#define PRIME_PRIMETEST_H_

#include <boost/random.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <cstdint>

namespace jlprime {
  bool primeTest(std::uint64_t n, boost::random::mt19937_64 rng, int passes);
}


#endif  // PRIME_PRIMETEST_H_
