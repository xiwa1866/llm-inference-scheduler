#pragma once

#include "request.h"
#include <vector>

constexpr size_t MAX_NUM_SEQS = 8;
constexpr size_t MAX_NUM_BATCHED_TOKENS = 4096;
struct Batch {
  std::vector<std::shared_ptr<Request>> requests;
  uint64_t batched_tokens;
  uint64_t osl;

  const uint64_t num_seqs() const;
  inline bool empty() const { return requests.empty(); }

  Batch();
};