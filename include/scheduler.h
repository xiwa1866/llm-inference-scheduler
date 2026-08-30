#pragma once

#include "request.h"
#include "worker_pool.h"
#include <memory>

namespace scheduler {

class Scheduler {
public:
    Scheduler(size_t num_workers);

    void submit(std::shared_ptr<Request> request);

    void shutdown();

private:
    WorkerPool worker_pool_;
};

} // namespace scheduler