#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

/*

MIT License

Copyright 2017 Tobias Müller

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <cstddef>
#include <stdexcept>
#include <type_traits>

#if !defined(__cplusplus) || (__cplusplus < 201402L)
#error circular_buffer requires C++14
#endif

template<typename T, typename elem_type = typename T::value_type> class circular_buffer_iterator
{
public:
  typedef T circular_buffer_type;

  typedef circular_buffer_iterator<T, elem_type> self_type;

  typedef std::bidirectional_iterator_tag iterator_category;
  typedef elem_type value_type;
  typedef std::size_t size_type;
  typedef std::size_t difference_type;
  typedef value_type* pointer;
  typedef const value_type* const_pointer;
  typedef value_type& reference;
  typedef const value_type& const_reference;

  circular_buffer_iterator(circular_buffer_type* circular_buffer, size_type start_pos) : _circular_buffer(circular_buffer), _pos(start_pos) {}

  circular_buffer_iterator(const circular_buffer_iterator<typename std::remove_const<T>::type> &other) : _circular_buffer(other._circular_buffer), _pos(other._pos) {}

  friend class circular_buffer_iterator<const T, const elem_type>;

  elem_type &operator*() { return (*_circular_buffer)[_pos]; }

  elem_type *operator->() { return &(operator*()); }

  self_type &operator++() {
    ++_pos;
    return *this;
  }

  self_type operator++(int) {
    self_type tmp(*this);
    ++(*this);
    return tmp;
  }

  self_type &operator--() {
    --_pos;
    return *this;
  }

  self_type operator--(int) {
    self_type tmp(*this);
    --(*this);
    return tmp;
  }

  self_type operator+(difference_type n) {
    self_type tmp(*this);
    tmp._pos += n;
    return tmp;
  }

  self_type &operator+=(difference_type n) {
    _pos += n;
    return *this;
  }

  self_type operator-(difference_type n) {
    self_type tmp(*this);
    tmp._pos -= n;
    return tmp;
  }

  self_type &operator-=(difference_type n) {
    _pos -= n;
    return *this;
  }

  bool operator==(const self_type &other) const {
    return (_circular_buffer == other._circular_buffer) && (_pos == other._pos);
  }

  bool operator!=(const self_type &other) const {
    return (_circular_buffer != other._circular_buffer) || (_pos != other._pos);
  }

private:
  circular_buffer_type *_circular_buffer;
  size_type _pos;
};

template<typename T, std::size_t N> class circular_buffer
{
public:
  typedef circular_buffer<T, N> self_type;
  typedef T value_type;
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef circular_buffer_iterator<self_type, self_type::value_type> iterator;
  typedef circular_buffer_iterator<const self_type, const self_type::value_type> const_iterator;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  void fill(const value_type& u) { std::fill_n(begin(), size(), u); }

  void clear(void) {
    size_ = 0;
    tail_ = 0;
    _head = 1;
  }

  // Iterators
  iterator begin() noexcept { return iterator(this, 0); }

  const_iterator begin() const noexcept { return const_iterator(this, 0); }

  iterator end() noexcept { return iterator(this, size()); }

  const_iterator end() const noexcept { return const_iterator(this, size()); }

  reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }

  const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }

  reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

  const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

  const_iterator cbegin() const noexcept { return const_iterator(this, 0); }

  const_iterator cend() const noexcept { return const_iterator(this, size()); }

  const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }

  const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

  // Capacity

  constexpr size_type size() const { return size_; }

  constexpr size_type maxsize_() const { return N; }

  constexpr bool empty() const { return size() == 0; }

  constexpr bool full() const { return size() == N; }

  // Element access

  reference operator[](std::size_t i) { return buffer_[_position_inbuffer_(i)]; }

  const_reference operator[](std::size_t i) const { return buffer_[_position_inbuffer_(i)]; }

  reference at(std::size_t i) {
    if (i >= N) {
      throw std::out_of_range("circular_buffer::at");
    }

    return buffer_[_position_inbuffer_(i)];
  }

  const_reference at(std::size_t i) const {
    if (i >= N) {
      throw std::out_of_range("circular_buffer::at");
    }

    return buffer_[_position_inbuffer_(i)];
  }

  reference front() { return *begin(); }

  const_reference front() const { return *begin(); }

  reference back() { return N ? *(end()-1) : *end(); }

  const_reference back() const { return N ? *end()-1 : *end() ; }

  // Add/remove

  void push_back(const value_type &item) {
    _incrementtail_();
    if (size_ == N) {
      _increment_head();
    }
    buffer_[tail_] = item;
  }

  void pop_front(void) {
    _increment_head();
  }

private:
  T buffer_[N ? N : 1];

  std::size_t size_ = 0;
  std::size_t tail_ = 0;
  std::size_t _head = 1;

  void _incrementtail_() {
    ++tail_;
    ++size_;
    if (tail_ == N) {
      tail_ = 0;
    }
  }

  void _increment_head() {
    ++_head;
    --size_;
    if (_head == N) {
      _head = 0;
    }
  }

  std::size_t _position_inbuffer_(std::size_t i) const {
    return (_head+i) % N;
  }

};

#endif
