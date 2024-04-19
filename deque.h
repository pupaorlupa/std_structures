#include <cmath>
#include <iostream>

template <typename T>
class Deque {
 public:
  Deque();
  explicit Deque(int space);
  Deque(int space, const T& value);
  Deque(const Deque& other);
  ~Deque();
  Deque& operator=(const Deque& other);

  size_t size() const;
  T& operator[](int index);
  const T& operator[](int index) const;
  T& at(size_t index);
  const T& at(size_t index) const;
  void push_back(const T& value);
  void push_front(const T& value);
  void pop_back();
  void pop_front();

  using value_type = T;

  template <typename U>
  class base_iterator {
   public:
    friend class Deque;

    using value_type = U;
    using pointer = value_type*;
    using difference_type = int;
    using reference = value_type&;
    using iterator_category = std::random_access_iterator_tag;

    reference operator*() const;
    base_iterator& operator++();
    base_iterator operator++(int);
    base_iterator& operator--();
    base_iterator operator--(int);
    base_iterator& operator+=(int number);
    base_iterator& operator-=(int number);
    difference_type operator-(const base_iterator& other) const;
    base_iterator operator-(int number) const;
    base_iterator operator+(int number) const;
    pointer operator->() { return (*current_row_) + dist_; }
    operator base_iterator<const U>();
    bool operator==(const base_iterator<U>& other) const;
    bool operator<(const base_iterator<U>& other) const;
    bool operator>(const base_iterator<U>& other) const;
    bool operator<=(const base_iterator<U>& other) const;
    bool operator>=(const base_iterator<U>& other) const;
    bool operator!=(const base_iterator<U>& other) const;

   private:
    base_iterator(T** c, int dist_) : current_row_(c), dist_(dist_) {}
    T** current_row_;
    int dist_;
  };

  using const_iterator = base_iterator<const T>;
  using iterator = base_iterator<T>;

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
  void erase(const iterator& given_it);

 private:
  void swap(Deque& other);
  template <bool have_args>
  void construct(size_t count, const T* item);

  static const int CHUNK_CAP_ = 256;

  T** chunks_;
  int first_chunk_index_ = -1;
  int first_el_index_ = -1;
  int last_chunk_index_ = -1;
  int last_el_index_ = -1;
  int chunks_quantity_ = 0;  // кол-во чанков
  size_t sz_ = 0;
};

template <typename T>
Deque<T>::Deque(): chunks_(nullptr) {}

template <typename T>
template <bool have_args>
void Deque<T>::construct(size_t count, const T* item) {
  int i = 0;
  int k = 0;
  chunks_ = reinterpret_cast<T**>(new char[chunks_quantity_ * sizeof(T*)]);
  try {
    for (; i < chunks_quantity_; ++i) {
      chunks_[i] = reinterpret_cast<T*>(new char[CHUNK_CAP_ * sizeof(T)]);
      if (i != chunks_quantity_ - 1) {
        for (k = 0;
             k <
             (i == chunks_quantity_ - 2
                  ? (static_cast<int>(sz_) + CHUNK_CAP_ - 1) % CHUNK_CAP_ + 1
                  : CHUNK_CAP_);
             ++k) {
          if constexpr (have_args) {
            new (chunks_[i] + k) T(*item);
          } else if constexpr (!have_args) {
            new (chunks_[i] + k) T();
          }
        }
      }
    }
  } catch (...) {
    for (int j = 0; j <= i; ++j) {
      for (int b = 0; b < (j == i ? k : CHUNK_CAP_); ++b) {
        (chunks_[j] + b)->~T();
      }
      delete[] reinterpret_cast<char*>(chunks_[j]);
    }
    delete[] reinterpret_cast<char*>(chunks_);
    throw;
  }
  first_chunk_index_ = 0;
  first_el_index_ = 0;
  last_chunk_index_ = chunks_quantity_ - 2;
  last_el_index_ = (static_cast<int>(sz_) + CHUNK_CAP_ - 1) % CHUNK_CAP_;
}

template <typename T>
Deque<T>::Deque(int space)
    : chunks_quantity_((space + CHUNK_CAP_ - 1) / CHUNK_CAP_ + 1), sz_(space) {
  construct<false>(space, nullptr);
}

template <typename T>
Deque<T>::Deque(int space, const T& value)
    : chunks_quantity_((space + CHUNK_CAP_ - 1) / CHUNK_CAP_ + 1), sz_(space) {
  construct<true>(space, &value);
}

template <typename T>
size_t Deque<T>::size() const {
  return sz_;
}

template <typename T>
T& Deque<T>::operator[](int index) {
  return chunks_[first_chunk_index_ + (first_el_index_ + index) / CHUNK_CAP_]
                [(first_el_index_ + index) % CHUNK_CAP_];
}

template <typename T>
const T& Deque<T>::operator[](int index) const {
  return chunks_[first_chunk_index_ + (first_el_index_ + index) / CHUNK_CAP_]
                [(first_el_index_ + index) % CHUNK_CAP_];
}

template <typename T>
Deque<T>::Deque(const Deque& other)
    : Deque() {
  for (auto iter: other) {
    push_back(iter);
  }
}

template <typename T>
Deque<T>& Deque<T>::operator=(const Deque& other) {
  if (this == &other) {
    return *this;
  }
  Deque<T> copy = other;
  swap(copy);
  return *this;
}

template <typename T>
void Deque<T>::swap(Deque& other) {
  std::swap(chunks_, other.chunks_);
  std::swap(first_chunk_index_, other.first_chunk_index_);
  std::swap(first_el_index_, other.first_el_index_);
  std::swap(last_el_index_, other.last_el_index_);
  std::swap(last_chunk_index_, other.last_chunk_index_);
  std::swap(sz_, other.sz_);
  std::swap(chunks_quantity_, other.chunks_quantity_);
}

template <typename T>
T& Deque<T>::at(size_t index) {
  if (index < 0 || index >= sz_) {
    throw std::out_of_range("Deque<T>::at() index is out of range");
  }
  return (*this)[index];
}

template <typename T>
const T& Deque<T>::at(size_t index) const {
  if (index < 0 || index >= sz_) {
    throw std::out_of_range("Deque<T>::at() index is out of range");
  }
  return (*this)[index];
}

template <typename T>
void Deque<T>::pop_back() {
  (chunks_[last_chunk_index_] + last_el_index_)->~T();
  if (last_el_index_ == 0) {
    last_el_index_ = CHUNK_CAP_ - 1;
    last_chunk_index_ -= 1;
  } else {
    last_el_index_ -= 1;
  }
  --sz_;
}

template <typename T>
void Deque<T>::push_back(const T& value) {  // поддерживаем инвариант, что
  // справа всего есть свободный чанк
  if (chunks_ == nullptr) {
    chunks_ = reinterpret_cast<T**>(new char[2 * sizeof(T*)]);
    bool first_created = false, el_created = false;
    try {
      chunks_[0] = reinterpret_cast<T*>(new char[CHUNK_CAP_ * sizeof(T)]);
      first_created = true;
      new (chunks_[0] + 0) T(value);
      el_created = true;
      chunks_[1] = reinterpret_cast<T*>(new char[CHUNK_CAP_ * sizeof(T)]);
      chunks_quantity_ = 2;
      first_chunk_index_ = 0;
      last_chunk_index_ = 0;
      first_el_index_ = 0;
      last_el_index_ = 0;
      sz_ = 0;  // в конце sz += 1;
    } catch (...) {
      if (el_created) {
        (chunks_[0] + 0)->~T();
      }
      if (first_created) {
        delete[] reinterpret_cast<char*>(chunks_[0]);
      }
      delete[] reinterpret_cast<char*>(chunks_);
      chunks_ = nullptr;
      throw;
    }
  } else if (last_el_index_ != CHUNK_CAP_ - 1) {
    new (chunks_[last_chunk_index_] + last_el_index_ + 1) T(value);
    last_el_index_ += 1;
  } else if (last_chunk_index_ != chunks_quantity_ - 2) {
    if (chunks_[last_chunk_index_ + 2] == nullptr) {
      chunks_[last_chunk_index_ + 2] =
          reinterpret_cast<T*>(new char[CHUNK_CAP_ * sizeof(T)]);
    }
    new (chunks_[last_chunk_index_ + 1] + 0) T(value);
    last_chunk_index_ += 1;
    last_el_index_ = 0;
  } else {
    T** new_chunks = reinterpret_cast<T**>(new char[chunks_quantity_ * 2 * sizeof(T*)]);
    int i = 0;
    bool last_created = false;
    try {
      for (; i < chunks_quantity_ * 2; ++i) {
        if (i < chunks_quantity_) {
          new_chunks[i] = chunks_[i];
        } else {
          new_chunks[i] = nullptr;
        }
      }
      new_chunks[last_chunk_index_ + 2] =
          reinterpret_cast<T*>(new char[CHUNK_CAP_ * sizeof(T)]);
      last_created = true;
      new (new_chunks[last_chunk_index_ + 1] + 0) T(value);
    } catch (...) {
      if (last_created) {
        delete[] reinterpret_cast<char*>(new_chunks[last_el_index_ + 2]);
      }
      delete[] reinterpret_cast<char*>(new_chunks);
      throw;
    }
    delete[] reinterpret_cast<char*>(chunks_);
    chunks_ = new_chunks;
    last_chunk_index_ += 1;
    last_el_index_ = 0;
    chunks_quantity_ *= 2;
  }
  sz_ += 1;
}

template <typename T>
void Deque<T>::push_front(const T& value) {
  if (chunks_ == nullptr) {
    chunks_ = reinterpret_cast<T**>(new char[2 * sizeof(T*)]);
    bool first_created = false, el_created = false;
    try {
      chunks_[0] = reinterpret_cast<T*>(new char[CHUNK_CAP_ * sizeof(T)]);
      first_created = true;
      new (chunks_[0] + 0) T(value);
      el_created = true;
      chunks_[1] = reinterpret_cast<T*>(new char[CHUNK_CAP_ * sizeof(T)]);
      chunks_quantity_ = 2;
      first_chunk_index_ = 0;
      last_chunk_index_ = 0;
      first_el_index_ = 0;
      last_el_index_ = 0;
      sz_ = 0;  // в конце sz += 1;
    } catch (...) {
      if (el_created) {
        (chunks_[0] + 0)->~T();
      }
      if (first_created) {
        delete[] reinterpret_cast<char*>(chunks_[0]);
      }
      delete[] reinterpret_cast<char*>(chunks_);
      chunks_ = nullptr;
      throw;
    }
  } else if (first_el_index_ != 0) {
    new (chunks_[first_chunk_index_] + first_el_index_ - 1) T(value);
    first_el_index_ -= 1;
  } else if (first_chunk_index_ != 0) {
    if (chunks_[first_chunk_index_ - 1] == nullptr) {
      chunks_[first_chunk_index_ - 1] =
          reinterpret_cast<T*>(new char[CHUNK_CAP_ * sizeof(T)]);
    }
    new (chunks_[first_chunk_index_ - 1] + CHUNK_CAP_ - 1) T(value);
    first_chunk_index_ -= 1;
    first_el_index_ = CHUNK_CAP_ - 1;
  } else {
    T** new_chunks = reinterpret_cast<T**>(new char[chunks_quantity_ * 2 * sizeof(T*)]);
    bool prev_created = false;
    try {
      for (int i = 0; i < chunks_quantity_; ++i) {
        new_chunks[i] = nullptr;
      }
      for (int i = chunks_quantity_; i < chunks_quantity_ * 2; ++i) {
        new_chunks[i] = chunks_[i - chunks_quantity_];
      }
      new_chunks[first_chunk_index_ + chunks_quantity_ - 1] =
          reinterpret_cast<T*>(new char[CHUNK_CAP_ * sizeof(T)]);
      prev_created = true;
      new (new_chunks[first_chunk_index_ + chunks_quantity_ - 1] + CHUNK_CAP_ - 1) T(value);
    } catch (...) {
      if (prev_created) {
        delete[] new_chunks[first_chunk_index_ + chunks_quantity_ - 1];
      }
      delete[] new_chunks;
      throw;
    }
    delete[] reinterpret_cast<char*>(chunks_);
    chunks_ = new_chunks;
    first_chunk_index_ = first_chunk_index_ + chunks_quantity_ - 1;
    last_chunk_index_ = last_chunk_index_ + chunks_quantity_;
    first_el_index_ = CHUNK_CAP_ - 1;
    chunks_quantity_ *= 2;
  }
  sz_ += 1;
}

template <typename T>
void Deque<T>::pop_front() {
  (chunks_[first_chunk_index_] + first_el_index_)->~T();
  if (first_el_index_ == CHUNK_CAP_ - 1) {
    first_chunk_index_ += 1;
    first_el_index_ = 0;
  } else {
    first_el_index_ += 1;
  }
  --sz_;
}

template <typename T>
template <typename U>
typename Deque<T>::template base_iterator<U>::reference
    Deque<T>::template base_iterator<U>::operator*() const {
  return (*current_row_)[dist_];
}

template <typename T>
template <typename U>
typename Deque<T>::template base_iterator<U>&
    Deque<T>::template base_iterator<U>::operator++() {
  if (current_row_ == nullptr) {
    return *this;
  }
  if (dist_ == Deque<T>::CHUNK_CAP_ - 1) {
    dist_ = 0;
    ++current_row_;
  } else {
    ++dist_;
  }
  return *this;
}

template <typename T>
template <typename U>
typename Deque<T>::template base_iterator<U>
    Deque<T>::template base_iterator<U>::operator++(int) {
  base_iterator<U> copy(current_row_, dist_);
  ++*this;
  return copy;
}

template <typename T>
template <typename U>
typename Deque<T>::template base_iterator<U>&
    Deque<T>::template base_iterator<U>::operator--() {
  if (current_row_ == nullptr) {
    return *this;
  }
  if (dist_ == 0) {
    --current_row_;
    dist_ = Deque<T>::CHUNK_CAP_ - 1;
  } else {
    --dist_;
  }
  return *this;
}

template <typename T>
template <typename U>
typename Deque<T>::template base_iterator<U>
    Deque<T>::template base_iterator<U>::operator--(int) {
  base_iterator<U> copy(current_row_, dist_);
  --*this;
  return copy;
}

template <typename T>
template <typename U>
typename Deque<T>::template base_iterator<U>&
    Deque<T>::template base_iterator<U>::operator+=(int number) {
  if (current_row_ == nullptr) {
    return *this;
  }
  if (number >= 0) {
    current_row_ += ((dist_ + number) / Deque<T>::CHUNK_CAP_);
    dist_ = (dist_ + number) % Deque<T>::CHUNK_CAP_;
  } else {
    number *= -1;
    current_row_ -= ((number + CHUNK_CAP_ - 1 - dist_) / CHUNK_CAP_);
    dist_ = (dist_ >= number)
                ? dist_ - number
                : (CHUNK_CAP_ + (dist_ - number) % CHUNK_CAP_) % CHUNK_CAP_;
  }
  return *this;
}

template <typename T>
template <typename U>
bool Deque<T>::template base_iterator<U>::operator<(
    const typename Deque<T>::template base_iterator<U>& other) const {
  return (current_row_ < other.current_row_) ||
         (current_row_ == other.current_row_ && dist_ < other.dist_);
}

template <typename T>
template <typename U>
bool Deque<T>::template base_iterator<U>::operator>(
    const typename Deque<T>::template base_iterator<U>& other) const {
  return (other < *this);
}

template <typename T>
template <typename U>
bool Deque<T>::template base_iterator<U>::operator==(
    const typename Deque<T>::template base_iterator<U>& other) const {
  return current_row_ == other.current_row_ && dist_ == other.dist_;
}

template <typename T>
template <typename U>
bool Deque<T>::template base_iterator<U>::operator>=(
    const typename Deque<T>::template base_iterator<U>& other) const {
  return !(*this < other);
}

template <typename T>
template <typename U>
bool Deque<T>::template base_iterator<U>::operator<=(
    const typename Deque<T>::template base_iterator<U>& other) const {
  return !(*this > other);
}

template <typename T>
template <typename U>
bool Deque<T>::template base_iterator<U>::operator!=(
    const typename Deque<T>::template base_iterator<U>& other) const {
  return !(*this == other);
}

template <typename T>
template <typename U>
typename Deque<T>::template base_iterator<U>::difference_type
    Deque<T>::template base_iterator<U>::operator-(
        const typename Deque<T>::template base_iterator<U>& other) const {
  return (current_row_ - other.current_row_) *
             static_cast<int>(Deque<T>::CHUNK_CAP_) +
         dist_ - other.dist_;
}

template <typename T>
typename Deque<T>::iterator Deque<T>::begin() {
  if (sz_ == 0) {
    return iterator(nullptr, first_el_index_);
  }
  return iterator(chunks_ + first_chunk_index_, first_el_index_);
}

template <typename T>
typename Deque<T>::iterator Deque<T>::end() {
  if (sz_ == 0) {
    return iterator(nullptr, first_el_index_);
  }
  if (last_el_index_ == CHUNK_CAP_ - 1) {
    return iterator(chunks_ + (last_chunk_index_ + 1), 0);
  }
  return iterator(chunks_ + last_chunk_index_, last_el_index_ + 1);
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::end() const {
  return cend();
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::begin() const {
  return cbegin();
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::cend() const {
  if (sz_ == 0) {
    return const_iterator(nullptr, first_el_index_);
  }
  if (last_el_index_ == CHUNK_CAP_ - 1) {
    return const_iterator(chunks_ + last_chunk_index_ + 1, 0);
  }
  return const_iterator(chunks_ + last_chunk_index_, last_el_index_ + 1);
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::cbegin() const {
  if (sz_ == 0) {
    return const_iterator(nullptr, first_el_index_);
  }
  return const_iterator(chunks_ + first_chunk_index_, first_el_index_);
}

template <typename T>
std::reverse_iterator<typename Deque<T>::iterator> Deque<T>::rbegin() {
  return std::reverse_iterator<typename Deque<T>::iterator>(end());
}

template <typename T>
std::reverse_iterator<typename Deque<T>::const_iterator> Deque<T>::rbegin()
    const {
  return std::reverse_iterator<typename Deque<T>::const_iterator>(cend());
}

template <typename T>
std::reverse_iterator<typename Deque<T>::iterator> Deque<T>::rend() {
  return std::reverse_iterator<typename Deque<T>::iterator>(begin());
}

template <typename T>
std::reverse_iterator<typename Deque<T>::const_iterator> Deque<T>::rend()
    const {
  return std::reverse_iterator<typename Deque<T>::const_iterator>(cbegin());
}

template <typename T>
std::reverse_iterator<typename Deque<T>::const_iterator> Deque<T>::crbegin()
    const {
  return std::reverse_iterator<typename Deque<T>::const_iterator>(cend());
}

template <typename T>
std::reverse_iterator<typename Deque<T>::const_iterator> Deque<T>::crend()
    const {
  return std::reverse_iterator<typename Deque<T>::const_iterator>(cbegin());
}

template <typename T>
typename Deque<T>::iterator Deque<T>::insert(
    const typename Deque<T>::iterator& given_it, const T& value) {
  if (given_it == end()) {
    push_back(value);
    return end() - 1;
  } else if (given_it == begin()) {
    push_front(value);
    return begin();
  }
  auto diff = given_it.current_row_ - begin().current_row_;
  push_back(value);
  auto i = end() - 1;
  for (; i != iterator(begin().current_row_ + diff, given_it.dist_); --i) {
    std::swap(*i, *(i - 1));
  }
  return i;
}

template <typename T>
void Deque<T>::erase(const typename Deque<T>::iterator& given_it) {
  for (auto i = given_it; i != end() - 1; ++i) {
    std::swap(*i, *(i + 1));
  }
  pop_back();
}

template <typename T>
template <typename U>
typename Deque<T>::template base_iterator<U>&
    Deque<T>::template base_iterator<U>::operator-=(int number) {
  *this += (-number);
  return *this;
}

template <typename T>
template <typename U>
typename Deque<T>::template base_iterator<U>
    Deque<T>::template base_iterator<U>::operator-(int number) const {
  base_iterator copy = base_iterator(current_row_, dist_);
  copy += (-number);
  return copy;
}

template <typename T>
template <typename U>
typename Deque<T>::template base_iterator<U>
    Deque<T>::template base_iterator<U>::operator+(int number) const {
  base_iterator copy = base_iterator(current_row_, dist_);
  if (current_row_ == nullptr) {
    return copy;
  }
  copy += number;
  return copy;
}

template <typename T>
Deque<T>::~Deque() {
  for (auto i = begin(); i != end(); ++i) {
    if (begin().current_row_ != nullptr) {
      i->~T();
    }
  }
  if (chunks_ != nullptr) {
    for (int i = 0; i < chunks_quantity_; ++i) {
      if (chunks_[i] != nullptr) {
        delete[] reinterpret_cast<char*>(chunks_[i]);
      }
    }
    delete[] reinterpret_cast<char*>(chunks_);
  }
}
