#include <cassert>
#include <iostream>

namespace splay {

// Node of the splay tree
// Invariants:
// 1. any key in the left subtree is strictly less than value
// 2. any key in the right subtree is strictly greater than value
// 3. no duplicate keys
template <typename Value>
struct splay_tree_node {

  splay_tree_node(const Value& value)
    : value{value}
    , node_count{1}
    , parent{nullptr}
    , left{nullptr}
    , right{nullptr}
  {}

  ~splay_tree_node() {
    auto* const node = this;
    node->parent = nullptr;
    node->left = nullptr;
    node->right = nullptr;
    node->node_count = 0;
  }

  bool is_root() const {
    const auto* node = this;
    return node->parent == nullptr;
  }

  bool is_left_child() const {
    const auto* node = this;
    return !node->is_root() && node->parent->left == node;
  }

  bool is_right_child() const {
    const auto* node = this;
    return !node->is_root() && node->parent->right == node;
  }

  const splay_tree_node<Value>* find_root() const {
    const auto* node = this;
    if (node == nullptr) {
      return nullptr;
    }
    while (node->parent != nullptr) {
      node = node->parent;
    }
    return node;
  }

  const splay_tree_node<Value>* rightmost_node() const {
    const auto* node = this;
    while (node->right != nullptr) {
      node = node->right;
    }
    return node;
  }

  splay_tree_node<Value>* rightmost_node() {
    const auto* const node = this;
    return const_cast<splay_tree_node<Value>*>(node->rightmost_node());
  }

  const splay_tree_node<Value>* leftmost_node() const {
    const auto* node = this;
    while (node->left != nullptr) {
      node = node->left;
    }
    return node;
  }

  splay_tree_node<Value>* leftmost_node() {
    const auto* const node = this;
    return const_cast<splay_tree_node<Value>*>(node->leftmost_node());
  }

  const splay_tree_node<Value>* next_node() const {
    const auto* node = this;
    auto next = static_cast<const splay_tree_node<Value>*>(nullptr);
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

  splay_tree_node<Value>* next_node() {
    const auto* const node = this;
    return const_cast<splay_tree_node<Value>*>(node->next_node());
  }

  const splay_tree_node<Value>* prev_node() const {
    const auto* node = this;
    auto prev = static_cast<const splay_tree_node<Value>*>(nullptr);
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

  splay_tree_node<Value>* prev_node() {
    const auto* const node = this;
    return const_cast<splay_tree_node<Value>*>(node->prev_node());
  }

  Value value;
  uint64_t node_count;
  splay_tree_node<Value>* parent;
  splay_tree_node<Value>* left;
  splay_tree_node<Value>* right;
};

template <typename Value>
std::ostream& operator << (std::ostream& out, const splay_tree_node<Value>& node) {
  out << "[" << node.value << "]"; // << ", " << node.node_count << "]";
  return out;
}


template <typename Value>
void destroy_node(splay_tree_node<Value>* node) {
  assert(node != nullptr);
  delete node;
}

template <typename Value>
splay_tree_node<Value>* create_node(const Value& value) {
  return new splay_tree_node<Value>(value);
}

template <typename Value>
void print_subtree(std::ostream& out, const splay_tree_node<Value>* root) {
  out << "(";
  if (root != nullptr) {
    print_subtree(out, root->left);
    out << *root;
    print_subtree(out, root->right);
  }
  out << ")";
}

// destroy subtree under the node `root`
template <typename Value>
void destroy_substree(splay_tree_node<Value>* root) {
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
void update_node_count(splay_tree_node<Value>* node) {
  if (node != nullptr) {
    node->node_count = uint64_t{1};
    node->node_count += (node->left != nullptr ? node->left->node_count : uint64_t{0});
    node->node_count += (node->right != nullptr ? node->right->node_count : uint64_t{0});
  }
}

// insert new node `new_node` to subtree at root `root`, without rebalancing
template <typename Key, typename Value, typename KeyExtractor, typename KeyComparator>
void insert_node(
    splay_tree_node<Value>* root,
    splay_tree_node<Value>* new_node,
    const KeyExtractor& extractor,
    const KeyComparator& comparator) {
  assert(root != nullptr);
  assert(new_node != nullptr);
  assert(new_node->node_count == 1);
  assert(new_node->left == nullptr);
  assert(new_node->right == nullptr);
  if (comparator(extractor(new_node->value), extractor(root->value))) {
    if (root->left == nullptr) {
      root->left = new_node;
      new_node->parent = root;
    } else {
      insert_node<Key, Value, KeyExtractor, KeyComparator>(
        root->left, new_node, extractor, comparator);
    }
    ++root->node_count;
  } else if (comparator(extractor(root->value), extractor(new_node->value))) {
    if (root->right == nullptr) {
      root->right = new_node;
      new_node->parent = root;
    } else {
      insert_node<Key, Value, KeyExtractor, KeyComparator>(
        root->right, new_node, extractor, comparator);
    }
    ++root->node_count;
  } else {
    // key already in the tree, do nothing
    ;
  }
}

template <typename Value>
void left_rotate_node(splay_tree_node<Value>* node) {
  /* u is node, a is parent, B is branch, p is granny
          p             p
          |             |
          a             u
         / \           / \
        u   C    =>   A   a
       / \               / \
      A  B              B  C
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
  update_node_count(parent);
  update_node_count(node);
}

template <typename Value>
void right_rotate_node(splay_tree_node<Value>* node) {
  /* u is node, a is parent, B is branch, p is granny

        p             p
        |             |
        a             u
       / \           / \
      C   u    =>   a  A
         / \       / \
        B  A      C  B
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
  update_node_count(parent);
  update_node_count(node);
}

template <typename Value>
void rotate_node(splay_tree_node<Value>* node) {
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
splay_tree_node<Value>* splay(splay_tree_node<Value>* node) {
  /* ------------------------------------------------------------------------------------
     zig_zig
            p                                                                p
            |                                 p                              |
            b                                 |                              u
           / \                                a                             / \
          a   D    left_rotate_node(a)      /   \    left_rotate_node(u)   A   a
         / \                              u       b                           / \
        u   C                            / \     / \                         B   b
       / \                              A   B   C   D                           / \
      A   B                                                                    C   D
     ------------------------------------------------------------------------------------
     zig_zag
        p                             p
        |                             |                                p
        b                             b                                |
       / \                           / \                               u
      D   a    left_rotate_node(u)  D   u   right_rotate_node(u)     /   \
         / \                           / \                         b       a
        u   C                         A   a                       / \     / \
       / \                               / \                     D   A   B   C
      A   B                             B   C
     ------------------------------------------------------------------------------------
     zag_zig
           p                                  p
           |                                  |                                p
           b                                  b                                |
          / \                                / \                               u
         a   D    right_rotate_node(u)      u   D    left_rotate_node(u)     /   \
        / \                                / \                             a       b
       C   u                              a   B                           / \     / \
          / \                            / \                             C   A   B   D
         A   B                          C   A
     ------------------------------------------------------------------------------------
     zag_zag
        p                                                                  p
        |                                p                                 |
        b                                |                                 u
       / \                               a                                / \
      D   a   right_rotate_node(a)     /   \   right_rotate_node(u)      a   B
         / \                         b       u                          / \
        C   u                       / \     / \                        b   A
           / \                     D   C   A   B                      / \
          A   B                                                      D   C
     ------------------------------------------------------------------------------------
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
  return node;
}

// find node with key `key` in the subtree under node `root`. If such node doesn't exist
// return the last node during this search
template <typename Key, typename Value, typename KeyExtractor, typename KeyComparator>
const splay_tree_node<Value>* find_candidate(
    const splay_tree_node<Value>* root,
    const Key& key,
    const KeyExtractor& extractor,
    const KeyComparator& comparator) {
  assert(root != nullptr);
  auto node = static_cast<const splay_tree_node<Value>*>(nullptr);
  if (comparator(key, extractor(root->value))) {
    if (root->left == nullptr) {
      node = root;
    } else {
      node = find_candidate(root->left, key, extractor, comparator);
    }
  } else if (comparator(extractor(root->value), key)) {
    if (root->right == nullptr) {
      node = root;
    } else {
      node = find_candidate(root->right, key, extractor, comparator);
    }
  } else {
    node = root;
  }
  return node;
}

template <typename Key, typename Value, typename KeyExtractor, typename KeyComparator>
splay_tree_node<Value>* find_candidate(
    splay_tree_node<Value>* root,
    const Key& key,
    const KeyExtractor& extractor,
    const KeyComparator& comparator) {
  const auto* const node = root;
  return const_cast<splay_tree_node<Value>*>(find_candidate(node, key, extractor, comparator));
}

template <typename Value>
splay_tree_node<Value>* copy_subtree(const splay_tree_node<Value>* root) {
  if (root == nullptr) {
    return nullptr;
  }
  auto node = create_node(root->value);
  assert(node != nullptr);
  node->node_count = root->node_count;
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
splay_tree_node<Value>* merge_subtrees(splay_tree_node<Value>* lhs, splay_tree_node<Value>* rhs) {
  assert(lhs == nullptr || lhs->parent == nullptr);
  assert(rhs == nullptr || rhs->parent == nullptr);
  if (lhs == nullptr) {
    return rhs;
  }
  if (rhs == nullptr) {
    return lhs;
  }
  auto max_lhs = splay(lhs->rightmost_node());
  assert(max_lhs->right == nullptr);
  max_lhs->right = rhs;
  rhs->parent = max_lhs;
  max_lhs->node_count += rhs->node_count;
  return max_lhs;
}

// split tree under node `root` onto two trees `left` and `right` such that
// any key in `left` is less than `key`
// any key in `right` is greater or equal to `key`
template <typename Key, typename Value, typename KeyExtractor, typename KeyComparator>
std::pair<splay_tree_node<Value>*, splay_tree_node<Value>*> split_subtree(
    splay_tree_node<Value>* root,
    const Key& key,
    const KeyExtractor& extractor,
    const KeyComparator& comparator) {
  assert(root != nullptr);
  assert(root->parent == nullptr);
  auto left = static_cast<splay_tree_node<Value>*>(nullptr);
  auto right = static_cast<splay_tree_node<Value>*>(nullptr);
  if (root != nullptr) {
    root = splay(find_candidate(root, key, extractor, comparator));
    if (comparator(extractor(root->value), key)) {
      left = root;
      right = root->right;
      // forget relatives
      left->right = nullptr;
      left->parent = nullptr;
      if (right != nullptr) {
        right->parent = nullptr;
        left->node_count -= right->node_count;
      }
    } else {
      left = root->left;
      right = root;
      // forget relatives
      right->left = nullptr;
      right->parent = nullptr;
      if (left != nullptr) {
        left->parent = nullptr;
        right->node_count -= left->node_count;
      }
    }
  }
  return std::make_pair(left, right);
}

template <typename Key, typename Value, typename KeyExtractor, typename KeyComparator>
struct splay_tree {
  explicit splay_tree(const KeyComparator& comparator = KeyComparator{})
    : root{nullptr}
    , extractor{}
    , comparator{comparator}
  {}

  splay_tree(std::initializer_list<Value> init, const KeyComparator& comparator = KeyComparator{})
    : splay_tree{std::begin(init), std::end(init), comparator}
  {}

  template <typename Iter>
  splay_tree(Iter first, Iter last, const KeyComparator& comparator = KeyComparator{})
    : splay_tree(comparator) {
    for (auto it = first; it != last; ++it) {
      this->insert(*it);
    }
  }

  splay_tree(const splay_tree& other)
    : splay_tree{} {
    this->root = copy_subtree(other.root);
  }

  splay_tree(splay_tree<Key, Value, KeyExtractor, KeyComparator>&& other)
    : splay_tree{} {
    this->swap(other);
  }

  splay_tree<Key, Value, KeyExtractor, KeyComparator>& operator = (
      const splay_tree<Key, Value, KeyExtractor, KeyComparator>& other) {
    if (this != std::addressof(other)) {
      auto temp = splay_tree<Key, Value, KeyExtractor, KeyComparator>{other};
      this->swap(temp);
    }
    return *this;
  }

  splay_tree<Key, Value, KeyExtractor, KeyComparator>& operator = (
     splay_tree<Key, Value, KeyExtractor, KeyComparator>&& other) {
    this->swap(other);
    return *this;
  }

  ~splay_tree() {
    auto* const tree = this;
    destroy_substree(tree->root);
  }

  const splay_tree_node<Value>* find(const Key& key) const {
    auto* const tree = this;
    if (tree->root == nullptr) {
      return nullptr;
    }
    auto node = find_candidate(tree->root, key, tree->extractor, tree->comparator);
    const auto strictly_less = tree->comparator(tree->extractor(node->value), key);
    const auto strictly_greater = tree->comparator(key, tree->extractor(node->value));
    if (strictly_less || strictly_greater) {
      return nullptr;
    }
    return node;
  }

  splay_tree_node<Value>* find(const Key& key) {
    const auto* const tree = this;
    return const_cast<splay_tree_node<Value>*>(tree->find(key));
  }

  // insert value `value` into the tree `tree` and rebalance the tree
  splay_tree_node<Value>* insert(const Value& value) {
    auto* const tree = this;
    auto new_node = create_node(value);
    assert(new_node != nullptr);
    if (tree->root == nullptr) {
      tree->root = new_node;
      new_node->parent = nullptr;
    } else {
      insert_node<Key, Value, KeyExtractor, KeyComparator>(
        tree->root, new_node, tree->extractor, tree->comparator);
      tree->root = splay(new_node);
    }
    return new_node;
  }

  // erase node `node` from tree `tree`
  splay_tree_node<Value>* erase(splay_tree_node<Value>* node) {
    auto* const tree = this;
    splay(node);
    const auto left = node->left;
    if (node->left != nullptr) {
      node->left->parent = nullptr;
    }
    const auto right = node->right;
    if (node->right != nullptr) {
      node->right->parent = nullptr;
    }
    destroy_node(node);
    tree->root = merge_subtrees(left, right);
    return right;
  }

  std::pair<
    splay_tree<Key, Value, KeyExtractor, KeyComparator>,
    splay_tree<Key, Value, KeyExtractor, KeyComparator>>
  split(const Value& value) {
    auto* const tree = this;
    auto trees = split_subtree(
      tree->root, tree->extractor(value), tree->extractor, tree->comparator);
    tree->root = nullptr;
    auto left_tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{};
    left_tree.root = trees.first;
    auto right_tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{};
    right_tree.root = trees.second;

    return std::make_pair(left_tree, right_tree);
  }

  void merge(splay_tree<Key, Value, KeyExtractor, KeyComparator>& rhs) {
    auto* const lhs = this;
    lhs->root = merge_subtrees(lhs->root, rhs.root);
    rhs.root = nullptr;
  }

  void swap(splay_tree<Key, Value, KeyExtractor, KeyComparator>& other) {
    auto* const tree = this;
    std::swap(tree->root, other.root);
  }

  splay_tree_node<Value>* root;
  KeyExtractor extractor;
  KeyComparator comparator;
};

template <typename Key, typename Value, typename KeyExtractor, typename KeyComparator>
std::ostream& operator << (
    std::ostream& out, const splay_tree<Key, Value, KeyExtractor, KeyComparator>& tree) {
  print_subtree(out, tree.root);
  return out;
}

}  // namespace splay
