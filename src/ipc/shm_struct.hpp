#ifndef TASINPUT_IPC_SHM_BLOCK_HPP_INCLUDED
#define TASINPUT_IPC_SHM_BLOCK_HPP_INCLUDED
#include <array>
#include <atomic>
#include "mupen64plus/m64p_plugin.h"
#include "../oslib/mutex.hpp"

namespace tasinput::ipc {
  // The struct members may appear in disarray,
  // but this ensures a small size.
  struct shm_block {
    std::array<CONTROL, 4> cstate;
    oslib::ipc_mutex cstate_lock;
    std::atomic_uint32_t inputs;
    std::atomic_flag cstate_flag;
  };
}
#endif