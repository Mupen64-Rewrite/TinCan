#include "backward.hpp"
#include <cstdio>
#include <exception>

namespace backward {
  backward::SignalHandling sh;
}

// Stolen from StackOverflow:
// https://stackoverflow.com/a/4541470/10808912
#ifdef __GNUG__
  #include <cxxabi.h>
  #include <cstdlib>
  #include <memory>

static std::string demangle(const char* name) {
  int status = -4;  // some arbitrary value to eliminate the compiler warning

  // enable c++11 by passing the flag -std=c++11 to g++
  std::unique_ptr<char, void (*)(void*)> res {
    abi::__cxa_demangle(name, NULL, NULL, &status), std::free};

  return (status == 0) ? res.get() : name;
}

#else

// does nothing if not g++
static std::string demangle(const char* name) {
  return name;
}

#endif

// Partly stolen from cppreference:
// https://en.cppreference.com/w/cpp/error/nested_exception#Example
static void print_exception(const std::exception& e, unsigned int level = 0) {
  static const char base_fmt[] = "Exception %s: %s\n";
  static const char sub_fmt[]  = "%2d | Caused by %s: %s\n";

  fprintf(
    stderr, (level == 0) ? base_fmt : sub_fmt, level,
    demangle(typeid(e).name()).c_str(), e.what());
  
  try {
    std::rethrow_if_nested(e);
  } 
  catch (const std::exception& nested) {
    print_exception(nested, level + 1);  
  }
  catch (...) {
    fprintf(stderr, "%2d | Unknown exception object", level + 1);
  }
}

void init_exceptions() {
  std::set_terminate([]() -> void {
    if (std::uncaught_exceptions() > 0) {
      try {
        std::rethrow_exception(std::current_exception());
      }
      catch (const std::exception& exc) {
        print_exception(exc);
      }
      catch (const char* exc) {
        fprintf(stderr, "String thrown: %s", exc);
      }
      catch (...) {
        fputs("Unknown exception object\n", stderr);
      }
    }
    else {
      fputs("terminate() called without an active exception\n", stderr);
    }
    std::exit(1);
  });
};