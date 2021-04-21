#ifndef SPLAY_TREE_SPLAY_TREE_H_
#define SPLAY_TREE_SPLAY_TREE_H_

#include <cassert>
#include <iostream>

#include "tree_impl.h"

namespace splay {

// Splay tree (no duplicate keys)
template <
  typename Key,
  typename Value,
  typename KeyComparator,
  typename KeyExtractor>
class splay_tree {
  using self = splay_tree<Key, Value, KeyComparator, KeyExtractor>;
  using node_type = tree_node<Value>;
  using base_type = detail::splay_tree_base<Value>;

 public:
  splay_tree()
    : splay_tree(KeyComparator{}, KeyExtractor{})
  {}

  splay_tree(const KeyComparator& comparator, const KeyExtractor& extractor)
    : impl{detail::create_tree<Value>()}
    , comparator{comparator}
    , extractor{extractor}
  {}

  splay_tree(
      std::initializer_list<Value> init,
      const KeyComparator& comparator = KeyComparator{},
      const KeyExtractor& extractor = KeyExtractor{})
    : splay_tree{std::begin(init), std::end(init), comparator, extractor}
  {}

  template <typename Iter>
  splay_tree(
      Iter first,
      Iter last,
      const KeyComparator& comparator = KeyComparator{},
      const KeyExtractor& extractor = KeyExtractor{})
    : splay_tree(comparator, extractor) {
    for (auto it = first; it != last; ++it) {
      this->insert(*it);
    }
  }

  splay_tree(const self& other)
    : splay_tree{other.comparator, other.extractor} {
    this->impl = detail::copy_tree(other.impl);
  }

  splay_tree(self&& other) noexcept
    : splay_tree{} {
    this->swap(other);
  }

  self& operator = (const self& other) {
    if (this != std::addressof(other)) {
      auto temp = self{other};
      this->swap(temp);
    }
    return *this;
  }

  self& operator = (self&& other) noexcept {
    this->swap(other);
    return *this;
  }

  ~splay_tree() {
    this->clear();
  }

  const node_type* root() const noexcept {
    return impl.root;
  }

  node_type* root() noexcept {
    return impl.root;
  }

  KeyExtractor key_extractor() const {
    return this->extractor;
  }

  KeyComparator key_comparator() const {
    return this->comparator;
  }

  size_t size() const noexcept {
    return detail::get_size_tree(this->impl);
  }

  bool empty() const noexcept {
    return detail::is_empty_tree(this->impl);
  }

  void splay(node_type* node) noexcept {
    detail::splay_node_tree(this->impl, node);
  }

  node_type* find(const Key& key) {
    return detail::find_tree(this->impl, key, this->comparator, this->extractor);
  }

  node_type* lower_bound(const Key& key) {
    return detail::lower_bound_tree(this->impl, key, this->comparator, this->extractor);
  }

  node_type* upper_bound(const Key& key) {
    return detail::upper_bound_tree(this->impl, key, this->comparator, this->extractor);
  }

  node_type* order_statistic(size_t n) noexcept {
    return detail::order_statistic_tree(this->impl, n);
  }

  node_type* insert(const Value& value) {
    return detail::insert_tree<Key, Value, KeyComparator, KeyExtractor>(
      this->impl, value, this->comparator, this->extractor);
  }

  node_type* erase(node_type* node) noexcept {
    return detail::erase_tree(this->impl, node);
  }

  self split_left(node_type* node) noexcept {
    auto right_tree = self{};
    right_tree.impl = detail::split_left_tree(this->impl, node);
    return right_tree;
  }

  self split_right(node_type* node) noexcept {
    auto right_tree = self{};
    right_tree.impl = detail::split_right_tree(this->impl, node);
    return right_tree;
  }

  void merge(self& rhs) {
    assert(detail::is_less(this->impl, rhs.impl, this->comparator, this->extractor));
    detail::merge_trees(this->impl, rhs.impl);
  }

  void swap(self& other) noexcept {
    auto* const tree = this;
    detail::swap_trees(tree->impl, other.impl);
    std::swap(tree->comparator, other.comparator);
    std::swap(tree->extractor, other.extractor);
  }

  void clear() noexcept {
    detail::clear_tree(this->impl);
  }

  template <typename Key_, typename Value_, typename KeyComparator_, typename KeyExtractor_>
  friend std::ostream& operator << (
    std::ostream& out, const splay_tree<Key_, Value_, KeyComparator_, KeyExtractor_>& tree);

 private:
  base_type impl;
  KeyComparator comparator;
  KeyExtractor extractor;
};

template <typename Key, typename Value, typename KeyComparator, typename KeyExtractor>
std::ostream& operator << (
    std::ostream& out, const splay_tree<Key, Value, KeyComparator, KeyExtractor>& tree) {
  detail::print_tree(out, tree.impl);
  return out;
}

}  // namespace splay

#endif  // SPLAY_TREE_SPLAY_TREE_H_