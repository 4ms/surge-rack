#pragma once

// The MetaModule's libstdc++ is built without gthreads, so <mutex> does not
// provide std::mutex or std::recursive_mutex. This header provides spinlock
// versions (it's force-included by metamodule_prefix.hh).
//
// recursive_mutex does not track its owner: re-locking from the same context
// works, but it does not exclude a different context that locks it while held.

#include <cstdint>
#include <mutex>

namespace std
{

struct mutex
{
    constexpr mutex() noexcept = default;
    mutex(const mutex &) = delete;
    mutex &operator=(const mutex &) = delete;

    void lock()
    {
        while (__atomic_test_and_set(&flag, __ATOMIC_ACQUIRE))
        {
        }
    }

    bool try_lock() { return !__atomic_test_and_set(&flag, __ATOMIC_ACQUIRE); }

    void unlock() { __atomic_clear(&flag, __ATOMIC_RELEASE); }

  private:
    volatile bool flag{false};
};

struct recursive_mutex
{
    constexpr recursive_mutex() noexcept = default;
    recursive_mutex(const recursive_mutex &) = delete;
    recursive_mutex &operator=(const recursive_mutex &) = delete;

    void lock() { depth++; }

    bool try_lock()
    {
        depth++;
        return true;
    }

    void unlock() { depth--; }

  private:
    uint32_t depth{0};
};

} // namespace std
