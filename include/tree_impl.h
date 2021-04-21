#ifndef SPLAY_TREE_TREE_IMPL_H_
#define SPLAY_TREE_TREE_IMPL_H_

#include <cassert>
#include <iostream>

#include "tree_node.h"

namespace splay {
namespace detail {

template <typename Value>
void print_subtree(std::ostream& out, const tree_node<Value>* root) {
  out << "(";
  if (root != nullptr) {
    print_subtree(out, root->left);
    print_node(out, *root);
    print_subtree(out, root->right);
  }
  out << ")";
}

template <typename Value>
void destroy_substree(tree_node<Value>* root) noexcept {
  if (root == nullptr) {
    return;
  }
  destroy_substree(root->left);
  root->left = nullptr;
  destroy_substree(root->right);
  root->right = nullptr;
  root->parent = nullptr;
  destroy_node(root);
}

template <typename Value>
void update_size(tree_node<Value>* node) noexcept {
  if (node != nullptr) {
    node->size = uint64_t{1};
    node->size += (node->left != nullptr ? node->left->size : uint64_t{0});
    node->size += (node->right != nullptr ? node->right->size : uint64_t{0});
  }
}

// insert value to subtree at root `root`, no rebalancing
template <typename Key, typename Value, typename KeyComparator, typename KeyExtractor>
tree_node<Value>* insert_subtree(
    tree_node<Value>* root,
    const Value& value,
    const KeyComparator& comparator,
    const KeyExtractor& extractor) {
  assert(root != nullptr);
  auto node = static_cast<tree_node<Value>*>(nullptr);
  while (root != nullptr) {
    node = root;
    if (comparator(extractor(value), extractor(root->value))) {
      if (root->left == nullptr) {
        auto new_node = create_node(value);
        root->left = new_node;
        new_node->parent = root;
        node = new_node;
        break;
      } else {
        root = root->left;
      }
    } else if (comparator(extractor(root->value), extractor(value))) {
      if (root->right == nullptr) {
        auto new_node = create_node(value);
        root->right = new_node;
        new_node->parent = root;
        node = new_node;
        break;
      } else {
        root = root->right;
      }
    } else {
      node = nullptr;
      break;
    }
  }
  if (node != nullptr) {
    auto parent = node->parent;
    while (parent != nullptr) {
      parent->size += uint64_t{1};
      parent = parent->parent;
    }
  }
  return node;
}

template <typename Value>
void left_rotate_node(tree_node<Value>* node) noexcept {
  /* u is node, a is parent, B is branch, p is granny
  *
  *      p             p
  *      |             |
  *      a             u
  *     / \           / \
  *    u   C    =>   A   a
  *   / \               / \
  *  A  B              B  C
  */

  assert(node != nullptr);
  assert(node->parent != nullptr);
  auto parent = node->parent;
  auto branch = node->right;
  auto granny = parent->parent;
  if (granny != nullptr) {
    if (parent->is_left_child()) {
      granny->left = node;
    } else if (parent->is_right_child()) {
      granny->right = node;
    } else {
      assert(false);
    }
  }
  node->parent = granny;
  node->right = parent;
  parent->parent = node;
  parent->left = branch;
  if (branch != nullptr) {
    branch->parent = parent;
  }
  update_size(parent);
  update_size(node);
}

template <typename Value>
void right_rotate_node(tree_node<Value>* node) noexcept {
  /* u is node, a is parent, B is branch, p is granny
  *
  *      p             p
  *      |             |
  *      a             u
  *     / \           / \
  *    C   u    =>   a  A
  *       / \       / \
  *      B  A      C  B
  */

  assert(node != nullptr);
  assert(node->parent != nullptr);
  auto parent = node->parent;
  auto branch = node->left;
  auto granny = parent->parent;
  if (granny != nullptr) {
    if (parent->is_left_child()) {
      granny->left = node;
    } else if (parent->is_right_child()) {
      granny->right = node;
    } else {
      assert(false);
    }
  }
  node->parent = granny;
  node->left = parent;
  parent->parent = node;
  parent->right = branch;
  if (branch != nullptr) {
    branch->parent = parent;
  }
  update_size(parent);
  update_size(node);
}

template <typename Value>
void rotate_node(tree_node<Value>* node) noexcept {
  if (node->is_left_child()) {
    left_rotate_node(node);
  } else if (node->is_right_child()) {
    right_rotate_node(node);
  } else {
    assert(false);
  }
}

// splay node `node`
template <typename Value>
void splay_node(tree_node<Value>* node) noexcept {
  /* ------------------------------------------------------------------------------------
  * zig_zig
  *        p                                                                p
  *        |                                 p                              |
  *        b                                 |                              u
  *       / \                                a                             / \
  *      a   D    left_rotate_node(a)      /   \    left_rotate_node(u)   A   a
  *     / \                              u       b                           / \
  *    u   C                            / \     / \                         B   b
  *   / \                              A   B   C   D                           / \
  *  A   B                                                                    C   D
  * ------------------------------------------------------------------------------------
  * zig_zag
  *    p                             p
  *    |                             |                                p
  *    b                             b                                |
  *   / \                           / \                               u
  *  D   a    left_rotate_node(u)  D   u   right_rotate_node(u)     /   \
  *     / \                           / \                         b       a
  *    u   C                         A   a                       / \     / \
  *   / \                               / \                     D   A   B   C
  *  A   B                             B   C
  * ------------------------------------------------------------------------------------
  * zag_zig
  *       p                                  p
  *       |                                  |                                p
  *       b                                  b                                |
  *      / \                                / \                               u
  *     a   D    right_rotate_node(u)      u   D    left_rotate_node(u)     /   \
  *    / \                                / \                             a       b
  *   C   u                              a   B                           / \     / \
  *      / \                            / \                             C   A   B   D
  *     A   B                          C   A
  * ------------------------------------------------------------------------------------
  * zag_zag
  *    p                                                                  p
  *    |                                p                                 |
  *    b                                |                                 u
  *   / \                               a                                / \
  *  D   a   right_rotate_node(a)     /   \   right_rotate_node(u)      a   B
  *     / \                         b       u                          / \
  *    C   u                       / \     / \                        b   A
  *       / \                     D   C   A   B                      / \
  *      A   B                                                      D   C
  * ------------------------------------------------------------------------------------
  */
  assert(node != nullptr);
  while (node->parent != nullptr) {
    if (node->parent->is_root()) {
      rotate_node(node);
    } else {
      auto zig_zag = node->is_left_child() && node->parent->is_right_child();
      auto zag_zig = node->is_right_child() && node->parent->is_left_child();
      if (zig_zag || zag_zig) {
        rotate_node(node);
        rotate_node(node);
      } else {
        rotate_node(node->parent);
        rotate_node(node);
      }
    }
  }
}

// find node with key `key` in the subtree under node `root`. If such node doesn't exist
// return the last node during this search
template <typename Key, typename Value, typename KeyComparator, typename KeyExtractor>
const tree_node<Value>* find_candidate_subtree(
    const tree_node<Value>* root,
    const Key& key,
    const KeyComparator& comparator,
    const KeyExtractor& extractor) noexcept {
  auto node = static_cast<const tree_node<Value>*>(nullptr);
  if (root != nullptr) {
    node = root->parent;
    while (root != nullptr) {
      node = root;
      if (comparator(key, extractor(root->value))) {
        root = root->left;
      } else if (comparator(extractor(root->value), key)) {
        root = root->right;
      } else {
        node = root;
        break;
      }
    }
  }
  return node;
}

template <typename Key, typename Value, typename KeyComparator, typename KeyExtractor>
tree_node<Value>* find_candidate_subtree(
    tree_node<Value>* root,
    const Key& key,
    const KeyComparator& comparator,
    const KeyExtractor& extractor) noexcept {
  const auto* const node = root;
  return const_cast<tree_node<Value>*>(find_candidate_subtree(node, key, comparator, extractor));
}

// find `n` the element (0-based indexing) with respect to keys order in the subtree of
// the node `root`
template <typename Value>
const tree_node<Value>* order_statistic_subtree(
  const tree_node<Value>* root, size_t n) noexcept {
  auto position = n;
  if (root != nullptr && n >= root->size) {
    return nullptr;
  }
  while (root != nullptr) {
    const auto left_subtree_size = root->left != nullptr ? root->left->size : uint32_t{0};
    if (position < left_subtree_size) {
      root = root->left;
    } else if (position == left_subtree_size) {
      break;
    } else {
      root = root->right;
      position -= left_subtree_size + 1;
    }
  }
  return root;
}

template <typename Value>
tree_node<Value>* order_statistic_subtree(tree_node<Value>* root, size_t n) noexcept {
  const auto* const node = root;
  return const_cast<tree_node<Value>*>(order_statistic_subtree(node, n));
}

// return the first node whose key is not less than key
template <typename Key, typename Value, typename KeyComparator, typename KeyExtractor>
tree_node<Value>* lower_bound_subtree(
    tree_node<Value>* root,
    const Key& key,
    const KeyComparator& comparator,
    const KeyExtractor& extractor) noexcept {
  auto node = static_cast<tree_node<Value>*>(nullptr);
  while (root != nullptr) {
    if (!comparator(extractor(root->value), key)) {
      node = root;
      root = root->left;
    } else {
      root = root->right;
    }
  }
  return node;
}

// return the first node whose key is greater than key
template <typename Key, typename Value, typename KeyComparator, typename KeyExtractor>
tree_node<Value>* upper_bound_subtree(
    tree_node<Value>* root,
    const Key& key,
    const KeyComparator& comparator,
    const KeyExtractor& extractor) noexcept {
  auto node = static_cast<tree_node<Value>*>(nullptr);
  while (root != nullptr) {
    if (comparator(key, extractor(root->value))) {
      node = root;
      root = root->left;
    } else {
      root = root->right;
    }
  }
  return node;
}

template <typename Value>
tree_node<Value>* copy_subtree(const tree_node<Value>* root) {
  if (root == nullptr) {
    return nullptr;
  }
  auto node = create_node(root->value);
  assert(node != nullptr);
  node->size = root->size;
  node->left = copy_subtree(root->left);
  if (node->left != nullptr) {
    node->left->parent = node;
  }
  node->right = copy_subtree(root->right);
  if (node->right != nullptr) {
    node->right->parent = node;
  }
  return node;
}

// merge two subtrees under node `lhs` and `rhs`
// all keys in subtree of `lhs` must be strictly less then any key in subtree of `rhs`
template <typename Value>
tree_node<Value>* merge_subtrees(
    tree_node<Value>* lhs, tree_node<Value>* rhs) noexcept {
  assert(lhs == nullptr || lhs->parent == nullptr);
  assert(rhs == nullptr || rhs->parent == nullptr);
  if (lhs == nullptr) {
    return rhs;
  }
  if (rhs == nullptr) {
    return lhs;
  }
  auto max_lhs = lhs->rightmost_node();
  splay_node(max_lhs);
  assert(max_lhs->right == nullptr);
  max_lhs->right = rhs;
  rhs->parent = max_lhs;
  max_lhs->size += rhs->size;
  return max_lhs;
}

// split root node onto two trees `left` and `right` such that
// root node goes into the left tree
template <typename Value>
std::pair<tree_node<Value>*, tree_node<Value>*> split_left_subtree(tree_node<Value>* root) noexcept {
  assert(root != nullptr);
  assert(root->parent == nullptr);
  auto left = static_cast<tree_node<Value>*>(nullptr);
  auto right = static_cast<tree_node<Value>*>(nullptr);
  left = root;
  right = root->right;
  // forget relatives
  left->right = nullptr;
  left->parent = nullptr;
  if (right != nullptr) {
    right->parent = nullptr;
    left->size -= right->size;
  }
  return std::make_pair(left, right);
}

// split root node onto two trees `left` and `right` such that
// root node goes into the right tree
template <typename Value>
std::pair<tree_node<Value>*, tree_node<Value>*> split_right_subtree(tree_node<Value>* root) noexcept {
  assert(root != nullptr);
  assert(root->parent == nullptr);
  auto left = static_cast<tree_node<Value>*>(nullptr);
  auto right = static_cast<tree_node<Value>*>(nullptr);
  left = root->left;
  right = root;
  // forget relatives
  right->left = nullptr;
  right->parent = nullptr;
  if (left != nullptr) {
    left->parent = nullptr;
    right->size -= left->size;
  }
  return std::make_pair(left, right);
}

template <typename Value>
struct splay_tree_base {
  tree_node<Value>* root;
};

template <typename Value>
splay_tree_base<Value> create_tree() noexcept {
  auto tree = splay_tree_base<Value>{};
  tree.root = nullptr;
  return tree;
}

template <typename Value>
splay_tree_base<Value> create_tree(const Value& value) {
  auto tree = splay_tree_base<Value>{};
  tree.root = create_node(value);
  tree.root->parent = nullptr;
  tree.root->left = nullptr;
  tree.root->right = nullptr;
  return tree;
}

template <typename Value>
splay_tree_base<Value> copy_tree(const splay_tree_base<Value>& other) {
  auto tree = create_tree<Value>();
  tree.root = copy_subtree(other.root);
  return tree;
}

template <typename Value>
void swap_trees(splay_tree_base<Value>& lhs, splay_tree_base<Value>& rhs) noexcept {
  std::swap(lhs.root, rhs.root);
}

template <typename Value>
void clear_tree(splay_tree_base<Value>& tree) noexcept {
  destroy_substree(tree.root);
  tree.root = nullptr;
}

template <typename Value>
size_t get_size_tree(const splay_tree_base<Value>& tree) noexcept {
  return tree.root != nullptr ? tree.root->size : size_t{0};
}

template <typename Value>
bool is_empty_tree(const splay_tree_base<Value>& tree) noexcept {
  return tree.root == nullptr;
}

template <typename Value>
void splay_node_tree(splay_tree_base<Value>& tree, tree_node<Value>* node) noexcept {
  assert(node != nullptr);
  assert(node->find_root() == tree.root);
  splay_node(node);
  tree.root = node;
}

// find node with key equal to `key`, if doesn't exist return null
// rebalances the tree
template <typename Key, typename Value, typename KeyComparator, typename KeyExtractor>
tree_node<Value>* find_tree(
    splay_tree_base<Value>& tree,
    const Key& key,
    const KeyComparator& comparator,
    const KeyExtractor& extractor) {
  auto node = find_candidate_subtree(tree.root, key, comparator, extractor);
  if (node != nullptr) {
    splay_node_tree(tree, node);
    const auto strictly_less = comparator(extractor(node->value), key);
    const auto strictly_greater = comparator(key, extractor(node->value));
    if (strictly_less || strictly_greater) {
      return nullptr;
    }
  }
  return node;
}

// find the first node in `tree` with the key not less than `key`
template <typename Key, typename Value, typename KeyComparator, typename KeyExtractor>
tree_node<Value>* lower_bound_tree(
    splay_tree_base<Value>& tree,
    const Key& key,
    const KeyComparator& comparator,
    const KeyExtractor& extractor) {
  auto bound = lower_bound_subtree(tree.root, key, comparator, extractor);
  if (bound != nullptr) {
    splay_node_tree(tree, bound);
  }
  return bound;
}

// find the first node in `tree` with the key greater than `key`
template <typename Key, typename Value, typename KeyComparator, typename KeyExtractor>
tree_node<Value>* upper_bound_tree(
    splay_tree_base<Value>& tree,
    const Key& key,
    const KeyComparator& comparator,
    const KeyExtractor& extractor) {
  auto bound = upper_bound_subtree(tree.root, key, comparator, extractor);
  if (bound != nullptr) {
    splay_node_tree(tree, bound);
  }
  return bound;
}

// insert value `value` into the tree `tree` and rebalance the tree
template <typename Key, typename Value, typename KeyComparator, typename KeyExtractor>
tree_node<Value>* insert_tree(
    splay_tree_base<Value>& tree,
    const Value& value,
    const KeyComparator& comparator,
    const KeyExtractor& extractor) {
  auto node = static_cast<tree_node<Value>*>(nullptr);
  if (tree.root == nullptr) {
    auto new_node = create_node(value);
    tree.root = new_node;
    new_node->parent = nullptr;
    node = new_node;
  } else {
    node = insert_subtree<Key, Value, KeyComparator, KeyExtractor>(
      tree.root, value, comparator, extractor);
    if (node != nullptr) {
      splay_node_tree(tree, node);
    }
  }
  return node;
}

// erase node `node` from tree `tree`
template <typename Value>
tree_node<Value>* erase_tree(splay_tree_base<Value>& tree, tree_node<Value>* node) noexcept {
  assert(node->find_root() == tree.root);
  splay_node_tree(tree, node);
  const auto left = node->left;
  if (node->left != nullptr) {
    node->left->parent = nullptr;
  }
  const auto right = node->right;
  if (node->right != nullptr) {
    node->right->parent = nullptr;
  }
  destroy_node(node);
  tree.root = merge_subtrees(left, right);
  return right;
}

// split `this` tree into two trees: left tree and the right tree
// all nodes to the right of the `node` (with respect to ordering) go to the right tree
// the remaining nodes to the left tree
// if node is null, then all nodes go to the left tree
// after `tree` contains the left part, the right part is returned
template <typename Value>
splay_tree_base<Value> split_left_tree(
   splay_tree_base<Value>& tree, tree_node<Value>* node) noexcept {
  auto split = std::pair<tree_node<Value>*, tree_node<Value>*>{};
  if (node != nullptr) {
    assert(node->find_root() == tree.root);
    splay_node_tree(tree, node);
    split = split_left_subtree(tree.root);
  } else {
    split = std::make_pair(tree.root, nullptr);
  }
  tree.root = split.first;
  auto right_tree = create_tree<Value>();
  right_tree.root = split.second;
  return right_tree;
}

// split `this` tree into two trees: left tree and the right tree
// all nodes to the left of the `node` (with respect to ordering) go to the left tree
// the remaining nodes to the right tree
// if node is null, then all nodes go to the left tree
// after `tree` contains the left part, the right part is returned
template <typename Value>
splay_tree_base<Value> split_right_tree(
    splay_tree_base<Value>& tree, tree_node<Value>* node) noexcept {
  auto split = std::pair<tree_node<Value>*, tree_node<Value>*>{};
  if (node != nullptr) {
    assert(node->find_root() == tree.root);
    splay_node_tree(tree, node);
    split = split_right_subtree(tree.root);
  } else {
    split = std::make_pair(tree.root, nullptr);
  }
  tree.root = split.first;
  auto right_tree = create_tree<Value>();
  right_tree.root = split.second;
  return right_tree;
}

// merge nodes from `rhs` tree into `this` tree.
// all keys in `rhs` tree must be greater than keys in `this` tree
// after call `lhs` contains all nodes, `rhs` is empty
template <typename Value>
void merge_trees(splay_tree_base<Value>& lhs, splay_tree_base<Value>& rhs) noexcept {
  lhs.root = merge_subtrees(lhs.root, rhs.root);
  rhs.root = nullptr;
}

// find nth-node (0-based indexing) in the tree with respect to key ordering
// rebalances the tree
template <typename Value>
tree_node<Value>* order_statistic_tree(splay_tree_base<Value>& tree, size_t n) noexcept {
  auto node = order_statistic_subtree(tree.root, n);
  if (node != nullptr) {
    splay_node_tree(tree, node);
  }
  return node;
}


template <typename Value, typename KeyComparator, typename KeyExtractor>
bool is_less(
    const splay_tree_base<Value>& lhs,
    const splay_tree_base<Value>& rhs,
    const KeyComparator& comparator,
    const KeyExtractor& extractor) {
  return (
    (lhs.root == nullptr) ||
    (rhs.root == nullptr) ||
    comparator(
      extractor(lhs.root->rightmost_node()->value),
      extractor(rhs.root->leftmost_node()->value)));
}

template <typename Value>
void print_tree(std::ostream& out, const splay_tree_base<Value>& tree) {
  print_subtree(out, tree.root);
}

}  // namespace detail
}  // namespace splay

#endif  // SPLAY_TREE_TREE_NODE_H_