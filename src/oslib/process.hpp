#ifndef OSLIB_PROCESS_HPP_INCLUDED
#define OSLIB_PROCESS_HPP_INCLUDED

#include <algorithm>
#include <cstdlib>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <string_view>
#include "preproc.hpp"

namespace oslib {
  class process;
}

#if defined(OSLIB_OS_POSIX)
  #pragma region POSIX implementation
  #include <unistd.h>
  #include <sys/wait.h>
  
  #include <array>
  #include <cstddef>
  #include <optional>
  #include <cerrno>
  #include <numeric>
  #include <system_error>
  
  class process {
  public:
    process(std::string_view program, std::initializer_list<std::string_view> args = {}) : rc(std::nullopt) {
      
      pid_t val = fork();
      if (val == -1) {
        throw std::system_error(errno, std::generic_category());
      }
      
      if (val == 0) {
        // allocate argv tables
        size_t total_chars = program.length() + 1;
        for (std::string_view a : args)
          total_chars += a.length() + 1;
        
        auto argstrs = std::make_unique<char[]>(total_chars);
        auto argv = std::make_unique<char*[]>(2 + args.size());
        
        // copy arguments in
        char* ps = argstrs.get();
        char** pp = argv.get();
        
        // copy in argv[0]
        *pp++ = ps;
        ps = std::copy(program.begin(), program.end(), ps);
        *ps++ = '\0';
        // copy in argv[1:]
        for (std::string_view a : args) {
          *pp++ = ps;
          ps = std::copy(a.begin(), a.end(), ps);
          *ps++ = '\0';
        }
        
        if (execvp(argv[0], argv.get()) == -1) {
          throw std::system_error(errno, std::generic_category());
        }
      }
      else {
        pid = val;
      }
    }
    
    bool joinable() {
      int stat;
      pid_t res = waitpid(pid, &stat, WNOHANG);
      
      if (res == -1) {
        if (errno == ECHILD) {
          return false;
        }
        throw std::system_error(errno, std::generic_category());
      }
      
      return res != 0;
    }
    
    // Wait for the process to close. Throws if the process has
    // already finished. Returns the exit code.
    int join() {
      int stat;
      while (true) {
        if (waitpid(pid, &stat, 0) == -1)  {
          throw std::system_error(errno, std::generic_category());
        }
        
        if (WIFEXITED(stat)) {
          int res = WEXITSTATUS(stat);
          rc = res;
          return res;
        }
        if (WIFSIGNALED(stat)) {
          int res = -WTERMSIG(stat);
          rc = res;
          return res;
        }
      }
    }
    
    // Get the process's exit code, if it has closed.
    // If the process was terminated by signal, return the signal's number, negated.
    int exit_code() {
      return rc.value();
    }
    
  private:
    pid_t pid;
    std::optional<int> rc;
  };
  #pragma endregion
#elif defined(OSLIB_OS_WIN32)
  #pragma region WinAPI implementation
  class process {
  public:
    process(std::string_view program, std::initializer_list<std::string_view> args = {}) : rc(std::nullopt) {
    }
    
    bool joinable() {}
    
    // Wait for the process to close. Throws if the process has
    // already finished. Returns the exit code.
    int join() {}
    
    // Get the process's exit code, if it has closed.
    // If the process was terminated by signal, return the signal's number, negated.
    int exit_code() {
      return rc.value();
    }
    
  private:
    HANDLE hnd;
    std::optional<int> rc;
  };
  #pragma endregion
#endif

#endif