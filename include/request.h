// base class representing an LLM prompt
#pragma once
#include <cstddef>
#include <cstdint>
#include <mutex>

struct WorkerPool;

class Request {
  friend WorkerPool;

public:
  enum class State { QUEUED, RUNNING, FINISHED, CANCELED };

private:
  uint64_t id;
  uint64_t isl;
  uint64_t osl;
  State state;
  mutable std::mutex state_mtx;
  static inline std::atomic<uint64_t> uid_counter{0};

public:
  Request(uint64_t isl, uint64_t osl);
  const uint64_t get_id() const;
  const uint64_t get_isl() const;
  const uint64_t get_osl() const;
  const State get_state() const;
  bool cancel();
};