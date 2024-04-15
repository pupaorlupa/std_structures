#include <iostream>
#include <memory>

template <typename T>
class WeakPtr;

template <typename T>
class SharedPtr;

template <typename T>
class EnableSharedFromThis {
 public:
  template <typename U>
  friend class SharedPtr;

  SharedPtr<T> shared_from_this() const noexcept { return wptr_.lock(); }

 protected:
  WeakPtr<T> wptr_;
};

// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------

namespace blocks {
struct BaseControlBlock {
  virtual void useDeleter() = 0;
  virtual void suicide() = 0;
  virtual ~BaseControlBlock() = default;

  int shared_count = 0;
  int weak_count = 0;
};

template <typename U, typename Deleter, typename Alloc>
struct ControlBlockRegular : BaseControlBlock {
  ControlBlockRegular(int sh_count, int w_count, U* pointer, const Deleter& del,
                      const Alloc& allocator);

  void useDeleter() override;
  void suicide() override;
  ~ControlBlockRegular() override = default;

  U* object;
  Deleter deleter;
  Alloc alloc;
};

template <typename U, typename Alloc>
struct ControlBlockMakeShared : BaseControlBlock {
  ControlBlockMakeShared(int sh_count, int w_count, Alloc allocator);

  void useDeleter() override;
  void suicide() override;
  ~ControlBlockMakeShared() override = default;

  Alloc alloc;
  alignas(U) int8_t object[sizeof(U)];
};
}  // namespace blocks

template <typename U, typename Deleter, typename Alloc>
blocks::ControlBlockRegular<U, Deleter, Alloc>::ControlBlockRegular(
    int sh_count, int w_count, U* pointer, const Deleter& del,
    const Alloc& allocator)
    : object(pointer), deleter(del), alloc(allocator) {
  shared_count = sh_count;
  weak_count = w_count;
}

template <typename U, typename Deleter, typename Alloc>
void blocks::ControlBlockRegular<U, Deleter, Alloc>::useDeleter() {
  deleter(object);
}

template <typename U, typename Deleter, typename Alloc>
void blocks::ControlBlockRegular<U, Deleter, Alloc>::suicide() {
  using CRBalloc = typename std::allocator_traits<Alloc>::template rebind_alloc<
      ControlBlockRegular<U, Deleter, Alloc>>;
  CRBalloc literally_cbralloc;
  auto old_this = this;
  std::allocator_traits<CRBalloc>::deallocate(literally_cbralloc, old_this, 1);
}

template <typename U, typename Alloc>
blocks::ControlBlockMakeShared<U, Alloc>::ControlBlockMakeShared(
    int sh_count, int w_count, Alloc allocator)
    : alloc(allocator) {
  shared_count = sh_count;
  weak_count = w_count;
}

template <typename U, typename Alloc>
void blocks::ControlBlockMakeShared<U, Alloc>::useDeleter() {
  using Talloc =
      typename std::allocator_traits<Alloc>::template rebind_alloc<U>;
  Talloc literally_taccot(alloc);
  std::allocator_traits<Talloc>::destroy(literally_taccot,
                                         reinterpret_cast<U*>(&object));
}

template <typename U, typename Alloc>
void blocks::ControlBlockMakeShared<U, Alloc>::suicide() {
  auto old_this = this;
  using CBMSalloc = typename std::allocator_traits<
      Alloc>::template rebind_alloc<ControlBlockMakeShared<U, Alloc>>;
  CBMSalloc literally_cbmsaccot(alloc);
  std::allocator_traits<CBMSalloc>::deallocate(literally_cbmsaccot, old_this,
                                               1);
}

// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------

template <typename T, typename Alloc, typename... Args>
SharedPtr<T> allocateShared(const Alloc& alloc, Args&&... args) {
  using AllocSharedControlBlock =
      typename std::allocator_traits<Alloc>::template rebind_alloc<
          typename blocks::template ControlBlockMakeShared<T, Alloc>>;
  AllocSharedControlBlock cb_alloc(alloc);
  typename blocks::template ControlBlockMakeShared<T, Alloc>* cb =
      std::allocator_traits<AllocSharedControlBlock>::allocate(cb_alloc, 1);
  using Talloc =
      typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
  Talloc literally_taccot(alloc);
  new (cb) blocks::ControlBlockMakeShared<T, Alloc>(1, 0, literally_taccot);
  std::allocator_traits<Talloc>::construct(literally_taccot,
                                           reinterpret_cast<T*>(&cb->object),
                                           std::forward<Args>(args)...);
  return SharedPtr<T>(static_cast<blocks::BaseControlBlock*>(cb),
                      reinterpret_cast<T*>(&cb->object));
}

template <typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args) {
  return allocateShared<T, std::allocator<T>>(std::allocator<T>(),
                                              std::forward<Args>(args)...);
}

// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------

template <typename T>
class SharedPtr {
 public:
  template <typename U>
  friend class WeakPtr;
  template <typename U, typename Alloc, typename... Args>
  friend SharedPtr<U> allocateShared(const Alloc& alloc, Args&&... args);

  template <typename U>
  friend class SharedPtr;

  SharedPtr(blocks::BaseControlBlock* given_cb, T* pointer);

  template <typename U>
  SharedPtr(blocks::BaseControlBlock* given_cb, U* pointer);

  SharedPtr();

  explicit SharedPtr(T* ptr);

  template <typename Deleter>
  SharedPtr(T* ptr, const Deleter& del);

  template <typename U, typename Deleter>
  SharedPtr(U* ptr, const Deleter& del);

  template <typename Deleter, typename Alloc>
  SharedPtr(T* ptr, const Deleter& deleter, const Alloc& allocator);

  template <typename U, typename Deleter, typename Alloc>
  SharedPtr(U* ptr, const Deleter& deleter, const Alloc& allocator);

  template <typename U, typename = std::enable_if_t<std::is_same_v<T, U> ||
                                                    std::is_base_of_v<T, U>>>
  SharedPtr(const SharedPtr<U>& other);

  SharedPtr(const SharedPtr& other);

  template <typename U, typename = std::enable_if_t<std::is_same_v<T, U> ||
                                                    std::is_base_of_v<T, U>>>
  SharedPtr(SharedPtr<U>&& other) noexcept;

  SharedPtr(SharedPtr&& other) noexcept;

  template <typename U, typename = std::enable_if_t<std::is_same_v<T, U> ||
                                                    std::is_base_of_v<T, U>>>
  SharedPtr& operator=(SharedPtr<U>&& other) noexcept;

  SharedPtr& operator=(SharedPtr&& other) noexcept;

  template <typename U, typename = std::enable_if_t<std::is_same_v<T, U> ||
                                                    std::is_base_of_v<T, U>>>
  SharedPtr& operator=(const SharedPtr<U>& other);

  SharedPtr& operator=(const SharedPtr& other);

  void reset(T* new_ptr);

  void reset();

  T& operator*() const;

  T* operator->() const;

  T* get() const;

  template <typename U, typename = std::enable_if_t<std::is_same_v<T, U> ||
                                                    std::is_base_of_v<T, U>>>
  void swap(SharedPtr<U>& other);

  void swap(SharedPtr& other);

  ~SharedPtr();

  int use_count() const;

 private:
  void real_destructor();

  T* ptr;
  blocks::BaseControlBlock* cb;
};

template <typename T>
class WeakPtr {
 public:
  template <typename U>
  friend class WeakPtr;

  WeakPtr();

  template <typename U, typename = std::enable_if_t<std::is_same_v<T, U> ||
                                                    std::is_base_of_v<T, U>>>
  WeakPtr(const SharedPtr<U>& pointer);

  WeakPtr(const SharedPtr<T>& pointer);

  template <typename U, typename = std::enable_if_t<std::is_same_v<T, U> ||
                                                    std::is_base_of_v<T, U>>>
  WeakPtr(const WeakPtr<U>& other);

  WeakPtr(const WeakPtr& other);

  template <typename U, typename = std::enable_if_t<std::is_same_v<T, U> ||
                                                    std::is_base_of_v<T, U>>>
  WeakPtr(WeakPtr&& other);

  template <typename U, typename = std::enable_if_t<std::is_same_v<T, U> ||
                                                    std::is_base_of_v<T, U>>>
  WeakPtr& operator=(const WeakPtr<U>& other);

  WeakPtr& operator=(const WeakPtr& other);

  bool expired() const;

  int use_count() const;

  SharedPtr<T> lock() const;

  ~WeakPtr();

 private:
  void real_destructor();

  T* ptr;
  typename blocks::BaseControlBlock* cb;
};

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

template <typename T>
SharedPtr<T>::SharedPtr(blocks::BaseControlBlock* given_cb, T* pointer)
    : ptr(pointer), cb(given_cb){};

template <typename T>
template <typename U>
SharedPtr<T>::SharedPtr(blocks::BaseControlBlock* given_cb, U* pointer)
    : ptr(pointer), cb(given_cb){};

template <typename T>
SharedPtr<T>::SharedPtr()
    : SharedPtr(nullptr, std::default_delete<T>(), std::allocator<T>()) {}

template <typename T>
SharedPtr<T>::SharedPtr(T* ptr)
    : SharedPtr(ptr, std::default_delete<T>(), std::allocator<T>()) {}

template <typename T>
template <typename Deleter>
SharedPtr<T>::SharedPtr(T* ptr, const Deleter& del)
    : SharedPtr(ptr, del, std::allocator<T>()) {}

template <typename T>
template <typename U, typename Deleter>
SharedPtr<T>::SharedPtr(U* ptr, const Deleter& del)
    : SharedPtr(ptr, del, std::allocator<U>()) {}

template <typename T>
template <typename Deleter, typename Alloc>
SharedPtr<T>::SharedPtr(T* ptr, const Deleter& deleter, const Alloc& allocator)
    : ptr(ptr) {
  if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
    ptr->wptr_ = this;
  }
  using CBRalloc = typename std::allocator_traits<Alloc>::template rebind_alloc<
      blocks::ControlBlockRegular<T, Deleter, Alloc>>;
  CBRalloc literally_cbralloc;
  blocks::ControlBlockRegular<T, Deleter, Alloc>* cbr =
      std::allocator_traits<CBRalloc>::allocate(literally_cbralloc, 1);
  new (cbr) blocks::ControlBlockRegular<T, Deleter, Alloc>(1, 0, ptr, deleter,
                                                           allocator);
  cb = cbr;
}

template <typename T>
template <typename U, typename Deleter, typename Alloc>
SharedPtr<T>::SharedPtr(U* ptr, const Deleter& deleter, const Alloc& allocator)
    : ptr(ptr) {
  if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
    ptr->wptr_ = this;
  }
  using CBRalloc = typename std::allocator_traits<Alloc>::template rebind_alloc<
      blocks::ControlBlockRegular<U, Deleter, Alloc>>;
  CBRalloc literally_cbralloc;
  blocks::ControlBlockRegular<U, Deleter, Alloc>* cbr =
      std::allocator_traits<CBRalloc>::allocate(literally_cbralloc, 1);
  new (cbr) blocks::ControlBlockRegular<U, Deleter, Alloc>(1, 0, ptr, deleter,
                                                           allocator);
  cb = cbr;
}

template <typename T>
template <typename U, typename>
SharedPtr<T>::SharedPtr(const SharedPtr<U>& other)
    : ptr(other.ptr), cb(other.cb) {
  if (cb != nullptr) {
    ++cb->shared_count;
  }
}

template <typename T>
SharedPtr<T>::SharedPtr(const SharedPtr& other) : ptr(other.ptr), cb(other.cb) {
  if (cb != nullptr) {
    ++cb->shared_count;
  }
}

template <typename T>
template <typename U, typename>
SharedPtr<T>::SharedPtr(SharedPtr<U>&& other) noexcept : ptr(other.ptr), cb(other.cb) {
  other.ptr = nullptr;
  other.cb = nullptr;
}

template <typename T>
SharedPtr<T>::SharedPtr(SharedPtr&& other) noexcept : ptr(other.ptr), cb(other.cb) {
  other.ptr = nullptr;
  other.cb = nullptr;
}

template <typename T>
template <typename U, typename>
SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr<U>&& other) noexcept {
  real_destructor();
  ptr = other.ptr;
  cb = other.cb;
  other.ptr = nullptr;
  other.cb = nullptr;
  return *this;
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr&& other) noexcept {
  real_destructor();
  ptr = other.ptr;
  cb = other.cb;
  other.ptr = nullptr;
  other.cb = nullptr;
  return *this;
}

template <typename T>
template <typename U, typename>
SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr<U>& other) {
  real_destructor();
  ptr = other.ptr;
  cb = other.cb;
  if (other.cb != nullptr) {
    ++cb->shared_count;
  }
  return *this;
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr& other) {
  if (this == &other) {
    return *this;
  }
  real_destructor();
  ptr = other.ptr;
  cb = other.cb;
  if (other.cb != nullptr) {
    ++cb->shared_count;
  }
  return *this;
}

template <typename T>
void SharedPtr<T>::reset(T* new_ptr) {
  real_destructor();
  ptr = new_ptr;
  using Deleter = std::default_delete<T>;
  using Alloc = std::allocator<T>;
  Deleter deleter;
  Alloc allocator;
  using CBRalloc = typename std::allocator_traits<Alloc>::template rebind_alloc<
      blocks::ControlBlockRegular<T, Deleter, Alloc>>;
  CBRalloc literally_cbralloc;
  blocks::ControlBlockRegular<T, Deleter, Alloc>* cbr =
      std::allocator_traits<CBRalloc>::allocate(literally_cbralloc, 1);
  std::allocator_traits<CBRalloc>::construct(literally_cbralloc, cbr, 1, 0, ptr,
                                             deleter, allocator);
  cb = cbr;
}

template <typename T>
void SharedPtr<T>::reset() {
  real_destructor();
  cb = nullptr;
  ptr = nullptr;
}

template <typename T>
T& SharedPtr<T>::operator*() const {
  return *ptr;
}

template <typename T>
T* SharedPtr<T>::operator->() const {
  return ptr;
}

template <typename T>
T* SharedPtr<T>::get() const {
  return ptr;
}

template <typename T>
template <typename U, typename>
void SharedPtr<T>::swap(SharedPtr<U>& other) {
  auto temp_cb = other.cb;
  auto temp_ptr = other.ptr;
  other.ptr = ptr;
  other.cb = cb;
  cb = temp_cb;
  ptr = temp_ptr;
}

template <typename T>
void SharedPtr<T>::swap(SharedPtr& other) {
  auto temp_cb = other.cb;
  auto temp_ptr = other.ptr;
  other.ptr = ptr;
  other.cb = cb;
  cb = temp_cb;
  ptr = temp_ptr;
}

template <typename T>
SharedPtr<T>::~SharedPtr() {
  real_destructor();
}

template <typename T>
int SharedPtr<T>::use_count() const {
  if (cb == nullptr) {
    return 0;
  }
  return cb->shared_count;
}

template <typename T>
void SharedPtr<T>::real_destructor() {
  if (cb == nullptr) {
    return;
  }
  --cb->shared_count;
  if (cb->shared_count == 0) {
    cb->useDeleter();
    ptr = nullptr;
    if (cb->weak_count == 0) {
      cb->suicide();
      cb = nullptr;
    }
  }
}

template <typename T>
WeakPtr<T>::WeakPtr() : ptr(nullptr), cb(nullptr) {}

template <typename T>
template <typename U, typename>
WeakPtr<T>::WeakPtr(const SharedPtr<U>& pointer)
    : ptr(pointer.ptr), cb(pointer.cb) {
  ++cb->weak_count;
}

template <typename T>
WeakPtr<T>::WeakPtr(const SharedPtr<T>& pointer)
    : ptr(pointer.ptr), cb(pointer.cb) {
  ++cb->weak_count;
}

template <typename T>
template <typename U, typename>
WeakPtr<T>::WeakPtr(const WeakPtr<U>& other) : ptr(other.ptr), cb(other.cb) {
  ++cb->weak_count;
}

template <typename T>
WeakPtr<T>::WeakPtr(const WeakPtr& other) : ptr(other.ptr), cb(other.cb) {
  ++cb->weak_count;
}

template <typename T>
template <typename U, typename>
WeakPtr<T>::WeakPtr(WeakPtr&& other) : ptr(other.ptr), cb(other.cb) {
  other.ptr = nullptr;
  other.cb = nullptr;
}

template <typename T>
template <typename U, typename>
WeakPtr<T>& WeakPtr<T>::operator=(const WeakPtr<U>& other) {
  real_destructor();
  ptr = other.ptr;
  cb = other.cb;
  if (cb != nullptr) {
    ++cb->weak_count;
  }
  return *this;
}

template <typename T>
WeakPtr<T>& WeakPtr<T>::operator=(const WeakPtr& other) {
  if (this == &other) {
    return *this;
  }
  real_destructor();
  ptr = other.ptr;
  cb = other.cb;
  if (cb != nullptr) {
    ++cb->weak_count;
  }
  return *this;
}

template <typename T>
bool WeakPtr<T>::expired() const {
  return cb->shared_count == 0;
}

template <typename T>
int WeakPtr<T>::use_count() const {
  if (cb == nullptr) {
    return 0;
  }
  return cb->shared_count;
}

template <typename T>
SharedPtr<T> WeakPtr<T>::lock() const {
  ++cb->shared_count;
  return SharedPtr<T>(cb, ptr);
}

template <typename T>
WeakPtr<T>::~WeakPtr() {
  real_destructor();
}

template <typename T>
void WeakPtr<T>::real_destructor() {
  if (cb == nullptr) {
    return;
  }
  --cb->weak_count;
  if (cb->shared_count == 0 && cb->weak_count == 0) {
    cb->suicide();
  }
}
