#ifndef SPLAY_TREE_TREE_NODE_H_
#define SPLAY_TREE_TREE_NODE_H_

#include <iostream>

namespace splay {

// Node of the binary tree

template <typename Value>
struct tree_node {

  tree_node(const Value& value) noexcept
    : value{value}
    , size{1}
    , parent{nullptr}
    , left{nullptr}
    , right{nullptr}
  {}

  ~tree_node() {
    auto* const node = this;
    node->parent = nullptr;
    node->left = nullptr;
    node->right = nullptr;
    node->size = uint64_t{0};
  }

  bool is_root() const noexcept {
    const auto* node = this;
    return node->parent == nullptr;
  }

  bool is_left_child() const noexcept {
    const auto* node = this;
    return !node->is_root() && node->parent->left == node;
  }

  bool is_right_child() const noexcept {
    const auto* node = this;
    return !node->is_root() && node->parent->right == node;
  }

  const tree_node<Value>* find_root() const noexcept {
    const auto* node = this;
    if (node == nullptr) {
      return nullptr;
    }
    while (node->parent != nullptr) {
      node = node->parent;
    }
    return node;
  }

  const tree_node<Value>* rightmost_node() const noexcept {
    const auto* node = this;
    while (node->right != nullptr) {
      node = node->right;
    }
    return node;
  }

  tree_node<Value>* rightmost_node() noexcept {
    const auto* const node = this;
    return const_cast<tree_node<Value>*>(node->rightmost_node());
  }

  const tree_node<Value>* leftmost_node() const noexcept {
    const auto* node = this;
    while (node->left != nullptr) {
      node = node->left;
    }
    return node;
  }

  tree_node<Value>* leftmost_node() noexcept {
    const auto* const node = this;
    return const_cast<tree_node<Value>*>(node->leftmost_node());
  }

  // find next node with respect to key order
  const tree_node<Value>* next_node() const noexcept {
    const auto* node = this;
    auto next = static_cast<const tree_node<Value>*>(nullptr);
    if (node->right != nullptr) {
      next = node->right;
      while (next->left != nullptr) {
        next = next->left;
      }
    } else {
      while (node->parent != nullptr && !node->is_left_child()) {
        node = node->parent;
      }
      next = node->parent;
    }
    return next;
  }

  tree_node<Value>* next_node() noexcept {
    const auto* const node = this;
    return const_cast<tree_node<Value>*>(node->next_node());
  }

  // find previous node with respect to key order
  const tree_node<Value>* prev_node() const {
    const auto* node = this;
    auto prev = static_cast<const tree_node<Value>*>(nullptr);
    if (node->left != nullptr) {
      prev = node->left;
      while (prev->right != nullptr) {
        prev = prev->right;
      }
    } else {
      while (node->parent != nullptr && !node->is_right_child()) {
        node = node->parent;
      }
      prev = node->parent;
    }
    return prev;
  }

  tree_node<Value>* prev_node() noexcept {
    const auto* const node = this;
    return const_cast<tree_node<Value>*>(node->prev_node());
  }

  Value value;
  uint64_t size;
  tree_node<Value>* parent;
  tree_node<Value>* left;
  tree_node<Value>* right;
};

namespace detail {

template <typename Value>
void print_node(std::ostream& out, const tree_node<Value>& node) {
  out << "[v=" << node.value << ", s=" << node.size << "]";
}

template <typename Value>
tree_node<Value>* create_node(const Value& value) {
  return new tree_node<Value>(value);
}


template <typename Value>
void destroy_node(tree_node<Value>* node) noexcept {
  assert(node != nullptr);
  delete node;
}


}  // namespace detail

template <typename Value>
std::ostream& operator << (std::ostream& out, const tree_node<Value>& node) {
  detail::print_node(out, node);
  return out;
}

}  // namespace splay

#endif  // SPLAY_TREE_TREE_NODE_H_