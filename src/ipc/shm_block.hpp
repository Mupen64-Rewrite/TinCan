#ifndef TINCAN_IPC_SHM_BLOCK_HPP_INCLUDED
#define TINCAN_IPC_SHM_BLOCK_HPP_INCLUDED
#include <array>
#include <atomic>
#include "../oslib/gate.hpp"
#include "../oslib/mutex.hpp"
#include "mupen64plus/m64p_plugin.h"

namespace tincan::ipc {
  // Contents of the shared-memory block.
  struct shm_block {
    struct shmflags {
      enum : uint32_t {
        // True if stopped
        stop = (uint32_t(1) << 0),
        // True to show dialogs.
        show = (uint32_t(1) << 1),
        dfl  = (uint32_t(1) << 2),
      };
    };
    // State of the controllers (whether they're plugged in or not)
    std::array<CONTROL, 4> cstate {};
    // Mutex controlling access to cstate.
    oslib::ipc_mutex cstate_lock {};
    
    // This gate sends a signal FROM the client.
    oslib::ipc_gate client_signal_gate {};
    
    // The inputs themselves.
    std::array<std::atomic_uint32_t, 4> inputs {};
    // Stores flags to show the window, etc.
    std::atomic_uint32_t flags;

    void client_cleanup() {
      cstate_lock.client_cleanup();
      client_signal_gate.client_cleanup();
    }
  };
}  // namespace tincan::ipc
#endif