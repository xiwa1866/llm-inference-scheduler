#include "scheduler.h"
#include <iostream>
#include <random>
#include <vector>
#include <cassert>
using namespace scheduler;
int main() {
  constexpr size_t NUM_WORKERS = 4;
  constexpr size_t NUM_REQUESTS = 20;

  std::cout << "Starting LLM scheduler...\n";
  std::cout << "Workers: " << NUM_WORKERS << '\n';
  std::cout << "Requests: " << NUM_REQUESTS << "\n\n";

  Scheduler scheduler(NUM_WORKERS);

  // Keep references so we can inspect the requests after
  // the workers have finished processing them.
  std::vector<std::shared_ptr<Request>> requests;
  requests.reserve(NUM_REQUESTS);

  // Fixed seed makes the workload reproducible.
  std::mt19937 gen(42);

  std::uniform_int_distribution<size_t> isl_dist(100, 1000);
  std::uniform_int_distribution<size_t> osl_dist(20, 100);

  // Submit requests.
  for (size_t i = 0; i < NUM_REQUESTS; ++i) {
    const size_t isl = isl_dist(gen);
    const size_t osl = osl_dist(gen);

    auto request = std::make_shared<Request>(isl, osl);

    std::cout << "Submitting REQUEST " << request->id
              << " (ISL=" << request->isl << ", OSL=" << request->osl << ")\n";

    requests.push_back(request);

    scheduler.submit(request);
  }

  std::cout << "\nAll requests submitted.\n";
  std::cout << "Shutting down LLM scheduler.\n\n";

  // Graceful shutdown:
  // finish requests already in the queue, then stop workers.
  scheduler.shutdown();

  std::cout << "\nChecking request states...\n";

  size_t finished = 0;

  for (const auto &request : requests) {
    std::cout << "Request " << request->id
              << " state = " << static_cast<int>(request->state) << '\n';

    if (request->state == Request::State::FINISHED) {
      ++finished;
    }
  }

  std::cout << "\n================================\n";
  std::cout << "Submitted: " << requests.size() << '\n';
  std::cout << "Finished:  " << finished << '\n';
  std::cout << "================================\n";

  // E2E correctness check.
  assert(finished == NUM_REQUESTS);

  std::cout << "All requests completed successfully.\n";

  return 0;
}