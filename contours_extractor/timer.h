#pragma once
#include <chrono>

namespace xr
{
  class Timer {
    std::chrono::high_resolution_clock::time_point tic_;

  public:
    Timer():
      tic_(std::chrono::high_resolution_clock::now())
    {

    }

    void tic() {
      tic_ = std::chrono::high_resolution_clock::now();
    }

    uint64_t toc() const {
      auto now = std::chrono::high_resolution_clock::now();
      return std::chrono::duration_cast<std::chrono::milliseconds>(now - tic_).count();
    }
  };
}
