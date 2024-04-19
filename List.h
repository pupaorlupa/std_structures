#include <pthread.h>

#include <iostream>
#include <memory>
#include <new>
#include <tuple>

template <size_t N>
class StackStorage {
 public:
  StackStorage(){};
  StackStorage& operator=(const StackStorage<N>& other) = delete;
  StackStorage(StackStorage& other) = delete;

  char storage[N];
  size_t shift = 0;

  void* align(size_t alignment, size_t bytes) {
    void* p = static_cast<void*>(storage + shift);
    size_t left = N - 1 - shift;
    std::align(alignment, bytes, p, left);
    shift = N - left + bytes;
    return p;
  }
};

template <typename T, size_t N>
class StackAllocator {
 public:
  typedef T value_type;

  template <typename U, size_t K>
  friend class StackAllocator;

  StackAllocator(): memory(&StackStorage<N>()) {};

  explicit StackAllocator(StackStorage<N>& storage): memory(&storage) {}

  ~StackAllocator() = default;

  StackAllocator& operator=(const StackAllocator& other) {
    StackAllocator<T, N> copy = other;
    swap(copy);
    return *this;
  }

  template <typename U>
  StackAllocator<T, N>& operator=(const StackAllocator<U, N>& other) {
    StackAllocator<U, N> copy = other;
    swap(copy);
    return *this;
  }

  template <typename U, size_t K>
  bool operator==(const StackAllocator<U, K>& other) {
    return memory == other.memory;
  }

  template <typename U, size_t K>
  bool operator!=(const StackAllocator<U, K>& other) {
    return memory != other.memory;
  }

  T* allocate(size_t count) {
    return reinterpret_cast<T*>(memory->align(alignment, sizeof(T) * count));
  }
  void deallocate(T* ptr, size_t) { std::ignore = ptr; }

  template <typename U>
  struct rebind {
    using other = StackAllocator<U, N>;
  };

  template <typename U>
  explicit StackAllocator(const StackAllocator<U, N>& other) : memory(other.memory) {}

 private:
  void swap(StackAllocator& other) {
    std::swap(memory, other.memory);
  }

  template <typename U>
  void swap(StackAllocator<U, N>& other) {
    std::swap(memory, other.memory);
  }
  
  StackStorage<N>* memory;
  static constexpr const size_t alignment = alignof(T);
};

//*********************************************************************************
//*********************************************************************************
//*********************************************************************************

template <typename T, class Alloc = std::allocator<T>>
class List {
 private:
  struct BaseNode;
  struct Node;

 public:
  List();
  explicit List(size_t count);
  List(size_t count, const T& value);
  explicit List(const Alloc& allocator);
  List(size_t count, const Alloc& allocator);
  List(size_t count, const T& value, const Alloc& allocator);

  List(const List<T, Alloc>& other);

  List& operator=(const List<T, Alloc>& other);

  ~List();

  Alloc get_allocator() const;
  size_t size() const;
  void clear();
  void push_back(const T& value);
  void push_front(const T& value);
  void pop_back();
  void pop_front();

  template <typename U>
  class base_iterator {
   public:
    friend class List;

    using value_type = U;
    using pointer = value_type*;
    using difference_type = int;
    using reference = value_type&;
    using iterator_category = std::bidirectional_iterator_tag;

    reference operator*() const;
    base_iterator& operator++();
    base_iterator operator++(int);
    base_iterator& operator--();
    base_iterator operator--(int);
    pointer operator->();
    operator base_iterator<const U>() const {
      return base_iterator<const U>(current_node);
    }
    operator base_iterator<U>() {
      return base_iterator<U>(current_node);
    }
    bool operator==(const base_iterator<U>& other) const;
    bool operator!=(const base_iterator<U>& other) const;

   private:
    explicit base_iterator(const BaseNode* base);
    explicit base_iterator(const Node* base);
    BaseNode* current_node;
  };

  using const_iterator = base_iterator<const T>;
  using iterator = base_iterator<T>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  iterator begin();
  iterator end();
  const_iterator begin() const;
  const_iterator end() const;
  const_iterator cbegin() const;
  const_iterator cend() const;
  std::reverse_iterator<iterator> rbegin();
  std::reverse_iterator<const_iterator> rbegin() const;
  std::reverse_iterator<const_iterator> crbegin() const;
  std::reverse_iterator<iterator> rend();
  std::reverse_iterator<const_iterator> rend() const;
  std::reverse_iterator<const_iterator> crend() const;

  iterator insert(const iterator& given_it, const T& value);
  const_iterator insert(const const_iterator& given_it, const T& value);
  void erase(const iterator& given_it);
  void erase(const const_iterator& given_it);

 private:
  template <bool with_args>
  iterator basic_insert(const iterator& given_it, const T* args); // cекретный инсерт только для избранных
  struct BaseNode {
    BaseNode* prev_;
    BaseNode* next_;

    explicit BaseNode(BaseNode* first, BaseNode* second) : prev_(first), next_(second) {}
    BaseNode() = default;
  };

  struct Node : BaseNode {
    T value;

    Node(const T& value) : value(value) {}
    Node() = default;
  };

  using Traits = std::allocator_traits<Alloc>;
  using NodeAlloc =
      typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
  using NodeTraits = std::allocator_traits<NodeAlloc>;

  BaseNode fake_;
  NodeAlloc node_alloc;
  size_t sz_ = 0;
};

template <typename T, class Alloc>
template <typename U>
List<T, Alloc>::template base_iterator<U>::base_iterator(const BaseNode* base)
    : current_node(const_cast<BaseNode*>(base)) {}

template <typename T, class Alloc>
template <typename U>
List<T, Alloc>::template base_iterator<U>::base_iterator(const Node* base)
    : current_node(static_cast<BaseNode*>(const_cast<Node*>(base))) {}

template <typename T, class Alloc>
typename List<T, Alloc>::iterator List<T, Alloc>::begin() {
  return ++iterator(&fake_);
}

template <typename T, class Alloc>
typename List<T, Alloc>::iterator List<T, Alloc>::end() {
  return iterator(&fake_);
}

template <typename T, class Alloc>
typename List<T, Alloc>::const_iterator List<T, Alloc>::begin() const {
  return ++const_iterator(&fake_);
}

template <typename T, class Alloc>
typename List<T, Alloc>::const_iterator List<T, Alloc>::end() const {
  return const_iterator(&fake_);
}

template <typename T, class Alloc>
typename List<T, Alloc>::const_iterator List<T, Alloc>::cbegin() const {
  return ++const_iterator(&fake_);
}

template <typename T, class Alloc>
typename List<T, Alloc>::const_iterator List<T, Alloc>::cend() const {
  return const_iterator(&fake_);
}

template <typename T, class Alloc>
std::reverse_iterator<typename List<T, Alloc>::iterator>
List<T, Alloc>::rbegin() {
  return std::reverse_iterator<typename List<T, Alloc>::iterator>(end());
}

template <typename T, class Alloc>
std::reverse_iterator<typename List<T, Alloc>::const_iterator>
List<T, Alloc>::rbegin() const {
  return std::reverse_iterator<typename List<T, Alloc>::const_iterator>(end());
}

template <typename T, class Alloc>
std::reverse_iterator<typename List<T, Alloc>::iterator>
List<T, Alloc>::rend() {
  return std::reverse_iterator<typename List<T, Alloc>::iterator>(begin());
}

template <typename T, class Alloc>
std::reverse_iterator<typename List<T, Alloc>::const_iterator>
List<T, Alloc>::crbegin() const {
  return std::reverse_iterator<typename List<T, Alloc>::const_iterator>(cend());
}

template <typename T, class Alloc>
std::reverse_iterator<typename List<T, Alloc>::const_iterator>
List<T, Alloc>::crend() const {
  return std::reverse_iterator<typename List<T, Alloc>::const_iterator>(
      cbegin());
}

template <typename T, class Alloc>
template <typename U>
typename List<T, Alloc>::template base_iterator<U>&
    List<T, Alloc>::template base_iterator<U>::operator++() {
  current_node = current_node->next_;
  return *this;
}

template <typename T, class Alloc>
template <typename U>
typename List<T, Alloc>::template base_iterator<U>&
    List<T, Alloc>::template base_iterator<U>::operator--() {
  current_node = current_node->prev_;
  return *this;
}

template <typename T, class Alloc>
template <typename U>
typename List<T, Alloc>::template base_iterator<U>
    List<T, Alloc>::template base_iterator<U>::operator++(int) {
  base_iterator<U> copy(current_node);
  current_node = current_node->next_;
  return copy;
}

template <typename T, class Alloc>
template <typename U>
typename List<T, Alloc>::template base_iterator<U>
    List<T, Alloc>::template base_iterator<U>::operator--(int) {
  base_iterator<U> copy(current_node);
  current_node = current_node->prev_;
  return copy;
}

template <typename T, class Alloc>
template <typename U>
typename List<T, Alloc>::template base_iterator<U>::reference
    List<T, Alloc>::template base_iterator<U>::operator*() const {
  return (static_cast<Node*>(current_node))->value;
}

template <typename T, class Alloc>
template <typename U>
typename List<T, Alloc>::template base_iterator<U>::pointer
    List<T, Alloc>::template base_iterator<U>::operator->() {
  return &(static_cast<Node*>(current_node))->value;
}

template <typename T, class Alloc>
typename List<T, Alloc>::iterator List<T, Alloc>::insert(
    const typename List<T, Alloc>::iterator& given_it, const T& value) {
  return basic_insert<true>(given_it, &value);
}

template <typename T, class Alloc>
typename List<T, Alloc>::const_iterator List<T, Alloc>::insert(
    const typename List<T, Alloc>::const_iterator& given_it, const T& value) {
  return const_iterator(basic_insert<true>(iterator(given_it), value));
}

template <typename T, class Alloc>
template <bool with_args>
typename List<T, Alloc>::iterator List<T, Alloc>::basic_insert(
    const typename List<T, Alloc>::iterator& given_it, const T* value) {
  BaseNode* next = given_it.current_node;
  Node* new_node = NodeTraits::allocate(node_alloc, 1);
  try {
    if constexpr (with_args) {
      NodeTraits::construct(node_alloc, new_node, *value);
    } else if constexpr (!with_args) {
      NodeTraits::construct(node_alloc, new_node);
    }
  } catch (...) {
    NodeTraits::deallocate(node_alloc, new_node, 1);
    throw;
  }
  ++sz_;
  new_node->next_ = next;
  new_node->prev_ = given_it.current_node->prev_;
  given_it.current_node->prev_->next_ = new_node;
  given_it.current_node->prev_ = new_node;
  return typename List<T, Alloc>::iterator(new_node);
}

template <typename T, class Alloc>
void List<T, Alloc>::erase(const typename List<T, Alloc>::iterator& given_it) {
  erase(const_iterator(given_it));
}

template <typename T, class Alloc>
void List<T, Alloc>::erase(
    const typename List<T, Alloc>::const_iterator& given_it) {
  given_it.current_node->next_->prev_ = given_it.current_node->prev_;
  given_it.current_node->prev_->next_ = given_it.current_node->next_;
  NodeTraits::destroy(node_alloc, static_cast<Node*>(given_it.current_node));
  NodeTraits::deallocate(node_alloc, static_cast<Node*>(given_it.current_node),
                         1);
  --sz_;
}

template <typename T, class Alloc>
void List<T, Alloc>::pop_back() {
  erase(--end());
}

template <typename T, class Alloc>
void List<T, Alloc>::pop_front() {
  erase(begin());
}

template <typename T, class Alloc>
void List<T, Alloc>::push_front(const T& value) {
  basic_insert<true>(begin(), &value);
}

template <typename T, class Alloc>
void List<T, Alloc>::push_back(const T& value) {
  basic_insert<true>(end(), &value);
}

template <typename T, class Alloc>
List<T, Alloc>::List() : fake_(BaseNode(&fake_, &fake_)) {}

template <typename T, class Alloc>
void List<T, Alloc>::clear() {
  size_t sz = sz_;
  for (size_t i = 0; i < sz; ++i) {
    pop_back();
  }
}

template <typename T, class Alloc>
size_t List<T, Alloc>::size() const {
  return sz_;
}

template <typename T, class Alloc>
List<T, Alloc>::List(size_t count) : fake_(BaseNode(&fake_, &fake_)) {
  iterator cur = begin();
  try {
    for (size_t i = 0; i < count; ++i) {
      basic_insert<false>(cur, nullptr);
    }
  } catch (...) {
    clear();
    throw;
  }
}

template <typename T, class Alloc>
List<T, Alloc>::List(size_t count, const Alloc& given_al)
    : fake_(BaseNode(&fake_, &fake_)),
      node_alloc(
          std::allocator_traits<Alloc>::select_on_container_copy_construction(
              given_al)) {
  iterator cur = begin();
  try {
    for (size_t i = 0; i < count; ++i) {
      basic_insert<false>(cur, nullptr);
    }
  } catch (...) {
    clear();
    throw;
  }
}

template <typename T, class Alloc>
List<T, Alloc>::List(size_t count, const T& value, const Alloc& given_al)
    : fake_(BaseNode(&fake_, &fake_)),
      node_alloc(
          std::allocator_traits<Alloc>::select_on_container_copy_construction(
              given_al)) {
  iterator cur = begin();
  try {
    for (int i = 0; i < count; ++i) {
      basic_insert<true>(cur, &value);
    }
  } catch (...) {
    clear();
    throw;
  }
}

template <typename T, class Alloc>
List<T, Alloc>::List(size_t count, const T& value) 
    : fake_(BaseNode(&fake_, &fake_)) {
  iterator cur = begin();
  try {
    for (int i = 0; i < count; ++i) {
      basic_insert<true>(cur, &value);
    }
  } catch (...) {
    clear();
    throw;
  }
}

template <typename T, class Alloc>
List<T, Alloc>::List(const Alloc& other_alloc)
    : fake_(BaseNode(&fake_, &fake_)), node_alloc(other_alloc) {}

template <typename T, class Alloc>
Alloc List<T, Alloc>::get_allocator() const {
  return node_alloc;
}

template <typename T, class Alloc>
List<T, Alloc>::~List() {
  clear();
}

template <typename T, class Alloc>
List<T, Alloc>::List(const List& other)
    : fake_(BaseNode(&fake_, &fake_)),
      node_alloc(std::allocator_traits<NodeAlloc>::
                     select_on_container_copy_construction(other.node_alloc)) {
  try {
    for (List::const_iterator iter = other.begin(); iter != other.end();
         ++iter) {
      push_back(*iter);
    }
  } catch (...) {
    clear();
    throw;
  }
}

template <typename T, class Alloc>
List<T, Alloc>& List<T, Alloc>::operator=(const List& other) {
  if (this == &other) {
    return *this;
  }
  BaseNode old_fake = fake_;
  size_t old_sz = sz_;
  NodeAlloc old_alloc = node_alloc;

  if (std::allocator_traits<
          Alloc>::propagate_on_container_copy_assignment::value) {
    node_alloc = other.node_alloc;
  }
  fake_ = BaseNode(&fake_, &fake_);
  sz_ = 0;
  try {
    for (List::const_iterator iter = other.begin(); iter != other.end();
         ++iter) {
      push_back(*iter);
    }
  } catch (...) {
    clear();
    fake_ = old_fake;
    node_alloc = old_alloc;
    sz_ = old_sz;
    throw;
  }
  BaseNode new_fake = fake_;
  size_t new_sz = sz_;
  NodeAlloc new_alloc = node_alloc;
  fake_ = old_fake;
  sz_ = old_sz;
  clear();
  fake_ = new_fake;
  sz_ = new_sz;
  node_alloc = new_alloc;
  return *this;
}

template <typename T, class Alloc>
template <typename U>
bool List<T, Alloc>::template base_iterator<U>::operator==(
    const typename List<T, Alloc>::template base_iterator<U>& other) const {
  return current_node == other.current_node;
}

template <typename T, class Alloc>
template <typename U>
bool List<T, Alloc>::template base_iterator<U>::operator!=(
    const typename List<T, Alloc>::template base_iterator<U>& other) const {
  return current_node != other.current_node;
}
