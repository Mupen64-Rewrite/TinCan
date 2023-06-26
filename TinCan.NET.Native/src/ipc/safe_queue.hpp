#ifndef TC_IPC_SAFE_QUEUE_HPP
#define TC_IPC_SAFE_QUEUE_HPP

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <queue>

namespace tc {
  template <class T, class Container = std::deque<T>>
  class safe_queue : public std::queue<T, Container> {
    using base_t = std::queue<T, Container>;
  public:
    using container_type  = Container;
    using value_type      = typename Container::value_type;
    using size_type       = typename Container::size_type;
    using reference       = typename Container::reference;
    using const_reference = typename Container::const_reference;

    using base_t::queue;
    
    safe_queue(const safe_queue& rhs) : base_t(rhs), m_mutex(), m_cv() {}
    safe_queue& operator=(const safe_queue& rhs) {
      return base_t::operator=(rhs);
    }
    
    safe_queue(safe_queue&& rhs) : base_t(rhs), m_mutex(), m_cv() {}
    safe_queue& operator=(safe_queue&& rhs) {
      return base_t::operator=(rhs);
    }

    reference front() {
      std::unique_lock lock(m_mutex);
      m_cv.wait(lock, [this] { return !base_t::empty(); });
      return base_t::front();
    }

    reference back() {
      std::unique_lock lock(m_mutex);
      m_cv.wait(lock, [this] { return !base_t::empty(); });
      return base_t::back();
    }

    using base_t::empty;
    using base_t::size;

    void push(const value_type& value) {
      {
        std::lock_guard lock(m_mutex);
        base_t::push(value);
      }
      m_cv.notify_one();
    }

    template <class... Args>
    decltype(auto) emplace(Args&&... args) {
      {
        std::lock_guard lock(m_mutex);
        base_t::emplace(std::forward<Args>(args)...);
      }
      m_cv.notify_one();
    }

    void pop() {
      std::lock_guard lock(m_mutex);
      base_t::pop();
    }
    
    void pop_return(reference ref) {
      std::lock_guard lock(m_mutex);
      ref = std::move(std::queue<T, Container>::front());
      base_t::pop();
    }

    void swap(safe_queue& rhs) {
      // swapping with self does nothing
      if (this == std::addressof(rhs))
        return;
      // lock both mutexes and swap
      std::scoped_lock lock(m_mutex, rhs.m_mutex);
      base_t::swap(rhs);
    }

  protected:
    using std::queue<T, Container>::c;
  private:
    std::mutex m_mutex {};
    std::condition_variable m_cv {};
  };
  
  template <class T, class Container>
  void swap(safe_queue<T, Container>& a, safe_queue<T, Container>& b) {
    a.swap(b);
  }
}  // namespace tc

#endif