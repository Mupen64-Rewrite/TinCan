#ifndef TASINPUT_IPC_SHM_BLOCK_HPP_INCLUDED
#define TASINPUT_IPC_SHM_BLOCK_HPP_INCLUDED
#include <array>
#include <atomic>
#include "mupen64plus/m64p_plugin.h"
#include "../oslib/mutex.hpp"

namespace tasinput::ipc {
  // Contents of the shared-memory block
  // between client and server.
  struct shm_block {
    std::array<CONTROL, 4> cstate {};
    oslib::ipc_mutex cstate_lock {};
    
    std::array<std::atomic_uint32_t, 4> inputs {};
    
    std::atomic_bool input_dfr_flag {};
    std::atomic_bool stop_flag {};
    std::atomic_bool show_flag {};
    
    void client_cleanup() {
      cstate_lock.client_cleanup();
    }
  };
}
#endif