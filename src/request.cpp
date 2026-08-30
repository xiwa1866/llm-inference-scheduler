#include "request.h"
#include <mutex>

const uint64_t Request::get_id() { return id; }
const uint64_t Request::get_isl() { return isl; }
const uint64_t Request::get_osl() { return osl; }
const Request::State Request::get_state() {
  {
    std::lock_guard<std::mutex> guard(state_mtx);
    return state;
  }
}

bool Request::cancel() {
  {
    std::lock_guard<std::mutex> guard(state_mtx);
    if (state == Request::State::QUEUED) {
      state = Request::State::CANCELED;
      return true;
    }
    return false;
  }
}

Request::Request(uint64_t isl, uint64_t osl)
    : id(uid_counter++), isl(isl), osl(osl), state(State::QUEUED) {}
