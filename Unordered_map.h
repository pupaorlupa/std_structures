#include <cmath>
#include <memory>
#include <new>
#include <tuple>
#include <vector>

template <typename T, class Alloc = std::allocator<T>>
class List {
 private:
  struct BaseNode;
  struct Node;

 public:
  template <class Key, class U, class Hash, class KeyEqual, class Allocator>
  friend class UnorderedMap;

  List();
  explicit List(size_t count);
  List(size_t count, const T &value);
  explicit List(const Alloc &given_al);
  List(size_t count, const Alloc &given_al);
  List(size_t count, const T &value, const Alloc &given_al);

  List(const List<T, Alloc> &other);
  List(List<T, Alloc> &&other) noexcept;

  List &operator=(const List<T, Alloc> &other);
  List &operator=(List<T, Alloc> &&other) noexcept;

  ~List();

  Alloc get_allocator() const;
  size_t size() const;
  void clear();
  void push_back(const T &value);
  void push_front(const T &value);
  void pop_back();
  void pop_front();

  template <typename U>
  class base_iterator {
   public:
    friend class List;

    template <class Key, class K, class Hash, class KeyEqual, class Allocator>
    friend class UnorderedMap;

    using value_type = U;
    using pointer = value_type *;
    using difference_type = int;
    using reference = value_type &;
    using iterator_category = std::bidirectional_iterator_tag;

    reference operator*() const;
    base_iterator &operator++();
    base_iterator operator++(int);
    base_iterator &operator--();
    base_iterator operator--(int);
    pointer operator->();
    operator base_iterator<const U>() const {
      return base_iterator<const U>(current_node);
    }
    bool operator==(const base_iterator<U> &other) const;
    bool operator!=(const base_iterator<U> &other) const;

   private:
    explicit base_iterator(const BaseNode *base);
    explicit base_iterator(const Node *base);
    BaseNode *current_node;
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

  template <typename... Args>
  iterator emplace(const iterator &given_it, Args &&...args);
  template <typename... Args>
  BaseNode *r_emplace(const iterator &given_it, Args &&...args);
  template <typename... Args>
  const_iterator emplace(const const_iterator &given_it, Args &&...args);

  iterator insert(const iterator &given_it, const T &value);
  const_iterator insert(const const_iterator &given_it, const T &value);
  void erase(const iterator &given_it);
  void erase(const const_iterator &given_it);

 private:
  BaseNode *insertNode(const iterator &given_it, BaseNode *new_node);

  iterator forgetNode(const iterator &given_it);

  struct BaseNode {
    BaseNode *prev_;
    BaseNode *next_;

    explicit BaseNode(BaseNode *first, BaseNode *second)
        : prev_(first), next_(second) {}

    BaseNode(BaseNode &&other) noexcept
        : prev_(other.prev_), next_(other.next_) {
      other.prev_ = nullptr;
      other.next_ = nullptr;
    }

    BaseNode(const BaseNode &other) : prev_(other.prev_), next_(other.next_) {}

    BaseNode() = default;

    BaseNode &operator=(const BaseNode &other) {
      if (this == &other) {
        return *this;
      }
      prev_ = other.prev_;
      next_ = other.next_;
      return *this;
    }
  };

  struct Node : BaseNode {
    T value;

    Node() = default;
    explicit Node(const T &value) : value(value) {}
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
List<T, Alloc>::template base_iterator<U>::base_iterator(const BaseNode *base)
    : current_node(const_cast<BaseNode *>(base)) {}

template <typename T, class Alloc>
template <typename U>
List<T, Alloc>::template base_iterator<U>::base_iterator(const Node *base)
    : current_node(static_cast<BaseNode *>(const_cast<Node *>(base))) {}

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
typename List<T, Alloc>::template base_iterator<U>
    &List<T, Alloc>::template base_iterator<U>::operator++() {
  current_node = current_node->next_;
  return *this;
}

template <typename T, class Alloc>
template <typename U>
typename List<T, Alloc>::template base_iterator<U>
    &List<T, Alloc>::template base_iterator<U>::operator--() {
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
  return (static_cast<Node *>(current_node))->value;
}

template <typename T, class Alloc>
template <typename U>
typename List<T, Alloc>::template base_iterator<U>::pointer
    List<T, Alloc>::template base_iterator<U>::operator->() {
  return &(static_cast<Node *>(current_node))->value;
}

template <typename T, class Alloc>
typename List<T, Alloc>::BaseNode *List<T, Alloc>::insertNode(
    const iterator &given_it, BaseNode *new_node) {
  BaseNode *next = given_it.current_node;
  ++sz_;
  new_node->next_ = next;
  new_node->prev_ = given_it.current_node->prev_;
  given_it.current_node->prev_->next_ = new_node;
  given_it.current_node->prev_ = new_node;
  return new_node;
}

template <typename T, class Alloc>
typename List<T, Alloc>::iterator List<T, Alloc>::forgetNode(
    const iterator &given_it) {
  BaseNode *next = given_it.current_node->next_;
  --sz_;
  given_it.current_node->prev_->next_ = next;
  next->prev_ = given_it.current_node->prev_;
  return typename List<T, Alloc>::iterator(next);
}

template <typename T, class Alloc>
template <typename... Args>
typename List<T, Alloc>::iterator
List<T, Alloc>::emplace(  // сдвигает текущую вперед, нашей дает этот номер
    const iterator &given_it, Args &&...args) {
  BaseNode *next = given_it.current_node;
  Node *new_node = NodeTraits::allocate(node_alloc, 1);
  try {
    NodeTraits::construct(node_alloc, new_node, std::forward<Args>(args)...);
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
template <typename... Args>
typename List<T, Alloc>::BaseNode *List<T, Alloc>::r_emplace(
    const iterator &given_it, Args &&...args) {
  BaseNode *next = given_it.current_node;
  Node *new_node = NodeTraits::allocate(node_alloc, 1);
  try {
    NodeTraits::construct(node_alloc, new_node, std::forward<Args>(args)...);
  } catch (...) {
    NodeTraits::deallocate(node_alloc, new_node, 1);
    throw;
  }
  ++sz_;
  new_node->next_ = next;
  new_node->prev_ = given_it.current_node->prev_;
  given_it.current_node->prev_->next_ = new_node;
  given_it.current_node->prev_ = new_node;
  return new_node;
}

template <typename T, class Alloc>
template <typename... Args>
typename List<T, Alloc>::const_iterator List<T, Alloc>::emplace(
    const const_iterator &given_it, Args &&...args) {
  BaseNode *next = given_it.current_node;
  Node *new_node = NodeTraits::allocate(node_alloc, 1);
  try {
    NodeTraits::construct(node_alloc, new_node, std::forward<Args>(args)...);
  } catch (...) {
    NodeTraits::deallocate(node_alloc, new_node, 1);
    throw;
  }
  ++sz_;
  new_node->next_ = next;
  new_node->prev_ = given_it.current_node->prev_;
  given_it.current_node->prev_->next_ = new_node;
  given_it.current_node->prev_ = new_node;
  return typename List<T, Alloc>::const_iterator(new_node);
}

template <typename T, class Alloc>
typename List<T, Alloc>::iterator List<T, Alloc>::insert(
    const typename List<T, Alloc>::iterator &given_it, const T &value) {
  return emplace(given_it, value);
}

template <typename T, class Alloc>
typename List<T, Alloc>::const_iterator List<T, Alloc>::insert(
    const typename List<T, Alloc>::const_iterator &given_it, const T &value) {
  return emplace(given_it, value);
}

template <typename T, class Alloc>
void List<T, Alloc>::erase(const typename List<T, Alloc>::iterator &given_it) {
  given_it.current_node->next_->prev_ = given_it.current_node->prev_;
  given_it.current_node->prev_->next_ = given_it.current_node->next_;
  NodeTraits::destroy(node_alloc, static_cast<Node *>(given_it.current_node));
  NodeTraits::deallocate(node_alloc, static_cast<Node *>(given_it.current_node),
                         1);
  --sz_;
}

template <typename T, class Alloc>
void List<T, Alloc>::erase(
    const typename List<T, Alloc>::const_iterator &given_it) {
  given_it.current_node->next_->prev_ = given_it.current_node->prev_;
  given_it.current_node->prev_->next_ = given_it.current_node->next_;
  NodeTraits::destroy(node_alloc, static_cast<Node *>(given_it.current_node));
  NodeTraits::deallocate(node_alloc, static_cast<Node *>(given_it.current_node),
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
void List<T, Alloc>::push_front(const T &value) {
  insert(begin(), value);
}

template <typename T, class Alloc>
void List<T, Alloc>::push_back(const T &value) {
  insert(end(), value);
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
      emplace(cur);
    }
  } catch (...) {
    clear();
    throw;
  }
}

template <typename T, class Alloc>
List<T, Alloc>::List(size_t count, const Alloc &given_al)
    : fake_(BaseNode(&fake_, &fake_)),
      node_alloc(
          std::allocator_traits<Alloc>::select_on_container_copy_construction(
              given_al)) {
  iterator cur = begin();
  try {
    for (size_t i = 0; i < count; ++i) {
      insert(cur);
    }
  } catch (...) {
    clear();
    throw;
  }
}

template <typename T, class Alloc>
List<T, Alloc>::List(size_t count, const T &value, const Alloc &given_al)
    : fake_(BaseNode(&fake_, &fake_)),
      node_alloc(
          std::allocator_traits<Alloc>::select_on_container_copy_construction(
              given_al)) {
  iterator cur = begin();
  try {
    for (int i = 0; i < count; ++i) {
      insert(cur, value);
    }
  } catch (...) {
    clear();
    throw;
  }
}

template <typename T, class Alloc>
List<T, Alloc>::List(size_t count, const T &value) : fake_(&fake_, &fake_) {
  iterator cur = begin();
  try {
    for (int i = 0; i < count; ++i) {
      insert(cur, value);
    }
  } catch (...) {
    clear();
    throw;
  }
}

template <typename T, class Alloc>
List<T, Alloc>::List(const Alloc &given_al)
    : fake_(BaseNode(&fake_, &fake_)), node_alloc(given_al) {}

template <typename T, class Alloc>
Alloc List<T, Alloc>::get_allocator() const {
  return node_alloc;
}

template <typename T, class Alloc>
List<T, Alloc>::~List() {
  clear();
}

template <typename T, class Alloc>
List<T, Alloc>::List(const List &other)
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
List<T, Alloc>::List(List &&other) noexcept
    : fake_(BaseNode(other.fake_.prev_, other.fake_.next_)),
      node_alloc(other.node_alloc),
      sz_(other.sz_) {
  other.sz_ = 0;
  other.fake_.next_->prev_ = &fake_;
  other.fake_.prev_->next_ = &fake_;
  other.fake_.prev_ = &other.fake_;
  other.fake_.next_ = &other.fake_;
}

template <typename T, class Alloc>
List<T, Alloc> &List<T, Alloc>::operator=(const List &other) {
  if (this == &other) {
    return *this;
  }
  BaseNode old_fake(fake_);
  size_t old_sz = sz_;
  NodeAlloc old_alloc = node_alloc;

  if (std::allocator_traits<
          Alloc>::propagate_on_container_copy_assignment::value) {
    node_alloc = other.node_alloc;
  }
  fake_ = BaseNode(&fake_, &fake_);
  sz_ = 0;
  try {
    for (List::iterator iter = other.begin(); iter != other.end(); ++iter) {
      push_back(*iter);
    }
  } catch (...) {
    clear();
    fake_ = old_fake;
    node_alloc = old_alloc;
    sz_ = old_sz;
    throw;
  }
  BaseNode new_fake(fake_);
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
List<T, Alloc> &List<T, Alloc>::operator=(List &&other) noexcept {
  if (this == &other) {
    return *this;
  }
  clear();
  if (std::allocator_traits<
          Alloc>::propagate_on_container_move_assignment::value) {
    node_alloc = other.node_alloc;
  }
  if (other.sz_ > 0) {
    fake_.next_ = other.fake_.next_;
    fake_.prev_ = other.fake_.prev_;
  }
  sz_ = other.sz_;
  other.sz_ = 0;
  fake_.next_->prev_ = &fake_;
  fake_.prev_->next_ = &fake_;
  other.fake_.next_ = &other.fake_;
  other.fake_.prev_ = &other.fake_;
  return *this;
}

template <typename T, class Alloc>
template <typename U>
bool List<T, Alloc>::template base_iterator<U>::operator==(
    const typename List<T, Alloc>::template base_iterator<U> &other) const {
  return current_node == other.current_node;
}

template <typename T, class Alloc>
template <typename U>
bool List<T, Alloc>::template base_iterator<U>::operator!=(
    const typename List<T, Alloc>::template base_iterator<U> &other) const {
  return current_node != other.current_node;
}

//************************************************************************************
//************************************************************************************
//************************************************************************************

template <class Key, class T, class Hash = std::hash<Key>,
          class KeyEqual = std::equal_to<Key>,
          class Allocator = std::allocator<std::pair<const Key, T>>>
class UnorderedMap {
 private:
  struct Node;

 public:
  friend Node;
  using NodeType = std::pair<const Key, T>;

  UnorderedMap()
      : vec_pointers_(std::vector<typename List<Node, Allocator>::BaseNode *>(
            10, nullptr)){};

  UnorderedMap(const UnorderedMap &other);
  UnorderedMap(UnorderedMap &&other) noexcept;

  UnorderedMap &operator=(const UnorderedMap &other);
  UnorderedMap &operator=(UnorderedMap &&other);

  ~UnorderedMap() = default;

  T &at(const Key &key);
  const T &at(const Key &key) const;
  T &operator[](const Key &key);
  T &operator[](Key &&key);

  size_t size() const;

  template <typename U, typename L>
  class base_iterator {
   public:
    friend class UnorderedMap;

    using value_type = L;
    using pointer = value_type *;
    using difference_type = int;
    using reference = value_type &;
    using iterator_category = std::forward_iterator_tag;

    reference operator*() const;
    pointer operator->() const;
    base_iterator &operator++();
    base_iterator operator++(int);
    operator base_iterator<const U, const L>() const {
      return base_iterator<const U, const L>(iter);
    }
    bool operator==(const base_iterator<U, L> &other) const;
    bool operator!=(const base_iterator<U, L> &other) const;

   private:
    explicit base_iterator(
        typename List<Node, Allocator>::template base_iterator<U> iter)
        : iter(iter) {}
    explicit base_iterator(typename List<Node, Allocator>::BaseNode *node)
        : iter(node) {}

    typename List<Node, Allocator>::template base_iterator<U> iter;
  };

  using const_iterator = base_iterator<const Node, const NodeType>;
  using iterator = base_iterator<Node, NodeType>;

  iterator begin();
  iterator end();
  const_iterator begin() const;
  const_iterator end() const;
  const_iterator cbegin() const;
  const_iterator cend() const;

  std::pair<iterator, bool> insert(const NodeType &node);
  std::pair<iterator, bool> insert(NodeType &&node);
  template <typename InputIt>
  void insert(InputIt first, const InputIt &second);

  template <typename... Args>
  std::pair<iterator, bool> emplace(Args &&...args);

  void erase(const iterator &given_it);
  void erase(const const_iterator &given_it);
  void erase(iterator first, const iterator &second);
  void erase(const_iterator first, const const_iterator &second);

  iterator find(const Key &key);
  const_iterator find(const Key &key) const;

  float load_factor() const;
  float max_load_factor() const;
  void max_load_factor(float ml);
  void reserve(size_t count);  // делает до кол-ва элементов

 private:
  void rehash(size_t count);  // делает кол-во ячеек
  bool check(const Key &key);
  typename List<Node, Allocator>::iterator find_place(const Key &key);

  std::vector<typename List<Node, Allocator>::BaseNode *> vec_pointers_;

  using ListNodeAlloc = typename std::allocator_traits<
      typename std::allocator_traits<Allocator>::template rebind_alloc<
          typename List<Node, Allocator>::Node>>;
  typename std::allocator_traits<Allocator>::template rebind_alloc<
      typename List<Node, Allocator>::Node>
      list_node_alloc;

  using NodeTypeAlloc =
      typename std::allocator_traits<typename std::allocator_traits<
          Allocator>::template rebind_alloc<NodeType>>;
  typename std::allocator_traits<Allocator>::template rebind_alloc<NodeType>
      node_type_alloc;

  struct Node {
    Node() = default;
    ~Node() = default;

    NodeType kv_;
    size_t hash;
  };

  Hash hasher;
  KeyEqual equalizer;
  List<Node, Allocator> intern_list_;
  float load_limit_ = 1;
};

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
void UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::erase(
    const iterator &given_it) {
  erase(const_iterator(given_it));
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
void UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::erase(
    const const_iterator &given_it) {
  size_t hash = (*(given_it.iter)).hash;
  if (vec_pointers_[hash % vec_pointers_.size()] ==
      given_it.iter.current_node) {
    auto copy = given_it;
    ++copy;
    if (copy != end() && (*(copy.iter)).hash % vec_pointers_.size() ==
                             hash % vec_pointers_.size()) {
      vec_pointers_[hash % vec_pointers_.size()] = copy.iter.current_node;
    } else {
      vec_pointers_[hash % vec_pointers_.size()] = nullptr;
    }
    intern_list_.erase(given_it.iter);
  } else {
    intern_list_.erase(given_it.iter);
  }
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
void UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::erase(
    const_iterator first, const const_iterator &second) {
  while (first != second) {
    auto copy = first;
    ++first;
    erase(copy);
  }
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
void UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::erase(
    iterator first, const iterator &second) {
  erase(const_iterator(first), const_iterator(second));
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
std::pair<typename UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::iterator,
          bool>
UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::insert(
    const NodeType &new_node) {
  return emplace(new_node);
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
std::pair<typename UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::iterator,
          bool>
UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::insert(NodeType &&new_node) {
  return emplace(const_cast<Key &&>(new_node.first),
                 std::move(new_node.second));
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
template <typename InputIt>
void UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::insert(
    InputIt first, const InputIt &second) {
  while (first != second) {
    insert(*first);
    ++first;
  }
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
template <typename... Args>
std::pair<typename UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::iterator,
          bool>
UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::emplace(Args &&...args) {
  typename List<Node, Allocator>::Node *new_list_node =
      ListNodeAlloc::allocate(list_node_alloc, 1);
  try {
    NodeTypeAlloc::construct(node_type_alloc, &(new_list_node->value.kv_),
                             std::forward<Args>(args)...);
  } catch (...) {
    ListNodeAlloc::deallocate(list_node_alloc, new_list_node, 1);
    throw;
  }

  if (check(new_list_node->value.kv_.first)) {
    ListNodeAlloc::destroy(list_node_alloc, new_list_node);
    ListNodeAlloc::deallocate(list_node_alloc, new_list_node, 1);
    return std::make_pair(end(), false);
  }

  if (static_cast<float>(intern_list_.size() + 1) /
          static_cast<float>(vec_pointers_.size()) >
      load_limit_) {
    rehash(vec_pointers_.size() * 2);  // noexcept????
  }

  new_list_node->value.hash = hasher(new_list_node->value.kv_.first);
  new_list_node->prev_ = nullptr;
  new_list_node->next_ = nullptr;
  size_t hash = new_list_node->value.hash;
  size_t vec_place = hash % vec_pointers_.size();

  if (vec_pointers_[vec_place] == nullptr) {
    vec_pointers_[vec_place] =
        intern_list_.insertNode(intern_list_.begin(), new_list_node);
    return std::make_pair(
        UnorderedMap::iterator{
            typename List<Node, Allocator>::iterator(vec_pointers_[vec_place])},
        true);
  }
  typename List<Node, Allocator>::iterator iter(vec_pointers_[vec_place]);
  while (iter != intern_list_.end() &&
         iter->hash % vec_pointers_.size() == hash % vec_pointers_.size()) {
    ++iter;
  }
  auto answer = intern_list_.insertNode(iter, new_list_node);
  return std::make_pair(UnorderedMap::iterator{answer}, true);
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
typename UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::iterator
UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::begin() {
  return iterator{intern_list_.begin()};
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
typename UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::iterator
UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::end() {
  return iterator{intern_list_.end()};
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
typename UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::const_iterator
UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::begin() const {
  return const_iterator{intern_list_.begin()};
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
typename UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::const_iterator
UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::end() const {
  return const_iterator{intern_list_.end()};
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
typename UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::const_iterator
UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::cbegin() const {
  return const_iterator{intern_list_.cbegin()};
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
typename UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::const_iterator
UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::cend() const {
  return const_iterator{intern_list_.end()};
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
template <typename U, typename L>
typename UnorderedMap<Key, T, Hash, KeyEqual,
                      Allocator>::template base_iterator<U, L>::reference
    UnorderedMap<Key, T, Hash, KeyEqual,
                 Allocator>::template base_iterator<U, L>::operator*() const {
  return (*iter).kv_;
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
template <typename U, typename L>
typename UnorderedMap<Key, T, Hash, KeyEqual,
                      Allocator>::template base_iterator<U, L>::pointer
    UnorderedMap<Key, T, Hash, KeyEqual,
                 Allocator>::template base_iterator<U, L>::operator->() const {
  return &(*iter).kv_;
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
template <typename U, typename L>
typename UnorderedMap<Key, T, Hash, KeyEqual,
                      Allocator>::template base_iterator<U, L>
    &UnorderedMap<Key, T, Hash, KeyEqual,
                  Allocator>::template base_iterator<U, L>::operator++() {
  ++iter;
  return *this;
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
template <typename U, typename L>
typename UnorderedMap<Key, T, Hash, KeyEqual,
                      Allocator>::template base_iterator<U, L>
    UnorderedMap<Key, T, Hash, KeyEqual,
                 Allocator>::template base_iterator<U, L>::operator++(int) {
  base_iterator<U, L> copy(iter);
  return copy;
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
template <typename U, typename L>
bool UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::
    template base_iterator<U, L>::operator==(
        const typename UnorderedMap<Key, T, Hash, KeyEqual,
                                    Allocator>::template base_iterator<U, L>
            &other) const {
  return iter == other.iter;
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
template <typename U, typename L>
bool UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::
    template base_iterator<U, L>::operator!=(
        const typename UnorderedMap<Key, T, Hash, KeyEqual,
                                    Allocator>::template base_iterator<U, L>
            &other) const {
  return iter != other.iter;
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
typename UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::iterator
UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::find(const Key &key) {
  size_t vec_place = hasher(key) % vec_pointers_.size();
  if (vec_pointers_[vec_place] == nullptr) {
    return end();
  }
  typename List<Node, Allocator>::iterator iter(vec_pointers_[vec_place]);
  while (iter != intern_list_.end() &&
         (*iter).hash % vec_pointers_.size() == vec_place) {
    if (equalizer((*iter).kv_.first, key)) {
      return iterator(iter);
    }
    ++iter;
  }
  return end();
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
bool UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::check(const Key &key) {
  size_t vec_place = hasher(key) % vec_pointers_.size();
  if (vec_pointers_[vec_place] == nullptr) {
    return false;
  }
  typename List<Node, Allocator>::iterator iter(vec_pointers_[vec_place]);
  while (iter != intern_list_.end() &&
         (*iter).hash % vec_pointers_.size() == vec_place) {
    if (equalizer((*iter).kv_.first, key)) {
      return true;
    }
    ++iter;
  }
  return false;
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
typename UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::const_iterator
UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::find(const Key &key) const {
  size_t vec_place = hasher(key) % vec_pointers_.size();
  if (vec_pointers_[vec_place] == nullptr) {
    return end();
  }
  typename List<Node, Allocator>::const_iterator iter(vec_pointers_[vec_place]);
  while (iter != intern_list_.end() &&
         (*iter).hash % vec_pointers_.size() == vec_place) {
    if (equalizer((*iter).kv_.first, key)) {
      return const_iterator(iter);
    }
    ++iter;
  }
  return end();
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
size_t UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::size() const {
  return intern_list_.size();
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
typename List<typename UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::Node,
              Allocator>::iterator
UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::find_place(const Key &key) {
  size_t vec_place = hasher(key) % vec_pointers_.size();
  if (vec_pointers_[vec_place] == nullptr) {
    return intern_list_.begin();
  }
  typename List<Node, Allocator>::iterator iter(vec_pointers_[vec_place]);
  while (iter != intern_list_.end() &&
         (*iter).hash % vec_pointers_.size() == vec_place) {
    ++iter;
  }
  return iter;
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
void UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::rehash(size_t count) {
  List<Node, Allocator> old_list(std::move(intern_list_));
  auto iter = old_list.begin();
  vec_pointers_.resize(count);
  vec_pointers_.assign(count, nullptr);
  size_t vec_place;
  while (iter != old_list.end()) {
    vec_place = (*iter).hash % count;
    auto node = iter.current_node;
    auto it = iter;
    ++iter;
    old_list.forgetNode(it);
    if (vec_pointers_[vec_place] == nullptr) {
      vec_pointers_[vec_place] = intern_list_.insertNode(
          find_place(static_cast<typename List<Node, Allocator>::Node *>(node)
                         ->value.kv_.first),
          node);
    } else {
      std::ignore = intern_list_.insertNode(
          find_place(static_cast<typename List<Node, Allocator>::Node *>(node)
                         ->value.kv_.first),
          node);
    }
  }
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
float UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::load_factor() const {
  return static_cast<float>(vec_pointers_.size()) /
         static_cast<float>(intern_list_.size());
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
float UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::max_load_factor() const {
  return load_limit_;
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
void UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::max_load_factor(
    float ml) {
  load_limit_ = ml;
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
void UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::reserve(size_t count) {
  if (vec_pointers_.size() * load_limit_ >= count) {
    return;
  }
  rehash(std::ceil(1.f / load_limit_ * static_cast<float>(count)));
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
T &UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::operator[](const Key &key) {
  size_t hash = hasher(key);
  if (vec_pointers_[hash % vec_pointers_.size()] == nullptr) {
    return (*(emplace(key, std::move(T())).first)).second;
  } else {
    typename List<Node, Allocator>::iterator iter(
        vec_pointers_[hash % vec_pointers_.size()]);
    while (iter != intern_list_.end() &&
           (*iter).hash % vec_pointers_.size() == hash % vec_pointers_.size()) {
      if (equalizer((*iter).kv_.first, key)) {
        return (*iter).kv_.second;
      }
      ++iter;
    }
    return (*(emplace(key, std::move(T())).first)).second;
  }
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
T &UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::operator[](Key &&key) {
  size_t hash = hasher(key);
  if (vec_pointers_[hash % vec_pointers_.size()] == nullptr) {
    return (*(emplace(std::move(key), std::move(T())).first)).second;
  } else {
    typename List<Node, Allocator>::iterator iter(
        vec_pointers_[hash % vec_pointers_.size()]);
    while (iter != intern_list_.end() &&
           (*iter).hash % vec_pointers_.size() == hash % vec_pointers_.size()) {
      if (equalizer((*iter).kv_.first, key)) {
        return (*iter).kv_.second;
      }
      ++iter;
    }
    return (*(emplace(std::move(key), std::move(T())).first)).second;
  }
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
T &UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::at(const Key &key) {
  auto answer = find(key);

  if (answer == end()) {
    throw std::exception();
  }
  return (*answer).second;
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
const T &UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::at(
    const Key &key) const {
  auto answer = find(key);
  if (answer == end()) {
    throw std::exception();
  }
  return (*answer).second;
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::UnorderedMap(
    const UnorderedMap &other)
    : vec_pointers_(
          std::vector<typename List<Node, Allocator>::BaseNode *>(10, nullptr)),
      list_node_alloc(ListNodeAlloc::select_on_container_copy_construction(
          other.list_node_alloc)),
      node_type_alloc(NodeTypeAlloc::select_on_container_copy_construction(
          other.node_type_alloc)),
      hasher(other.hasher),
      equalizer(other.equalizer) {
  for (auto iter : other) {
    insert(iter);
  }
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::UnorderedMap(
    UnorderedMap &&other) noexcept
    : vec_pointers_(std::move(other.vec_pointers_)),
      list_node_alloc(other.list_node_alloc),
      node_type_alloc(other.node_type_alloc),
      hasher(other.hasher),
      equalizer(other.equalizer),
      intern_list_(std::move(other.intern_list_)) {}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
UnorderedMap<Key, T, Hash, KeyEqual, Allocator>
    &UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::operator=(
        const UnorderedMap &other) {
  while (intern_list_.size() > 0) {
    erase(begin());
  }
  if (std::allocator_traits<
          ListNodeAlloc>::propagate_on_container_copy_assignment::value) {
    list_node_alloc = other.list_node_alloc;
  }
  if (std::allocator_traits<
          NodeTypeAlloc>::propagate_on_container_copy_assignment::value) {
    node_type_alloc = other.node_type_alloc;
  }
  hasher = other.hasher;
  equalizer = other.equalizer;
  for (auto iter : other) {
    insert(NodeType(iter));
  }
  return *this;
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
UnorderedMap<Key, T, Hash, KeyEqual, Allocator>
    &UnorderedMap<Key, T, Hash, KeyEqual, Allocator>::operator=(
        UnorderedMap &&other) {
  vec_pointers_ = std::move(other.vec_pointers_);
  if (std::allocator_traits<
          ListNodeAlloc>::propagate_on_container_move_assignment::value) {
    list_node_alloc = std::move(other.list_node_alloc);
  }
  if (std::allocator_traits<
          NodeTypeAlloc>::propagate_on_container_move_assignment::value) {
    node_type_alloc = std::move(other.node_type_alloc);
  }
  hasher = std::move(other.hasher);
  equalizer = std::move(other.equalizer);
  intern_list_ = std::move(other.intern_list_);
  return *this;
}
