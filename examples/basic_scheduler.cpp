#include "scheduler.h"
#include <cassert>
#include <iostream>
#include <random>
#include <vector>
using namespace scheduler;
int main() {
  constexpr size_t NUM_WORKERS = 4;
  constexpr size_t NUM_PRODUCERS = 4;
  constexpr size_t REQUESTS_PER_PRODUCER = 25;

  Scheduler scheduler(NUM_WORKERS);

  std::vector<std::thread> producers;
  producers.reserve(NUM_PRODUCERS);

  for (size_t producer_id = 0; producer_id < NUM_PRODUCERS; ++producer_id) {

    producers.emplace_back([producer_id, &scheduler]() {
      std::mt19937 gen(42 + producer_id);

      std::uniform_int_distribution<size_t> isl_dist(100, 1000);
      std::uniform_int_distribution<size_t> osl_dist(20, 100);

      for (size_t i = 0; i < REQUESTS_PER_PRODUCER; ++i) {
        auto request = std::make_shared<Request>(isl_dist(gen), osl_dist(gen));

        scheduler.submit(request);
        std::ostringstream msg;
        msg << "submitted Request " << request->id << std::endl;
        std::cout << msg.str();
      }
    });
  }

  for (auto &producer : producers) {
    producer.join();
  }

  std::cout << "All producers finished submitting requests.\n";

  scheduler.shutdown();

  std::cout << "Scheduler shutdown complete.\n";

  return 0;
}