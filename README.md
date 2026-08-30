# LLM Inference Scheduler

A small C++ concurrency project that simulates an LLM inference scheduler using a shared worker pool.

## V1

The current version supports:

* Multiple worker threads
* Thread-safe request queue
* Producer/consumer synchronization with `std::mutex` and `std::condition_variable`
* Graceful shutdown
* Simulated LLM prefill and decode workloads
* Request lifecycle: `QUEUED → RUNNING → FINISHED`

## Running

Build with CMake + Ninja, then run:

```bash
cmake --build build
./build/basic_scheduler
```

`main()` creates a scheduler, generates a reproducible workload with randomized ISL/OSL, submits the requests, performs a graceful shutdown, and verifies that every request reaches `FINISHED`.

## Roadmap

Future versions may explore:

* Multiple concurrent producers
* Priority scheduling
* Request cancellation
* Dynamic batching
* Scheduling and latency metrics
