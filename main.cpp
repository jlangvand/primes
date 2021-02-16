#include <boost/multiprecision/miller_rabin.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/random.hpp>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <list>
#include <mutex>
#include <ostream>
#include <pthread.h>

using std::cout;

namespace mp = boost::multiprecision;
namespace bpo = boost::program_options;

std::mutex cout_mutex;
std::mutex foundPrimes_mutex;
std::list<std::uint32_t> foundPrimes;

const int bp[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23 };

struct thread_data {
  int thread_id;
  std::uint32_t start;
  std::uint32_t end;
  int tests;
  boost::random::mt19937 rng;
};

bool primeTest(std::uint32_t n, boost::random::mt19937 rng, int passes);
void *PrimeWorker(void *threadarg);

int main(int argc, const char** argv) {
  bpo::options_description desc("Usage");
  desc.add_options()
    ("help",
     "show this text")
    ("from", bpo::value<std::uint32_t>(),
     "Find primes starting from (integer)")
    ("to", bpo::value<std::uint32_t>(),
     "Find primes up to (integer)")
    ("threads", bpo::value<int>(),
     "Number of threads")
    ("passes", bpo::value<int>(),
     "Number of passes to use for primality determination");
  bpo::variables_map vm;
  bpo::store(bpo::parse_command_line(argc, argv, desc), vm);
  bpo::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  std::uint32_t t_start = 2;
  std::uint32_t t_end = 10;
  int threadCount = 16;
  int passes = 6;

  if (vm.count("to")) {
    t_end = vm["to"].as<std::uint32_t>();
  } else {
    std::cout << "Specify an upper bound\n" << desc;
    return 1;
  }
  if (vm.count("from")) t_start = vm["from"].as<std::uint32_t>();
  if (vm.count("threads")) threadCount = vm["threads"].as<int>();
  if (vm.count("passes")) passes = vm["passes"].as<int>();

  if (t_start % 2 == 0)
    t_start++;
  if (t_end % 2 == 0)
    t_end--;
  if ((t_end - t_start) / 2 <= threadCount)
    threadCount = (t_end - t_start) / 3;

  pthread_t thread[threadCount];
  struct thread_data td[threadCount];
  int rc;
    
  std::cout << "Testing for primes in range " << t_start << " - " << t_end
            << "\nMiller-Rabin, " << passes << " passes\n" << threadCount
            << " threads\n";

  std::uint32_t _e, _s = 0;
  boost::random::mt19937 gen(clock());
  
  for (int i = 0; i < threadCount; i++) {
    _e = _s + ((t_end - t_start) / threadCount);
    //cout << "Thread start: " << _s + t_start << ", end: " << _e + t_start << std::endl;
    td[i].thread_id = i;
    td[i].rng = gen;
    td[i].start = _s + t_start;
    td[i].end = _e + t_start;
    td[i].tests = passes;
    rc = pthread_create(&thread[i], NULL, PrimeWorker, (void *)&td[i]);
    if (rc) {
      std::cout << "Error:unable to create thread," << rc << "\n";
      exit(-1);
    }
    _s = ++_e;
  }

  for (int i = 0; i < threadCount; i++) {
    pthread_join(thread[i], NULL);
  }

  for (foundPrimes.sort(); foundPrimes.size(); foundPrimes.pop_front()) {
    cout << foundPrimes.front() << "\n";
  }

  pthread_exit(NULL);

  //std::cout << "Main thread finished\n";
}

bool primeTest(std::uint32_t n, boost::random::mt19937 rng, int passes) {
  for (int i = 0; i < 9; i++) if (!(n % bp[i])) return n == bp[i];
  if (n < 8) return false;
  return mp::miller_rabin_test(n, passes, rng);
}

void *PrimeWorker(void *threadarg) {
  struct thread_data *arg = (struct thread_data *) threadarg;
  std::uint32_t _start = arg->start;
  std::uint32_t foundCount = 0;
  if (_start % 2 == 0) _start++;
  for (std::uint32_t n = _start; n <= arg->end; n += 2) {
    if (primeTest(n, arg->rng, arg->tests)) {
      const std::lock_guard<std::mutex> lock(foundPrimes_mutex);
      foundPrimes.push_back(n);
    }
  }
  pthread_exit(NULL);
}
