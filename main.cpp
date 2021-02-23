#include "primetest.h"

#include <boost/random/mersenne_twister.hpp>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <list>
#include <mutex>
#include <ostream>
#include <pthread.h>

#include <boost/multiprecision/miller_rabin.hpp>
#include <boost/program_options.hpp>
#include <boost/random.hpp>

#define MIN(a, b) a < b ? a : b

namespace bpo = boost::program_options;

std::mutex cout_mutex;
std::mutex foundPrimes_mutex;
std::list<std::uint64_t> foundPrimes;

struct thread_data {
  int thread_id;
  std::uint64_t start;
  std::uint64_t end;
  int tests;
  boost::random::mt19937_64 rng;
};

void *PrimeWorker(void *threadarg);

int main(int argc, const char** argv) {
  bpo::options_description desc("Usage");
  desc.add_options()
    ("help",
     "show this text")
    ("from", bpo::value<std::uint64_t>(),
     "Find primes starting from (integer)")
    ("to", bpo::value<std::uint64_t>(),
     "Find primes up to (integer)")
    ("threads", bpo::value<int>(),
     "Number of threads")
    ("passes", bpo::value<int>(),
     "Number of passes to use for primality determination")
    ("no-output",
     "Don't print primes");
  bpo::variables_map vm;
  bpo::store(bpo::parse_command_line(argc, argv, desc), vm);
  bpo::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  std::uint64_t t_start = 1;
  std::uint64_t t_end = 10;
  unsigned int threadCount = 16;
  int passes = 6;
  bool printNumbers = true;

  if (vm.count("to")) {
    t_end = vm["to"].as<std::uint64_t>();
  } else {
    std::cout << "Specify an upper bound\n" << desc;
    return 1;
  }
  if (vm.count("from")) t_start = vm["from"].as<std::uint64_t>();
  if (vm.count("threads")) threadCount = vm["threads"].as<int>();
  if (vm.count("passes")) passes = vm["passes"].as<int>();
  if (vm.count("no-output")) printNumbers = false;

  if ((t_end - t_start) / 2 <= threadCount)
    threadCount = (t_end - t_start) / 3;

  pthread_t thread[threadCount];
  struct thread_data td[threadCount];
  int rc;
    
  std::cout << "Testing for primes in range " << t_start << " - " << t_end
            << "\nMiller-Rabin, " << passes << " passes\n" << threadCount
            << " threads\n";

  std::uint64_t _e, _s = 0;
  boost::random::mt19937_64 gen(clock());
  
  for (unsigned int i = 0; i < threadCount; i++) {
    _e = _s + ((t_end - t_start) / threadCount);
    td[i].thread_id = i;
    td[i].rng = gen;
    td[i].start = _s + t_start;
    td[i].end = MIN(_e + t_start, t_end);
    td[i].tests = passes;
    rc = pthread_create(&thread[i], NULL, PrimeWorker, (void *)&td[i]);
    if (rc) {
      std::cout << "Error:unable to create thread," << rc << "\n";
      exit(-1);
    }
    _s = ++_e;
  }

  for (unsigned int i = 0; i < threadCount; i++) {
    pthread_join(thread[i], NULL);
  }

  if (!printNumbers) return 0;

  for (foundPrimes.sort(); foundPrimes.size(); foundPrimes.pop_front()) {
    std::cout << foundPrimes.front() << "\n";
  }

  return 0;
}


void *PrimeWorker(void *threadarg) {
  struct thread_data *arg = (struct thread_data *) threadarg;
  std::uint64_t _start = arg->start;
  if (_start % 2 == 0) _start++;
  for (std::uint64_t n = _start; n <= arg->end; n += 2) {
    if (jlprime::primeTest(n, arg->rng, arg->tests)) {
      const std::lock_guard<std::mutex> lock(foundPrimes_mutex);
      foundPrimes.push_back(n);
    }
  }
  pthread_exit(NULL);
}
