#ifndef TC_UTIL_GATE_HPP
#define TC_UTIL_GATE_HPP

#include <atomic>
namespace tc {
  class gate {
  public:
    gate(bool unlocked = false) : m_flag() {
      if (unlocked)
        m_flag.test_and_set();
    }
    
    gate(const gate&) = delete;
    gate& operator=(const gate&) = delete;
    
    gate(gate&&) = delete;
    gate& operator=(gate&&) = delete;
  
    void lock() {
      m_flag.clear();
    }
  
    void unlock() {
      if (!m_flag.test_and_set())
        m_flag.notify_all();
    }
    
    void wait() {
      while (!m_flag.test()) {
        m_flag.wait(false);
      }
    }
    
    bool try_wait() {
      return m_flag.test();
    }
  private:
    std::atomic_flag m_flag;
  };
}

#endif