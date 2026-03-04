#pragma once

#include <cassert>
#include <memory>
#include <vector>

namespace exchange {

/**
 * A fast, simple pre-allocated Memory Pool for objects of type T.
 * Avoids the overhead of dynamic allocation during the critical path.
 */
template <typename T> class MemoryPool {
public:
  explicit MemoryPool(size_t capacity) : capacity_(capacity), free_index_(0) {
    pool_.resize(capacity);
    free_indices_.resize(capacity);
    for (size_t i = 0; i < capacity; ++i) {
      free_indices_[i] = i;
    }
  }

  // Disallow copy and move for simplicity
  MemoryPool(const MemoryPool &) = delete;
  MemoryPool &operator=(const MemoryPool &) = delete;

  template <typename... Args> T *allocate(Args &&...args) {
    if (free_index_ >= capacity_) {
      return nullptr; // Out of memory
    }
    size_t index = free_indices_[free_index_++];
    T *ptr = &pool_[index];
    // In-place construct
    new (ptr) T(std::forward<Args>(args)...);
    return ptr;
  }

  void deallocate(T *ptr) {
    if (!ptr)
      return;
    // Call destructor
    ptr->~T();
    // Calculate index
    size_t index = ptr - pool_.data();
    assert(index < capacity_);
    // Return to free list
    assert(free_index_ > 0);
    free_indices_[--free_index_] = index;
  }

private:
  size_t capacity_;
  size_t free_index_; // Next available index in free_indices_
  std::vector<T> pool_;
  std::vector<size_t> free_indices_;
};

} // namespace exchange
