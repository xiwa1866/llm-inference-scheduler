#include "scheduler.h"
#include <cassert>
#include <iostream>
#include <random>
#include <vector>
using namespace scheduler;

void test_cancellation() {
  constexpr size_t NUM_WORKERS = 1;
  constexpr size_t NUM_REQUESTS = 20;

  Scheduler scheduler(NUM_WORKERS);

  std::vector<std::shared_ptr<Request>> requests;
  requests.reserve(NUM_REQUESTS);

  // Long first request keeps the single worker busy,
  // allowing later requests to remain QUEUED.
  auto first_request = std::make_shared<Request>(100000, 20);
  requests.push_back(first_request);

  scheduler.submit(first_request);

  std::cout << "Submitted Request " << first_request->get_id()
            << " (long-running)\n";

  // These requests should mostly remain queued behind the first request.
  for (size_t i = 1; i < NUM_REQUESTS; ++i) {
    auto request = std::make_shared<Request>(1000, 20);
    requests.push_back(request);

    scheduler.submit(request);

    bool cancelled = request->cancel();

    std::ostringstream msg;
    msg << "Submitted Request " << request->get_id()
        << " -> cancel = " << (cancelled ? "SUCCESS" : "FAILED") << '\n';
    std::cout << msg.str();
  }

  scheduler.shutdown();

  std::cout << "\nFinal request states:\n";

  for (const auto &request : requests) {
    std::ostringstream msg;
    msg << "Request " << request->get_id()
        << " state = " << static_cast<int>(request->get_state()) << '\n';
    std::cout << msg.str();
  }
}

void test_job_submission() {
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
        msg << "submitted Request " << request->get_id() << std::endl;
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
}

int main() {
  test_cancellation();
  // test_job_submission();
  return 0;
}