
#include <mupen64plus/m64p_common.h>
#include <mupen64plus/m64p_plugin.h>
#include <mupen64plus/m64p_types.h>

#include <latch>
#include <thread>
#include <tuple>
#include "global.hpp"
#include "ipc/postbox.hpp"
#include "util/fs_helper.hpp"
#include <boost/process.hpp>
#include <boost/process/io.hpp>

#define TC_IF_NOT_NULL(ptr) \
  if (ptr)                  \
  *ptr
#define TC_EXPORT(T) extern "C" EXPORT T CALL

#ifdef _WIN32
  #define TC_EXECUTABLE_EXT ".exe"
  #define TC_NEWLINE "\r\n"
#else
  #define TC_EXECUTABLE_EXT
  #define TC_NEWLINE "\n"
#endif

TC_EXPORT(m64p_error)
PluginStartup(
  m64p_dynlib_handle core_handle, void* log_context,
  void (*log_callback)(void* context, int level, const char* str)) {
  if (tc::g_postbox.has_value())
    return M64ERR_ALREADY_INIT;

  tc::g_core_handle  = core_handle;
  tc::g_log_context  = log_context;
  tc::g_log_callback = log_callback;

  try {
    // init temp dir
    tc::g_tempdir.emplace();
    auto sock_path = tc::g_tempdir->operator*() / "socket.sock";

    // init socket
    tc::trace(M64MSG_VERBOSE, "Initializing ZeroMQ");
    tc::g_zmq_ctx.emplace();
    tc::trace(M64MSG_VERBOSE, "Creating socket");
    tc::g_postbox.emplace(
      *tc::g_zmq_ctx, fmt::format("ipc://{}", sock_path.string()));
    auto conn_ep = tc::g_postbox->endpoint();
    tc::tracef(M64MSG_VERBOSE, "Using endpoint {}", conn_ep);

    // prep handlers and start postbox
    tc::trace(M64MSG_VERBOSE, "Initializing event loop");
    tc::setup_post_listeners();
    tc::g_post_thread.emplace(tc::post_thread_loop);

    // prep wait handle
    auto wait_handle = tc::g_postbox->wait("Ready");

    // Start the GUI
    tc::trace(M64MSG_VERBOSE, "Starting GUI");
    auto exe_path =
      tc::get_own_path().parent_path() / "TinCan.NET" TC_EXECUTABLE_EXT;
    tc::g_process.emplace(exe_path.c_str(), boost::process::args({conn_ep}));

    // wait for the GUI to be ready
    tc::trace(M64MSG_VERBOSE, "Waiting for connection...");
    wait_handle.await();
    tc::trace(M64MSG_VERBOSE, "Setup complete");
  }
  catch (const std::exception& exc) {
    fmt::print("Caught exception: {}" TC_NEWLINE, exc.what());
    return M64ERR_SYSTEM_FAIL;
  }
  catch (...) {
    return M64ERR_SYSTEM_FAIL;
  }

  return M64ERR_SUCCESS;
}

TC_EXPORT(m64p_error) PluginShutdown() {
  using namespace std::literals;
  if (!tc::g_postbox.has_value())
    return M64ERR_NOT_INIT;

  tc::g_control_states = nullptr;

  tc::trace(M64MSG_VERBOSE, "Signaling shutdown");
  tc::g_postbox->enqueue("Shutdown"sv);

  tc::trace(M64MSG_VERBOSE, "Awaiting shutdown");
  if (tc::g_process.has_value() && tc::g_process->joinable())
    tc::g_process->join();

  tc::trace(M64MSG_VERBOSE, "Cleaning up");

  tc::g_post_thread->request_stop();
  tc::g_post_thread->join();
  tc::g_post_thread.reset();

  tc::g_postbox.reset();
  tc::g_zmq_ctx.reset();

  tc::trace(M64MSG_VERBOSE, "Shutdown complete");
  return M64ERR_SUCCESS;
}

TC_EXPORT(m64p_error)
PluginGetVersion(
  m64p_plugin_type* plugin_type, int* plugin_version, int* api_version,
  const char** plugin_name_ptr, int* caps) {
  TC_IF_NOT_NULL(plugin_type)     = M64PLUGIN_INPUT;
  TC_IF_NOT_NULL(plugin_version)  = 0x000100;
  TC_IF_NOT_NULL(api_version)     = 0x020101;
  TC_IF_NOT_NULL(plugin_name_ptr) = "TinCan.NET";
  TC_IF_NOT_NULL(caps)            = 0;
  return M64ERR_SUCCESS;
}

TC_EXPORT(int) RomOpen() {
  tc::g_postbox->enqueue("ShowWindows");

  return true;
}

TC_EXPORT(void) RomClosed() {
  tc::g_postbox->enqueue("HideWindows");
}

TC_EXPORT(void) InitiateControllers(CONTROL_INFO info) {
  tc::g_control_states = info.Controls;
  tc::g_postbox->enqueue("RequestUpdateControls");
}

TC_EXPORT(void) GetKeys(int index, BUTTONS* keys) {
  using namespace std::literals;
  tc::g_postbox->post_and_wait("UpdateInputs", [=](const msgpack::object& obj) -> bool {
    auto [rcv_index, rcv_value] = obj.as<std::tuple<int, uint32_t>>();
    if (rcv_index != index)
      return false;
    
    keys->Value = rcv_value;
    return true;
  }, "RequestUpdateInputs", index);
}

TC_EXPORT(void) ControllerCommand(int index, unsigned char* data) {}

TC_EXPORT(void) ReadController(int index, unsigned char* data) {}

TC_EXPORT(void) SDL_KeyDown(int modifiers, int scancode) {}

TC_EXPORT(void) SDL_KeyUp(int modifiers, int scancode) {}