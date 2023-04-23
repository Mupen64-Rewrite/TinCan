#ifndef TINCAN_IPC_MESSAGE_HPP_INCLUDED
#define TINCAN_IPC_MESSAGE_HPP_INCLUDED
#include <cstdint>
namespace tincan::ipc {
  struct message {
    uint32_t type;
    uint32_t len;
    char data[];
  };
}
#endif