// base class representing an LLM prompt
#pragma once
#include <cstddef>
#include <cstdint>
#include <atomic>
struct Request {
  uint64_t id;
  size_t isl;
  size_t osl;
  enum class State { QUEUED, RUNNING, FINISHED };
  State state;
  static inline std::atomic<uint64_t> uid_counter{0};
  Request(size_t isl, size_t osl)
      : id(uid_counter++), isl(isl), osl(osl),
        state(State::QUEUED) {}
};