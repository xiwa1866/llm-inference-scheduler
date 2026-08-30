#include "worker_pool.h"
#include "request.h"
#include <chrono>
#include <iostream>
#include <cassert>

#define PREFILL_TOKENS_PER_MS 5000
#define DECODE_MS_PER_TOKEN 5
void inline simulate_work(size_t isl, size_t osl) {
    // sleep by time
    std::this_thread::sleep_for(std::chrono::milliseconds(isl/PREFILL_TOKENS_PER_MS));
    for(int i=0; i<osl; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(DECODE_MS_PER_TOKEN));
    }
}


/* Main work loop
Iteravatively fetch for queued request
Quit upon request */
void WorkerPool::worker_loop(const size_t worker_id) {
  std::shared_ptr<Request> request;
  while (true) {
    // critical section
    {
      std::unique_lock<std::mutex> guard(mtx);

      // wait for task or termination
      cv.wait(guard,
              [&] { return this->stop_server || !request_queue.empty(); });

      if (this->stop_server && request_queue.empty()) {
        // graceful shutdown, until all requests have been handled.
        return;
      }

      request = request_queue.front();
      request_queue.pop();
    }
    request->state = Request::State::RUNNING;
    simulate_work(request->isl, request->osl);
    request->state = Request::State::FINISHED;
    std::cout << "WORKER " << worker_id << " finished REQUEST " << request->id << std::endl;
  }
}

void WorkerPool::shutdown() {
  {
    std::lock_guard<std::mutex> guard(mtx);
    assert(!this->stop_server && "already shut down!");
    this->stop_server = true;
  }

  for (auto &worker : this->workers) {
    worker.join();
  }
  this->workers.clear();
}

WorkerPool::WorkerPool(size_t num_works) {
  for (int i = 0; i < num_works; i++) {
    workers.emplace_back(&WorkerPool::worker_loop, this, i);
  }
}