#pragma once

#if defined(__linux__)
#include <sys/epoll.h>

#elif defined(__APPLE__)
#include <cerrno>
#include <cstdint>
#include <vector>

#include <sys/event.h>
#include <sys/types.h>

/// epoll-compatible types and functions implemented with kqueue on macOS.
struct epoll_event {
  uint32_t events;
  union {
    void *ptr;
  } data;
};

constexpr int EPOLL_CTL_ADD = 1;
constexpr int EPOLL_CTL_DEL = 2;
constexpr int EPOLL_CTL_MOD = 3;

constexpr uint32_t EPOLLIN = 0x001;
constexpr uint32_t EPOLLOUT = 0x004;
constexpr uint32_t EPOLLERR = 0x008;
constexpr uint32_t EPOLLHUP = 0x010;
constexpr uint32_t EPOLLET = 1u << 31;

inline int epoll_create(int /*size_hint*/) { return kqueue(); }

inline int epoll_ctl(int kq, int op, int fd, epoll_event *ev) {
  if (op == EPOLL_CTL_DEL) {
    struct kevent kev[2];
    EV_SET(&kev[0], fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
    EV_SET(&kev[1], fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
    kevent(kq, kev, 2, nullptr, 0, nullptr);
    return 0;
  }

  if (op != EPOLL_CTL_ADD && op != EPOLL_CTL_MOD) {
    errno = EINVAL;
    return -1;
  }

  struct kevent kev[2];
  int n = 0;
  const auto flags = (ev->events & EPOLLET) ? EV_CLEAR : 0;

  if (ev->events & EPOLLIN) {
    EV_SET(&kev[n++], fd, EVFILT_READ, EV_ADD | flags, 0, 0, ev->data.ptr);
  }
  if (ev->events & EPOLLOUT) {
    EV_SET(&kev[n++], fd, EVFILT_WRITE, EV_ADD | flags, 0, 0, ev->data.ptr);
  }
  if (n == 0) {
    errno = EINVAL;
    return -1;
  }

  return kevent(kq, kev, n, nullptr, 0, nullptr) == -1 ? -1 : 0;
}

inline int epoll_wait(int kq, epoll_event *events, int maxevents,
                      int timeout_ms) {
  struct timespec ts{};
  struct timespec *timeout_ptr = nullptr;
  if (timeout_ms >= 0) {
    ts.tv_sec = timeout_ms / 1000;
    ts.tv_nsec = static_cast<long>((timeout_ms % 1000) * 1000000L);
    timeout_ptr = &ts;
  }

  std::vector<struct kevent> kevs(maxevents);
  const int n = kevent(kq, nullptr, 0, kevs.data(), maxevents, timeout_ptr);
  if (n <= 0) {
    return n;
  }

  for (int i = 0; i < n; ++i) {
    events[i].data.ptr = kevs[i].udata;
    events[i].events = 0;
    if (kevs[i].filter == EVFILT_READ) {
      events[i].events |= EPOLLIN;
    }
    if (kevs[i].filter == EVFILT_WRITE) {
      events[i].events |= EPOLLOUT;
    }
    if (kevs[i].flags & EV_EOF) {
      events[i].events |= EPOLLHUP;
    }
    if (kevs[i].fflags) {
      events[i].events |= EPOLLERR;
    }
  }

  return n;
}

#else
#error "epoll is not supported on this platform"
#endif
