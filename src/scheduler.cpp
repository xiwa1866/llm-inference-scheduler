#include "scheduler.h"
#include "worker_pool.h"
#include <iostream>
#include <mutex>

using namespace scheduler;
void Scheduler::submit(std::shared_ptr<Request> request) {
  {
    std::lock_guard<std::mutex> guard(worker_pool_.mtx);
    worker_pool_.request_queue.push(request);
  }

  worker_pool_.cv.notify_one();
}

void Scheduler::shutdown() {
  std::cout << "Shutting down LLM scheduler." << std::endl;
  worker_pool_.shutdown();
}

Scheduler::Scheduler(size_t num_workers) : worker_pool_(num_workers) {}