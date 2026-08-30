#pragma once
#include "request.h"
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#define MAX_QUEUE_SIZE 16
struct WorkerPool {
  std::queue<std::shared_ptr<Request>> request_queue;
  std::mutex mtx;
  std::condition_variable cv_consumer;
  std::condition_variable cv_producer;
  std::vector<std::thread> workers;
  bool stop_server = false;
  const size_t size();
  void worker_loop(const size_t worker_id);
  void shutdown();

  WorkerPool(size_t num_workers);
};