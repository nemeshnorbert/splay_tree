#ifndef SPLAY_TREE_IMPLICIT_SPLAY_TREE_H_
#define SPLAY_TREE_IMPLICIT_SPLAY_TREE_H_

#include <cassert>
#include <iostream>

#include "tree_impl.h"

namespace splay {

// Splay tree with implicit keys
template <typename Value>
class implicit_splay_tree {
  using self = implicit_splay_tree<Value>;
  using node_type = tree_node<Value>;
  using base_type = detail::splay_tree_base<Value>;

 public:
  implicit_splay_tree()
    : impl{detail::create_tree<Value>()}
  {}

  implicit_splay_tree(std::initializer_list<Value> init)
    : implicit_splay_tree{std::begin(init), std::end(init)}
  {}

  template <typename Iter>
  implicit_splay_tree(Iter first, Iter last)
    : implicit_splay_tree{} {
    for (auto it = first; it != last; ++it) {
      this->insert(*it);
    }
  }

  implicit_splay_tree(const self& other)
    : implicit_splay_tree{} {
    this->impl = detail::copy_tree(other.impl);
  }

  implicit_splay_tree(self&& other) noexcept
    : implicit_splay_tree{} {
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

  ~implicit_splay_tree() {
    this->clear();
  }

  const node_type* root() const noexcept {
    return impl.root;
  }

  node_type* root() noexcept {
    return impl.root;
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

  node_type* insert(const Value& value) {
    auto new_tree = detail::create_tree<Value>(value);
    detail::merge_trees(this->impl, new_tree);
    return new_tree.root;
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
    detail::merge_trees(this->impl, rhs.impl);
  }

  node_type* order_statistic(size_t n) noexcept {
    return detail::order_statistic_tree(this->impl, n);
  }

  void swap(self& other) noexcept {
    auto* const tree = this;
    detail::swap_trees(tree->impl, other.impl);
  }

  void clear() noexcept {
    detail::clear_tree(this->impl);
  }

  template <typename Value_>
  friend std::ostream& operator << (std::ostream& out, const implicit_splay_tree<Value_>& tree);

 private:
  base_type impl;
};

template <typename Value>
std::ostream& operator << (std::ostream& out, const implicit_splay_tree<Value>& tree) {
  detail::print_tree(out, tree.impl);
  return out;
}

}  // namespace splay

#endif  // SPLAY_TREE_IMPLICIT_SPLAY_TREE_H_