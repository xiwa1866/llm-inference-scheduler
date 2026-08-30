#include "scheduler.h"
#include "worker_pool.h"
#include <iostream>
#include <mutex>

using namespace scheduler;
void Scheduler::submit(std::shared_ptr<Request> request) {
  {
    std::unique_lock<std::mutex> guard(worker_pool_.mtx);
    // Block input request from overpressuring the scheduler
    worker_pool_.cv_producer.wait(guard, [&] {
      return worker_pool_.size() < MAX_QUEUE_SIZE || worker_pool_.stop_server;
    });
    if (worker_pool_.stop_server)
      return;
    worker_pool_.request_queue.push(request);
  }

  worker_pool_.cv_consumer.notify_one();
}

void Scheduler::shutdown() {
  std::cout << "Shutting down LLM scheduler." << std::endl;
  worker_pool_.shutdown();
}

Scheduler::Scheduler(size_t num_workers) : worker_pool_(num_workers) {}