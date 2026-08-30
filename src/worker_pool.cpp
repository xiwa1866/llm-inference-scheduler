#include "worker_pool.h"
#include "request.h"
#include <cassert>
#include <chrono>
#include <iostream>

#define PREFILL_TOKENS_PER_MS 5000
#define DECODE_MS_PER_TOKEN 5
void inline simulate_work(size_t isl, size_t osl) {
  // sleep by time
  std::this_thread::sleep_for(
      std::chrono::milliseconds(isl / PREFILL_TOKENS_PER_MS));
  for (int i = 0; i < osl; i++) {
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
      cv_consumer.wait(
          guard, [&] { return this->stop_server || !request_queue.empty(); });

      if (this->stop_server && request_queue.empty()) {
        // graceful shutdown, until all requests have been handled.
        return;
      }

      request = request_queue.front();
      request_queue.pop();
    }

    cv_producer.notify_one();
    {
      std::lock_guard<std::mutex> guard(request->state_mtx);
      if (request->state == Request::State::CANCELED)
        continue;
      assert(request->state == Request::State::QUEUED &&
             "request state is not QUEUED");
      request->state = Request::State::RUNNING;
    }

    simulate_work(request->isl, request->osl);
    {
      std::lock_guard<std::mutex> guard(request->state_mtx);
      request->state = Request::State::FINISHED;
    }

    std::ostringstream msg;
    msg << "WORKER " << worker_id << " finished REQUEST " << request->id
        << std::endl;
    std::cout << msg.str();
  }
}

void WorkerPool::shutdown() {
  {
    std::lock_guard<std::mutex> guard(mtx);
    assert(!this->stop_server && "already shut down!");
    this->stop_server = true;
  }
  cv_consumer.notify_all();
  for (auto &worker : this->workers) {
    worker.join();
  }
  this->workers.clear();
}

const size_t WorkerPool::size() { return this->request_queue.size(); }

WorkerPool::WorkerPool(size_t num_works) {
  for (int i = 0; i < num_works; i++) {
    workers.emplace_back(&WorkerPool::worker_loop, this, i);
  }
}