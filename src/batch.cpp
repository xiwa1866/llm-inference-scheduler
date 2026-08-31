#include "batch.h"

Batch::Batch() : batched_tokens(0), osl(0) {}
const uint64_t Batch::num_seqs() const { return requests.size(); }