#include "global.hpp"
#include <mupen64plus/m64p_types.h>
#include "ipc/postbox.hpp"
#include <boost/process.hpp>
#include <bit>
#include <cstdint>
#include <cstring>
#include <optional>

m64p_dynlib_handle tc::g_core_handle;
void* tc::g_log_context;
void (*tc::g_log_callback)(void* context, int level, const char* str);

std::optional<tc::tempdir_handle> tc::g_tempdir;
std::optional<zmq::context_t> tc::g_zmq_ctx;
std::optional<tc::postbox> tc::g_postbox;

std::optional<std::jthread> tc::g_post_thread;
std::optional<boost::process::child> tc::g_process;

CONTROL* tc::g_control_states = nullptr;
std::array<std::atomic_uint32_t, 4> tc::g_input_states { 0, 0, 0, 0 };

static void do_log(const msgpack::object& obj) {
  auto args = obj.as<std::tuple<int, std::string>>();
  tc::trace((m64p_msg_level) std::get<0>(args), std::get<1>(args));
}
static void do_update_controls(const msgpack::object& obj) {
  auto args = obj.as<std::tuple<int, bool, int, int, int>>();
  auto& state = tc::g_control_states[std::get<0>(args)];
  state.Present = std::get<1>(args);
  state.RawData = std::get<2>(args);
  state.Plugin = std::get<3>(args);
  state.Type = std::get<4>(args);
}

void tc::setup_post_listeners() {
  g_postbox->listen("Log", do_log);
  g_postbox->listen("UpdateControls", do_update_controls);
}

void tc::post_thread_loop(std::stop_token tok) {
  do {
    g_postbox->event_loop(tok);
  } while (!tok.stop_requested());
}