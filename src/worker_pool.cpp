#include "worker_pool.h"
#include "batch.h"
#include "request.h"
#include <cassert>
#include <chrono>
#include <iostream>

constexpr double DECODE_PERF = 5.0; // ms per decode step at batch=1
constexpr double DECODE_ALPHA = 0.6;

// Naive algorithm simulating time token based on batch size and length
void simulate_prefill(size_t num_seqs, size_t batch_tokens) {
  if (batch_tokens == 0 || num_seqs == 0)
    return;

  constexpr double ALPHA = 0.6;
  constexpr double TOKEN_COST = 0.01;

  // More sequences improve parallelism, but with diminishing returns.
  double efficiency = std::pow(static_cast<double>(num_seqs), ALPHA);

  double milliseconds =
      TOKEN_COST * static_cast<double>(batch_tokens) / efficiency;

  std::this_thread::sleep_for(
      std::chrono::duration<double, std::milli>(milliseconds));
}

void inline simulate_decode(size_t batch_size, size_t max_osl) {
  if (batch_size == 0 || max_osl == 0)
    return;

  for (size_t step = 0; step < max_osl; ++step) {
    // Each sequence produces one token per decode step.
    double work = std::pow(static_cast<double>(batch_size), DECODE_ALPHA);

    double milliseconds = work * DECODE_PERF;

    std::this_thread::sleep_for(
        std::chrono::duration<double, std::milli>(milliseconds));
  }
}

void simulate_work(size_t isl, size_t osl, size_t num_seqs) {
  // sleep by time
  simulate_prefill(num_seqs, isl);
  simulate_decode(num_seqs, osl);
}

/* Main work loop
Iteravatively fetch for queued request
Quit upon request */
void WorkerPool::worker_loop(const size_t worker_id) {
  std::shared_ptr<Request> request;
  while (true) {
    Batch b;
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

      // process batched request
      size_t remaining_token = MAX_NUM_BATCHED_TOKENS;
      while (!request_queue.empty() && b.num_seqs() < MAX_NUM_SEQS) {
        request = request_queue.front();
        uint64_t isl = 0;
        {
          std::lock_guard<std::mutex> guard(request->state_mtx);
          if (request->state == Request::State::CANCELED) {
            request_queue.pop();
            continue;
          }
          assert(request->state == Request::State::QUEUED &&
                 "request state is not QUEUED");

          isl = request->get_isl();
          if (isl > MAX_NUM_BATCHED_TOKENS) {
            // Super long input
            // Simply drop since no chunked-prefill support yet
            std::ostringstream msg;
            msg << "WORKER " << "unsupported LONG request for V1" << request->id
                << std::endl;
            std::cout << msg.str();
            request_queue.pop();
            cv_producer.notify_one();
            continue;
          }

          // Break when achieved full batch
          if (isl > remaining_token)
            break;

          // No cancellation, ISL is finite and can be put into batch if reached
          // here
          request->state = Request::State::RUNNING;
        }

        // greedily drop requests into the batch
        request_queue.pop();
        cv_producer.notify_one();
        b.requests.emplace_back(request);
        remaining_token -= isl;
        b.batched_tokens += isl;
        b.osl = std::max(b.osl, request->get_osl());
      }
    }

    if (b.empty())
      continue;

    simulate_work(b.batched_tokens, b.osl, b.num_seqs());

    auto assign_finish = [worker_id](Batch &b) {
      std::ostringstream msg;
      msg << "WORKER " << worker_id << " finished REQUEST ";
      for (auto &request : b.requests) {
        {
          std::lock_guard<std::mutex> guard(request->state_mtx);
          request->state = Request::State::FINISHED;
        }

        msg << request->id << " ";
      }
      msg << std::endl;
      std::cout << msg.str();
    };

    assign_finish(b);
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

const size_t WorkerPool::size() const { return this->request_queue.size(); }

WorkerPool::WorkerPool(size_t num_works) {
  for (int i = 0; i < num_works; i++) {
    workers.emplace_back(&WorkerPool::worker_loop, this, i);
  }
}