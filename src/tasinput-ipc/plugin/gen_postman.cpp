// The main() function is derived from google::protobuf::compiler::PluginMain.
// Because licensing, I am legally required to copy/paste their license here.
// ===========================================================================
// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/text_format.h>
#include <boost/core/demangle.hpp>
#include <google/protobuf/compiler/plugin.pb.h>

#include <google/protobuf/io/io_win32.h>

#include <fmt/core.h>
#include <boost/algorithm/string.hpp>

#include <iostream>
#include <iterator>
#include <sstream>
#include <string_view>

namespace proto  = google::protobuf;
namespace protoc = google::protobuf::compiler;

using namespace std::literals;

constexpr std::string_view literal_strip(
  std::string_view s, size_t b = 1, size_t e = 0) {
  return s.substr(b, s.length() - (b + e));
}

// Server templates
// ================

constexpr std::string_view include_list = literal_strip(R"!cpp!(
#include <boost/core/demangle.hpp>
#include <tnp/ipc_proto.hpp>
#include <tnp_ipc.pb.h>
#include <fmt/core.h>
)!cpp!");

constexpr std::string_view server_class_decl = literal_strip(R"!cpp!(
class {0}Server : public ::tnp::ipc::Server {{
private:
{1}
public:
  bool handle_request(
    const ::tnp::ipc::MessageQuery& query, 
    ::tnp::ipc::MessageReply& reply) const override;
}};
)!cpp!"sv);

constexpr std::string_view server_method_decl = literal_strip(
  R"!cpp!(
  void {0}(const {1}& query, {2}& reply) const;
)!cpp!"sv);

constexpr std::string_view server_method_stub_macro = literal_strip(R"!cpp!(
#define {0}_SERVER_METHOD_STUBS \
{1}static_assert(true, "this just makes it easier for me");
)!cpp!");
constexpr std::string_view server_method_stub_impl = literal_strip(R"!cpp!(
void {0}Server::{1}(const {2}& query, {3}& reply) const {{}} \
)!cpp!");

constexpr std::string_view server_handle_def = literal_strip(R"!cpp!(
bool {0}Server::handle_request(
  const ::tnp::ipc::MessageQuery& query, 
  ::tnp::ipc::MessageReply& reply) const {{
  
  if (query.service() != "{1}")
    return false;
  
  std::string_view method = query.method();
{2}
  return false;
}}
)!cpp!");

constexpr std::string_view server_branch = literal_strip(
  R"!cpp!(
  {0}if (method == "{1}") {{
    {2} query_data;
    query.data().UnpackTo(&query_data);
    {3} reply_data;
    try {{
      {1}(query_data, reply_data);
    }}
    catch (const std::exception& e) {{
      tnp::ipc::Exception e_data;
      e_data.set_type(boost::core::demangle(typeid(e).name()));
      e_data.set_detail(e.what());
      
      // set reply parameters
      reply.set_id(query.id());
      reply.set_error(true);
      reply.mutable_data()->PackFrom(e_data);
      return true;
    }}
    
    reply.set_id(query.id());
    reply.set_error(false);
    reply.mutable_data()->PackFrom(reply_data);
    return true;
  }}
)!cpp!"sv);

// Client templates
// ================
constexpr std::string_view client_class_decl = literal_strip(R"!cpp!(
class {0}Client : public ::tnp::ipc::Client {{
public:
  using ::tnp::ipc::Client::Client;
private:
  using ::tnp::ipc::Client::call;
public:
{1}
}};
)!cpp!");

constexpr std::string_view client_method_stub = literal_strip(R"!cpp!(
  {2} {0}(const {1}& query) const;
)!cpp!"sv);

constexpr std::string_view client_method_decl = literal_strip(R"!cpp!(
{3} {0}Client::{1}(const {2}& query) const {{
  ::tnp::ipc::MessageReply queue_reply = call(query, "{4}", "{1}");
  if (queue_reply.error()) {{
    ::tnp::ipc::Exception exc;
    queue_reply.data().UnpackTo(&exc);
    
    auto out = fmt::format("RPC threw {{}}: {{}}", exc.type(), exc.detail());
    throw ::tnp::ipc::remote_call_error(out);
  }}
  {3} reply;
  queue_reply.data().UnpackTo(&reply);
  return reply;
}}
)!cpp!");

namespace std {
  template <class T>
  unique_ptr(T*) -> unique_ptr<T>;
}

class IPCCodeGenerator : public protoc::CodeGenerator {
public:
  bool Generate(
    const proto::FileDescriptor* file, const std::string& parameter,
    protoc::GeneratorContext* generator_context,
    std::string* error) const override {
    std::string stem = protoc::StripProto(file->name());

    if (file->service_count() == 0) {
      // do not generate anything if there are no services
      return true;
    }

    // includes
    {
      std::unique_ptr file(
        generator_context->OpenForInsert(stem + ".pb.h", "includes"));
      proto::io::CodedOutputStream cos(file.get());
      cos.WriteString(std::string(include_list));
    }

    // declaration gen
    {
      std::string out_buffer;
      std::string server_methods;
      std::string client_methods;
      std::string server_stubs;
      for (size_t i = 0; i < file->service_count(); i++) {
        const auto* service = file->service(i);
        
        std::string server_type = "::" + service->full_name();
        boost::replace_all(server_type, ".", "::");
        std::string screaming_snake_server = service->full_name();
        boost::to_upper(screaming_snake_server);
        boost::replace_all(screaming_snake_server, ".", "_");

        server_methods.clear();
        client_methods.clear();
        server_stubs.clear();
        for (size_t j = 0; j < service->method_count(); j++) {
          const auto* method     = service->method(j);
          std::string input_type = "::" + method->input_type()->full_name();
          boost::replace_all(input_type, ".", "::");

          std::string output_type = "::" + method->output_type()->full_name();
          boost::replace_all(output_type, ".", "::");
          
          // Generates stubs for server and client
          fmt::format_to(
            std::back_inserter(server_methods), server_method_decl,
            method->name(), input_type, output_type);
          fmt::format_to(
            std::back_inserter(client_methods), client_method_stub,
            method->name(), input_type, output_type);
          fmt::format_to(
            std::back_inserter(server_stubs), server_method_stub_impl,
            server_type, method->name(), input_type, output_type);
        }

        // combines stubs with class declaration
        fmt::format_to(
          std::back_inserter(out_buffer), server_class_decl, service->name(),
          server_methods);
        fmt::format_to(
          std::back_inserter(out_buffer), client_class_decl, service->name(),
          client_methods);
        fmt::format_to(
          std::back_inserter(out_buffer), server_method_stub_macro, 
          screaming_snake_server, server_stubs);
      }

      std::unique_ptr file(
        generator_context->OpenForInsert(stem + ".pb.h", "namespace_scope"));
      proto::io::CodedOutputStream cos(file.get());
      cos.WriteString(out_buffer);
    }

    // definition gen
    {
      std::string out_buffer;
      std::string branches;
      for (size_t i = 0; i < file->service_count(); i++) {
        const auto* service = file->service(i);

        branches.clear();
        for (size_t j = 0; j < service->method_count(); j++) {
          const auto* method     = service->method(j);
          std::string input_type = method->input_type()->full_name();
          boost::replace_all(input_type, ".", "::");

          std::string output_type = method->output_type()->full_name();
          boost::replace_all(output_type, ".", "::");

          std::string text_else = (i == 0) ? "" : "else ";

          // Generates stub: void Method(const A& query, B& reply)
          fmt::format_to(
            std::back_inserter(branches), server_branch, text_else,
            method->name(), input_type, output_type);
          
          fmt::format_to(
            std::back_inserter(out_buffer), client_method_decl, 
            service->name(), method->name(), input_type, output_type, service->full_name());
        }

        // combines stubs with class declaration
        fmt::format_to(
          std::back_inserter(out_buffer), server_handle_def, service->name(),
          service->full_name(), branches);
      }

      std::unique_ptr file(
        generator_context->OpenForInsert(stem + ".pb.cc", "namespace_scope"));
      proto::io::CodedOutputStream cos(file.get());
      cos.WriteString(out_buffer);
    }

    return true;
  }

  uint64_t GetSupportedFeatures() const override { return 1; };
};

int main(int argc, char* argv[]) {
  IPCCodeGenerator gen;
  return protoc::PluginMain(argc, argv, &gen);
}