#pragma once

#include <forward_list>
#include <memory>
#include <mutex>
#include <vector>

template<class T, class... Args>
std::unique_ptr<T> make_unique_nothrow(Args &&...args) noexcept(noexcept(T(std::forward<Args>(args)...))) {
    return std::unique_ptr<T>(new (std::nothrow) T(std::forward<Args>(args)...));
}

template<typename T>
inline typename std::unique_ptr<T> make_unique_nothrow(size_t num) noexcept {
    return std::unique_ptr<T>(new (std::nothrow) std::remove_extent_t<T>[num]());
}

template<class T, class... Args>
std::shared_ptr<T> make_shared_nothrow(Args &&...args) noexcept(noexcept(T(std::forward<Args>(args)...))) {
    return std::shared_ptr<T>(new (std::nothrow) T(std::forward<Args>(args)...));
}

template<typename T, class Allocator, class Predicate>
bool remove_locked_first_if(std::mutex &mutex, std::forward_list<T, Allocator> &list, Predicate pred) {
    std::lock_guard<std::mutex> lock(mutex);
    auto oit = list.before_begin(), it = std::next(oit);
    while (it != list.end()) {
        if (pred(*it)) {
            list.erase_after(oit);
            return true;
        }
        oit = it++;
    }
    return false;
}

// those work only in powers of 2
#define ROUNDDOWN(val, align) ((val) & ~(align - 1))
#define ROUNDUP(val, align)   ROUNDDOWN(((val) + (align - 1)), align)

bool GetTitleIdOfDisc(uint64_t *titleId, bool *discPresent);