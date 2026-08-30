#pragma once
#include "request.h"
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
struct WorkerPool {
  std::queue<std::shared_ptr<Request>> request_queue;
  std::mutex mtx;
  std::condition_variable cv;
  std::vector<std::thread> workers;
  bool stop_server = false;
  void worker_loop(const size_t worker_id);
  void shutdown();

  WorkerPool(size_t num_workers);
};