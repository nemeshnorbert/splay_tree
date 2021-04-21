#include <algorithm>
#include <random>

#include "splay_tree.h"
#include "implicit_splay_tree.h"

namespace splay {
namespace test {

template <typename Value>
struct size_check_result {
  bool ok;
  const tree_node<Value>* broken_node;
};

template <typename Value>
size_check_result<Value> check_size(const tree_node<Value>* node) {
  if (node == nullptr) {
    return size_check_result<Value>{true, nullptr};
  }
  const auto left_size = node->left != nullptr ? node->left->size : uint64_t{0};
  const auto right_size = node->right != nullptr ? node->right->size : uint64_t{0};
  if (node->size != left_size + right_size + 1) {
    return size_check_result<Value>{false, node};
  }
  const auto left_result = check_size(node->left);
  const auto right_result = check_size(node->right);
  if (!left_result.ok) {
    return left_result;
  }
  if (!right_result.ok) {
    return right_result;
  }
  return size_check_result<Value>{true, nullptr};
}

enum class structure_type : uint32_t {
  kOk,
  kParentForgotNode,
  kLeftChildForgotParent,
  kRightChildForgotParent
};

template <typename Value>
struct structure_check_result {
  structure_type type;
  const tree_node<Value>* broken_node;
};

template <typename Value>
structure_check_result<Value> check_structure(const tree_node<Value>* node) {
  if (node == nullptr) {
    return structure_check_result<Value>{structure_type::kOk, nullptr};
  }
  const auto left_check = check_structure(node->left);
  if (left_check.type != structure_type::kOk) {
    return left_check;
  }
  const auto right_check = check_structure(node->right);
  if (right_check.type != structure_type::kOk) {
    return right_check;
  }
  if (node->parent != nullptr) {
    if (node->parent->left != node && node->parent->right != node) {
      return structure_check_result<Value>{structure_type::kParentForgotNode, node};
    }
  }
  if (node->left != nullptr) {
    if (node->left->parent != node) {
      return structure_check_result<Value>{structure_type::kLeftChildForgotParent, node};
    }
  }
  if (node->right != nullptr) {
    if (node->right->parent != node) {
      return structure_check_result<Value>{structure_type::kRightChildForgotParent, node};
    }
  }
  return structure_check_result<Value>{structure_type::kOk, nullptr};
}

enum class ordering_type : uint32_t {
  kBalanced,
  kUnbalanced
};

template <typename Value>
struct ordering_check_result {
  ordering_type type;
  const tree_node<Value>* broken_node;
  const tree_node<Value>* min_node;
  const tree_node<Value>* max_node;
};

template <typename Key, typename Value, typename KeyComparator, typename KeyExtractor>
ordering_check_result<Value> check_ordering(
    const tree_node<Value>* node,
    const KeyExtractor& extractor,
    const KeyComparator& comparator) {
  if (node == nullptr) {
    return ordering_check_result<Value>{ordering_type::kBalanced, nullptr, nullptr, nullptr};
  }
  auto check_result = ordering_check_result<Value>{ordering_type::kBalanced, nullptr, node, node};
  // checks
  const auto left_check_result = check_ordering<Key, Value, KeyComparator, KeyExtractor>(
    node->left, extractor, comparator);
  if (left_check_result.type != ordering_type::kBalanced) {
    check_result.type = ordering_type::kUnbalanced;
    check_result.broken_node = left_check_result.broken_node;
  }
  const auto right_check_result = check_ordering<Key, Value, KeyComparator, KeyExtractor>(
    node->right, extractor, comparator);
  if (right_check_result.type != ordering_type::kBalanced) {
    check_result.type = ordering_type::kUnbalanced;
    check_result.broken_node = right_check_result.broken_node;
  }
  const auto left_max_node = left_check_result.max_node;
  if (left_max_node != nullptr &&
      !comparator(extractor(left_max_node->value), extractor(node->value))) {
    check_result.type = ordering_type::kUnbalanced;
    check_result.broken_node = node;
  }
  const auto right_min_node = right_check_result.min_node;
  if (right_min_node != nullptr &&
      comparator(extractor(right_min_node->value), extractor(node->value))) {
    check_result.type = ordering_type::kUnbalanced;
    check_result.broken_node = node;
  }
  // recalc min and max
  const auto left_min_node = left_check_result.min_node;
  if (left_min_node != nullptr &&
      comparator(extractor(left_min_node->value), extractor(check_result.min_node->value))) {
    check_result.min_node = left_min_node;
  }
  const auto right_max_node = right_check_result.max_node;
  if (right_max_node != nullptr &&
      comparator(extractor(check_result.max_node->value), extractor(right_max_node->value))) {
    check_result.max_node = right_max_node;
  }
  return check_result;
}

template <typename Key, typename Value, typename KeyComparator, typename KeyExtractor>
void check_subtree(
    const tree_node<Value>* node,
    const KeyComparator& comparator,
    const KeyExtractor& extractor) {
  auto structure_check = check_structure(node);
  assert(structure_check.type == structure_type::kOk);
  auto size_check = check_size(node);
  assert(size_check.ok);
  auto order_check = check_ordering<Key, Value, KeyComparator, KeyExtractor>(
    node, extractor, comparator);
  assert(order_check.type == ordering_type::kBalanced);
}

template <typename Key, typename Value, typename KeyComparator, typename KeyExtractor>
void check_tree(const splay_tree<Key, Value, KeyComparator, KeyExtractor>& tree) {
  check_subtree<Key, Value, KeyComparator, KeyExtractor>(
    tree.root(), tree.key_comparator(), tree.key_extractor());
}

template <typename Value>
void check_subtree(const tree_node<Value>* node) {
  auto structure_check = check_structure(node);
  assert(structure_check.type == structure_type::kOk);
  auto size_check = check_size(node);
  assert(size_check.ok);
}

template <typename Value>
void check_tree(const implicit_splay_tree<Value>& tree) {
  check_subtree<Value>(tree.root());
}

class Int64 {
 public:
  explicit Int64(int64_t value) noexcept
    : value{value} {
  }

  bool operator == (const Int64& rhs) const {
    return this->value == rhs.value;
  }

  bool operator != (const Int64& rhs) const {
    return this->value != rhs.value;
  }

  int64_t value;
};

std::ostream& operator << (std::ostream& out, const Int64& value) {
  out << value.value;
  return out;
}

class Int32 {
 public:
  explicit Int32(int32_t value) noexcept
    : value{value} {
  }

  int32_t value;
};

class Int32Extractor {
 public:
  Int32 operator ()(const Int64& value) const noexcept {
    return Int32{static_cast<int32_t>(value.value)};
  }
};

class Int32Comparator {
 public:
  bool operator () (const Int32& lhs, const Int32& rhs) const noexcept {
    return lhs.value < rhs.value;
  }
};

class splay_tree_tester {
 public:
  using Value = Int64;
  using Key = Int32;
  using KeyExtractor = Int32Extractor;
  using KeyComparator = Int32Comparator;
  using tree_type = splay_tree<Key, Value, KeyComparator, KeyExtractor>;

  void test_create_and_destroy_empty_tree() {
    auto tree = tree_type{};
    check_tree(tree);
  }

  void test_insert_into_empty_tree() {
    auto tree = tree_type{{Value{1}}};
    check_tree(tree);
    assert(tree.root() != nullptr);
    assert(tree.root()->value == Value{1});
    assert(tree.root()->size == 1);
  }

  void test_insert_exact_structure_easy() {
    auto tree = tree_type{
      {Value{1}, Value{2}, Value{4}, Value{3}}};
    check_tree(tree);

    // ((()[1, 1]())[2, 2]())[3, 4](()[4, 1]()))
    assert(tree.root() != nullptr);
    assert(tree.root()->value == Value{3});
    assert(tree.root()->size == 4);
    assert(tree.root()->parent == nullptr);
    assert(tree.root()->right != nullptr);
    assert(tree.root()->right != nullptr);

    assert(tree.root()->right->parent == tree.root());
    assert(tree.root()->right->value == Value{4});
    assert(tree.root()->right->size == 1);
    assert(tree.root()->right->left == nullptr);
    assert(tree.root()->right->right == nullptr);

    assert(tree.root()->left->parent == tree.root());
    assert(tree.root()->left->value == Value{2});
    assert(tree.root()->left->size == 2);
    assert(tree.root()->left->left != nullptr);
    assert(tree.root()->left->right == nullptr);

    assert(tree.root()->left->left->parent = tree.root()->left);
    assert(tree.root()->left->left->value == Value{1});
    assert(tree.root()->left->left->size == 1);
    assert(tree.root()->left->left->left == nullptr);
    assert(tree.root()->left->left->right == nullptr);
  }

  void test_insert_exact_structure_hard() {
    auto tree = tree_type{
      {Value{1}, Value{2}, Value{-12}, Value{15}, Value{-2}, Value{-7}, Value{4}}};
    check_tree(tree);

    assert(tree.root() != nullptr);
    assert(tree.root()->value == Value{4});
    assert(tree.root()->parent == nullptr);
    assert(tree.root()->left != nullptr);
    assert(tree.root()->right != nullptr);
    assert(tree.root()->left->parent == tree.root());
    assert(tree.root()->left->value == Value{-7});
    assert(tree.root()->left->left != nullptr);
    assert(tree.root()->left->right != nullptr);
    assert(tree.root()->right->parent == tree.root());
    assert(tree.root()->right->value == Value{15});
    assert(tree.root()->right->left == nullptr);
    assert(tree.root()->right->right == nullptr);
    assert(tree.root()->left->left->parent == tree.root()->left);
    assert(tree.root()->left->left->value == Value{-12});
    assert(tree.root()->left->left->left == nullptr);
    assert(tree.root()->left->left->right == nullptr);
    assert(tree.root()->left->right->parent == tree.root()->left);
    assert(tree.root()->left->right->value == Value{-2});
    assert(tree.root()->left->right->left == nullptr);
    assert(tree.root()->left->right->right != nullptr);
    assert(tree.root()->left->right->right->parent == tree.root()->left->right);
    assert(tree.root()->left->right->right->value == Value{2});
    assert(tree.root()->left->right->right->right == nullptr);
    assert(tree.root()->left->right->right->left != nullptr);
    assert(tree.root()->left->right->right->left->parent == tree.root()->left->right->right);
    assert(tree.root()->left->right->right->left->value == Value{1});
    assert(tree.root()->left->right->right->left->left == nullptr);
    assert(tree.root()->left->right->right->left->right == nullptr);
  }

  void test_next_node() {
    auto values = std::vector<int64_t>{{1, 2, -12, 15, -2, -7, 4}};
    auto tree = tree_type{};
    check_tree(tree);
    for (const auto& value : values) {
      tree.insert(Value{value});
    }
    check_tree(tree);
    assert(tree.root() != nullptr);
    assert(tree.root()->size == values.size());
    auto it = tree.root()->leftmost_node();
    std::sort(std::begin(values), std::end(values));
    auto current = std::begin(values);
    while (it != nullptr) {
      assert(it->value == Value{*current});
      ++current;
      it = it->next_node();
    }
  }

  void test_prev_node() {
    auto values = std::vector<int64_t>{{1, 2, -12, 15, -2, -7, 4}};
    auto tree = tree_type{};
    check_tree(tree);
    for (const auto& value : values) {
      tree.insert(Value{value});
    }
    check_tree(tree);
    assert(tree.root() != nullptr);
    assert(tree.root()->size == values.size());
    auto it = tree.root()->rightmost_node();
    std::sort(std::begin(values), std::end(values));
    auto current = std::rbegin(values);
    while (it != nullptr) {
      assert(it->value == Value{*current});
      ++current;
      it = it->prev_node();
    }
    check_tree(tree);
  }

  void test_copy_empty_tree() {
    auto tree = tree_type{};
    check_tree(tree);
    auto copied = tree;
    assert(copied.empty());
    assert(copied.size() == 0);
    check_tree(tree);
  }

  void test_copy_tree() {
    auto values = std::vector<int64_t>{{1, 2, -12, 15, -2, -7, 4}};
    auto tree = tree_type{};
    check_tree(tree);
    for (const auto& value : values) {
      tree.insert(Value{value});
    }
    check_tree(tree);
    auto copied = tree;
    check_tree(copied);
    assert(copied.root() != nullptr);
    assert(copied.root()->size == values.size());
    auto it = copied.root()->leftmost_node();
    std::sort(std::begin(values), std::end(values));
    auto current = std::begin(values);
    while (it != nullptr) {
      assert(it->value == Value{*current});
      ++current;
      it = it->next_node();
    }
    check_tree(copied);
  }

  void test_swap_trees() {
    auto lhs_values = std::vector<int64_t>{{1, 2, -12, 15, -2, -7, 4}};
    auto lhs_tree = tree_type{};
    check_tree(lhs_tree);
    for (const auto& value : lhs_values) {
      lhs_tree.insert(Value{value});
    }
    check_tree(lhs_tree);
    assert(lhs_tree.size() == lhs_values.size());

    auto rhs_values = std::vector<int64_t>{{1, 2, 4, 3}};
    auto rhs_tree = tree_type{};
    check_tree(rhs_tree);
    for (const auto& value : rhs_values) {
      rhs_tree.insert(Value{value});
    }
    check_tree(rhs_tree);
    assert(rhs_tree.size() == rhs_values.size());

    lhs_tree.swap(rhs_tree);

    std::sort(std::begin(rhs_values), std::end(rhs_values));
    auto rhs_current = std::begin(rhs_values);
    auto lhs_it = lhs_tree.root()->leftmost_node();
    while (lhs_it != nullptr) {
      assert(lhs_it->value == Value{*rhs_current});
      ++rhs_current;
      lhs_it = lhs_it->next_node();
    }
    check_tree(lhs_tree);

    std::sort(std::begin(lhs_values), std::end(lhs_values));
    auto lhs_current = std::begin(lhs_values);
    auto rhs_it = rhs_tree.root()->leftmost_node();
    while (rhs_it != nullptr) {
      assert(rhs_it->value == Value{*lhs_current});
      ++lhs_current;
      rhs_it = rhs_it->next_node();
    }
    check_tree(rhs_tree);
  }

  void test_find_existing_value() {
    auto tree = tree_type{{Value{1}}};
    check_tree(tree);
    auto node = tree.find(Key{1});
    check_tree(tree);
    assert(node != nullptr);
    assert(node->value == Value{1});
  }

  void test_find_missing_value() {
    auto tree = tree_type{{Value{1}}};
    check_tree(tree);
    auto node = tree.find(Key{2});
    check_tree(tree);
    assert(node == nullptr);
  }

  void test_find_batch() {
    const auto present_values = std::vector<int32_t>{{1, 2, 3, -1, 5, -2}};
    const auto missing_values = std::vector<int32_t>{{100, 200, 300, -100, 500, -200}};
    auto tree = tree_type{};
    check_tree(tree);
    for (const auto& value : present_values) {
      tree.insert(Value{value});
    }
    check_tree(tree);
    for (const auto& value : present_values) {
      auto node = tree.find(Key{value});
      check_tree(tree);
      assert(node != nullptr);
      assert(node->value == Value{value});
    }
    for (const auto& value : missing_values) {
      auto node = tree.find(Key{value});
      check_tree(tree);
      assert(node == nullptr);
    }
  }

  void test_order_statistic_empty_tree() {
    auto tree = tree_type{};
    check_tree(tree);
    assert(tree.empty());
    for (auto idx = size_t{0}; idx < 5; ++idx) {
      auto node = tree.order_statistic(idx);
      check_tree(tree);
      assert(node == nullptr);
    }
  }

  void test_order_statistic() {
    auto values = std::vector<int64_t>{{1, 2, -12, 15, -2, -7, 4}};
    auto tree = tree_type{};
    check_tree(tree);
    for (const auto& value : values) {
      tree.insert(Value{value});
    }
    check_tree(tree);
    assert(tree.size() == values.size());
    std::sort(std::begin(values), std::end(values));
    for (auto idx = size_t{0}; idx < values.size(); ++idx) {
      auto node = tree.order_statistic(idx);
      check_tree(tree);
      assert(node != nullptr);
      assert(node->value == Value{values[idx]});
    }
  }

  void test_order_statistic_out_of_range() {
    auto values = std::vector<int64_t>{{1, 2, -12, 15, -2, -7, 4}};
    auto tree = tree_type{};
    check_tree(tree);
    for (const auto& value : values) {
      tree.insert(Value{value});
    }
    check_tree(tree);
    assert(tree.size() == values.size());
    for (auto idx = values.size(); idx < 2 * values.size(); ++idx) {
      auto node = tree.order_statistic(idx);
      check_tree(tree);
      assert(node == nullptr);
    }
  }

  void test_merge_two_empty_trees() {
    auto lhs_tree = tree_type{};
    check_tree(lhs_tree);
    auto rhs_tree = tree_type{};
    check_tree(rhs_tree);
    lhs_tree.merge(rhs_tree);
    check_tree(lhs_tree);
    check_tree(rhs_tree);
    assert(rhs_tree.empty());
    assert(rhs_tree.root() == nullptr);
  }

  void test_split_left_single_node_tree() {
    {
      auto tree = tree_type{{Value{1}}};
      check_tree(tree);
      auto split_node = tree.find(Key{0});
      check_tree(tree);
      auto right_tree = tree.split_left(split_node);
      check_tree(tree);
      check_tree(right_tree);
      auto& left_tree = tree;
      assert(right_tree.root() == nullptr);
      assert(left_tree.root() != nullptr);
      assert(left_tree.root()->value == Value{1});
      assert(left_tree.root()->size == 1);
    }
    {
      auto tree = tree_type{{Value{1}}};
      check_tree(tree);
      auto split_node = tree.find(Key{1});
      check_tree(tree);
      auto right_tree = tree.split_left(split_node);
      check_tree(tree);
      check_tree(right_tree);
      auto& left_tree = tree;
      assert(right_tree.root() == nullptr);
      assert(left_tree.root() != nullptr);
      assert(left_tree.root()->value == Value{1});
      assert(left_tree.root()->size == 1);
    }
    {
      auto tree = tree_type{{Value{1}}};
      check_tree(tree);
      auto split_node = tree.find(Key{2});
      check_tree(tree);
      auto right_tree = tree.split_left(split_node);
      check_tree(tree);
      check_tree(right_tree);
      auto& left_tree = tree;
      assert(right_tree.root() == nullptr);
      assert(left_tree.root() != nullptr);
      assert(left_tree.root()->value == Value{1});
      assert(left_tree.root()->size == 1);
    }
  }

  void test_split_right_single_node_tree() {
    {
      auto tree = tree_type{{Value{1}}};
      check_tree(tree);
      auto split_node = tree.find(Key{0});
      check_tree(tree);
      auto right_tree = tree.split_right(split_node);
      check_tree(tree);
      check_tree(right_tree);
      auto& left_tree = tree;
      assert(right_tree.root() == nullptr);
      assert(left_tree.root() != nullptr);
      assert(left_tree.root()->value == Value{1});
      assert(left_tree.root()->size == 1);
    }
    {
      auto tree = tree_type{{Value{1}}};
      check_tree(tree);
      auto split_node = tree.find(Key{1});
      check_tree(tree);
      auto right_tree = tree.split_right(split_node);
      check_tree(tree);
      check_tree(right_tree);
      auto& left_tree = tree;
      assert(left_tree.root() == nullptr);
      assert(right_tree.root() != nullptr);
      assert(right_tree.root()->value == Value{1});
      assert(right_tree.root()->size == 1);
    }
    {
      auto tree = tree_type{{Value{1}}};
      check_tree(tree);
      auto split_node = tree.find(Key{2});
      check_tree(tree);
      auto right_tree = tree.split_right(split_node);
      check_tree(tree);
      check_tree(right_tree);
      auto& left_tree = tree;
      assert(right_tree.root() == nullptr);
      assert(left_tree.root() != nullptr);
      assert(left_tree.root()->value == Value{1});
      assert(left_tree.root()->size == 1);
    }
  }

  void test_split_left() {
    auto tree = tree_type{
      {Value{1}, Value{4}, Value{3}, Value{2}, Value{7}, Value{0}}};
    check_tree(tree);
    auto split_node = tree.find(Key{3});
    check_tree(tree);
    auto right_tree = tree.split_left(split_node);
    check_tree(tree);
    auto& left_tree = tree;

    auto left_it = left_tree.root()->leftmost_node();
    auto left_answers = std::vector<int64_t>{{0, 1, 2, 3}};
    auto left_current = std::begin(left_answers);
    while (left_it != nullptr) {
      assert(left_it->value == Value{*left_current});
      left_it = left_it->next_node();
      ++left_current;
    }
    check_tree(left_tree);

    auto right_it = right_tree.root()->leftmost_node();
    auto right_answers = std::vector<int64_t>{{4, 7}};
    auto right_current = std::begin(right_answers);
    while (right_it != nullptr) {
      assert(right_it->value == Value{*right_current});
      right_it = right_it->next_node();
      ++right_current;
    }
    check_tree(right_tree);
  }

  void test_split_right() {
    auto tree = tree_type{
      {Value{1}, Value{4}, Value{3}, Value{2}, Value{7}, Value{0}}};
    check_tree(tree);
    auto split_node = tree.find(Key{3});
    check_tree(tree);
    auto right_tree = tree.split_right(split_node);
    check_tree(tree);
    auto& left_tree = tree;

    auto left_it = left_tree.root()->leftmost_node();
    auto left_answers = std::vector<int64_t>{{0, 1, 2}};
    auto left_current = std::begin(left_answers);
    while (left_it != nullptr) {
      assert(left_it->value == Value{*left_current});
      left_it = left_it->next_node();
      ++left_current;
    }
    check_tree(left_tree);

    auto right_it = right_tree.root()->leftmost_node();
    auto right_answers = std::vector<int64_t>{{3, 4, 7}};
    auto right_current = std::begin(right_answers);
    while (right_it != nullptr) {
      assert(right_it->value == Value{*right_current});
      right_it = right_it->next_node();
      ++right_current;
    }
    check_tree(right_tree);
  }

  void test_merge_with_empty_tree_left() {
    auto lhs_tree = tree_type{
      {Value{1}, Value{2}, Value{3}}};
    check_tree(lhs_tree);
    auto rhs_tree = tree_type{};
    check_tree(rhs_tree);
    lhs_tree.merge(rhs_tree);
    check_tree(lhs_tree);
    check_tree(rhs_tree);
    assert(!lhs_tree.empty());
    assert(lhs_tree.root() != nullptr);
    assert(lhs_tree.root()->size == 3);
    assert(lhs_tree.root()->value == Value{3});
    assert(lhs_tree.root()->right == nullptr);
    assert(lhs_tree.root()->left != nullptr);
    assert(lhs_tree.root()->left->value == Value{2});
    assert(lhs_tree.root()->left->right == nullptr);
    assert(lhs_tree.root()->left->left != nullptr);
    assert(lhs_tree.root()->left->left->value == Value{1});
    assert(rhs_tree.empty());
    assert(rhs_tree.root() == nullptr);
  }

  void test_merge_with_empty_tree_right() {
    auto lhs_tree = tree_type{};
    check_tree(lhs_tree);
    auto rhs_tree = tree_type{
      {Value{1}, Value{2}, Value{3}}};
    check_tree(rhs_tree);
    lhs_tree.merge(rhs_tree);
    check_tree(lhs_tree);
    check_tree(rhs_tree);
    assert(lhs_tree.root() != nullptr);
    assert(lhs_tree.root()->size == 3);
    assert(lhs_tree.root()->value == Value{3});
    assert(lhs_tree.root()->right == nullptr);
    assert(lhs_tree.root()->left != nullptr);
    assert(lhs_tree.root()->left->value == Value{2});
    assert(lhs_tree.root()->left->right == nullptr);
    assert(lhs_tree.root()->left->left != nullptr);
    assert(lhs_tree.root()->left->left->value == Value{1});
  }

  void test_merge_simple() {
    auto lhs_tree = tree_type{
      {Value{1}, Value{2}, Value{3}}};
    check_tree(lhs_tree);
    auto rhs_tree = tree_type{
      {Value{4}, Value{5}, Value{6}}};
    check_tree(rhs_tree);
    lhs_tree.merge(rhs_tree);
    check_tree(lhs_tree);
    check_tree(rhs_tree);
    auto it = lhs_tree.root()->leftmost_node();
    auto answers = std::vector<int64_t>{{1, 2, 3, 4, 5, 6}};
    auto current = std::begin(answers);
    while (it != nullptr) {
      assert(it->value == Value{*current});
      it = it->next_node();
      ++current;
    }
  }

  void test_erase_root() {
    auto tree = tree_type{{Value{1}}};
    check_tree(tree);
    tree.erase(tree.root());
    check_tree(tree);
    assert(tree.empty());
    assert(tree.root() == nullptr);
  }

  void test_erase_simple() {
    auto tree = tree_type{
      {Value{1}, Value{2}, Value{3}}};
    check_tree(tree);
    tree.erase(tree.root());
    check_tree(tree);
    assert(tree.root() != nullptr);
    assert(tree.root()->size == 2);
    assert(tree.root()->value == Value{2});
    assert(tree.root()->left->parent == tree.root());
    assert(tree.root()->left != nullptr);
    assert(tree.root()->left->value == Value{1});
    assert(tree.root()->left->left == nullptr);
    assert(tree.root()->left->right == nullptr);
  }

  void test_erase_batch() {
    const auto values = std::vector<int32_t>{{1, 2, -12, 15, -2, -7, 4}};
    auto tree = tree_type{};
    check_tree(tree);
    for (const auto& value : values) {
      tree.insert(Value{value});
    }
    check_tree(tree);
    for (const auto& value : values) {
      auto node = tree.find(Key{value});
      assert(node != nullptr);
      assert(node->value == Value{value});
      tree.erase(node);
      check_tree(tree);
      if (tree.root() != nullptr) {
        node = tree.find(Key{value});
        assert(node == nullptr);
      }
    }
    assert(tree.root() == nullptr);
  }

  void test_clear_tree() {
    const auto values = std::vector<int32_t>{{1, 2, -12, 15, -2, -7, 4}};
    auto tree = tree_type{};
    check_tree(tree);
    for (const auto& value : values) {
      tree.insert(Value{value});
    }
    check_tree(tree);
    tree.clear();
    check_tree(tree);
    assert(tree.empty());
    assert(tree.root() == nullptr);
  }

  void test_all() {
    test_create_and_destroy_empty_tree();
    test_insert_into_empty_tree();
    test_insert_exact_structure_easy();
    test_insert_exact_structure_hard();
    test_next_node();
    test_prev_node();
    test_copy_empty_tree();
    test_copy_tree();
    test_swap_trees();
    test_find_existing_value();
    test_find_missing_value();
    test_find_batch();
    test_order_statistic_empty_tree();
    test_order_statistic();
    test_order_statistic_out_of_range();
    test_merge_two_empty_trees();
    test_split_left_single_node_tree();
    test_split_right_single_node_tree();
    test_split_left();
    test_split_right();
    test_merge_with_empty_tree_left();
    test_merge_with_empty_tree_right();
    test_merge_simple();
    test_erase_root();
    test_erase_simple();
    test_erase_batch();
    test_clear_tree();
  }
};

class implicit_splay_tree_tester {
 public:
  using Value = Int64;
  using tree_type = implicit_splay_tree<Value>;

  void test_create_and_destroy_empty_tree() {
    auto tree = tree_type{};
    check_tree(tree);
  }

  void test_insert_into_empty_tree() {
    auto tree = tree_type{{Value{1}}};
    check_tree(tree);
    assert(tree.root() != nullptr);
    assert(tree.root()->value == Value{1});
    assert(tree.root()->size == 1);
  }

  void test_insert_exact_structure_easy() {
    auto tree = tree_type{
      {Value{1}, Value{2}, Value{4}, Value{3}}};
    check_tree(tree);

    // ((()[1, 1]())[2, 2]())[3, 4](()[4, 1]()))
    assert(tree.root() != nullptr);
    assert(tree.root()->value == Value{3});
    assert(tree.root()->size == 4);
    assert(tree.root()->parent == nullptr);
    assert(tree.root()->right != nullptr);
    assert(tree.root()->right != nullptr);

    assert(tree.root()->right->parent == tree.root());
    assert(tree.root()->right->value == Value{4});
    assert(tree.root()->right->size == 1);
    assert(tree.root()->right->left == nullptr);
    assert(tree.root()->right->right == nullptr);

    assert(tree.root()->left->parent == tree.root());
    assert(tree.root()->left->value == Value{2});
    assert(tree.root()->left->size == 2);
    assert(tree.root()->left->left != nullptr);
    assert(tree.root()->left->right == nullptr);

    assert(tree.root()->left->left->parent = tree.root()->left);
    assert(tree.root()->left->left->value == Value{1});
    assert(tree.root()->left->left->size == 1);
    assert(tree.root()->left->left->left == nullptr);
    assert(tree.root()->left->left->right == nullptr);
  }

  void test_insert_exact_structure_hard() {
    auto tree = tree_type{
      {Value{1}, Value{2}, Value{-12}, Value{15}, Value{-2}, Value{-7}, Value{4}}};
    check_tree(tree);

    assert(tree.root() != nullptr);
    assert(tree.root()->value == Value{4});
    assert(tree.root()->parent == nullptr);
    assert(tree.root()->left != nullptr);
    assert(tree.root()->right != nullptr);
    assert(tree.root()->left->parent == tree.root());
    assert(tree.root()->left->value == Value{-7});
    assert(tree.root()->left->left != nullptr);
    assert(tree.root()->left->right != nullptr);
    assert(tree.root()->right->parent == tree.root());
    assert(tree.root()->right->value == Value{15});
    assert(tree.root()->right->left == nullptr);
    assert(tree.root()->right->right == nullptr);
    assert(tree.root()->left->left->parent == tree.root()->left);
    assert(tree.root()->left->left->value == Value{-12});
    assert(tree.root()->left->left->left == nullptr);
    assert(tree.root()->left->left->right == nullptr);
    assert(tree.root()->left->right->parent == tree.root()->left);
    assert(tree.root()->left->right->value == Value{-2});
    assert(tree.root()->left->right->left == nullptr);
    assert(tree.root()->left->right->right != nullptr);
    assert(tree.root()->left->right->right->parent == tree.root()->left->right);
    assert(tree.root()->left->right->right->value == Value{2});
    assert(tree.root()->left->right->right->right == nullptr);
    assert(tree.root()->left->right->right->left != nullptr);
    assert(tree.root()->left->right->right->left->parent == tree.root()->left->right->right);
    assert(tree.root()->left->right->right->left->value == Value{1});
    assert(tree.root()->left->right->right->left->left == nullptr);
    assert(tree.root()->left->right->right->left->right == nullptr);
  }

  void test_next_node() {
    auto values = std::vector<int64_t>{{1, 2, -12, 15, -2, -7, 4}};
    auto tree = tree_type{};
    check_tree(tree);
    for (const auto& value : values) {
      tree.insert(Value{value});
    }
    check_tree(tree);
    assert(tree.root() != nullptr);
    assert(tree.root()->size == values.size());
    auto it = tree.root()->leftmost_node();
    std::sort(std::begin(values), std::end(values));
    auto current = std::begin(values);
    while (it != nullptr) {
      assert(it->value == Value{*current});
      ++current;
      it = it->next_node();
    }
  }

  void test_prev_node() {
    auto values = std::vector<int64_t>{{1, 2, -12, 15, -2, -7, 4}};
    auto tree = tree_type{};
    check_tree(tree);
    for (const auto& value : values) {
      tree.insert(Value{value});
    }
    check_tree(tree);
    assert(tree.root() != nullptr);
    assert(tree.root()->size == values.size());
    auto it = tree.root()->rightmost_node();
    std::sort(std::begin(values), std::end(values));
    auto current = std::rbegin(values);
    while (it != nullptr) {
      assert(it->value == Value{*current});
      ++current;
      it = it->prev_node();
    }
    check_tree(tree);
  }

  void test_copy_empty_tree() {
    auto tree = tree_type{};
    check_tree(tree);
    auto copied = tree;
    assert(copied.empty());
    assert(copied.size() == 0);
    check_tree(tree);
  }

  void test_copy_tree() {
    auto values = std::vector<int64_t>{{1, 2, -12, 15, -2, -7, 4}};
    auto tree = tree_type{};
    check_tree(tree);
    for (const auto& value : values) {
      tree.insert(Value{value});
    }
    check_tree(tree);
    auto copied = tree;
    check_tree(copied);
    assert(copied.root() != nullptr);
    assert(copied.root()->size == values.size());
    auto it = copied.root()->leftmost_node();
    std::sort(std::begin(values), std::end(values));
    auto current = std::begin(values);
    while (it != nullptr) {
      assert(it->value == Value{*current});
      ++current;
      it = it->next_node();
    }
    check_tree(copied);
  }

  void test_swap_trees() {
    auto lhs_values = std::vector<int64_t>{{1, 2, -12, 15, -2, -7, 4}};
    auto lhs_tree = tree_type{};
    check_tree(lhs_tree);
    for (const auto& value : lhs_values) {
      lhs_tree.insert(Value{value});
    }
    check_tree(lhs_tree);
    assert(lhs_tree.size() == lhs_values.size());

    auto rhs_values = std::vector<int64_t>{{1, 2, 4, 3}};
    auto rhs_tree = tree_type{};
    check_tree(rhs_tree);
    for (const auto& value : rhs_values) {
      rhs_tree.insert(Value{value});
    }
    check_tree(rhs_tree);
    assert(rhs_tree.size() == rhs_values.size());

    lhs_tree.swap(rhs_tree);

    std::sort(std::begin(rhs_values), std::end(rhs_values));
    auto rhs_current = std::begin(rhs_values);
    auto lhs_it = lhs_tree.root()->leftmost_node();
    while (lhs_it != nullptr) {
      assert(lhs_it->value == Value{*rhs_current});
      ++rhs_current;
      lhs_it = lhs_it->next_node();
    }
    check_tree(lhs_tree);

    std::sort(std::begin(lhs_values), std::end(lhs_values));
    auto lhs_current = std::begin(lhs_values);
    auto rhs_it = rhs_tree.root()->leftmost_node();
    while (rhs_it != nullptr) {
      assert(rhs_it->value == Value{*lhs_current});
      ++lhs_current;
      rhs_it = rhs_it->next_node();
    }
    check_tree(rhs_tree);
  }

  void test_order_statistic_empty_tree() {
    auto tree = tree_type{};
    check_tree(tree);
    assert(tree.empty());
    for (auto idx = size_t{0}; idx < 5; ++idx) {
      auto node = tree.order_statistic(idx);
      check_tree(tree);
      assert(node == nullptr);
    }
  }

  void test_order_statistic() {
    auto values = std::vector<int64_t>{{1, 2, -12, 15, -2, -7, 4}};
    auto tree = tree_type{};
    check_tree(tree);
    for (const auto& value : values) {
      tree.insert(Value{value});
    }
    check_tree(tree);
    assert(tree.size() == values.size());
    std::sort(std::begin(values), std::end(values));
    for (auto idx = size_t{0}; idx < values.size(); ++idx) {
      auto node = tree.order_statistic(idx);
      check_tree(tree);
      assert(node != nullptr);
      assert(node->value == Value{values[idx]});
    }
  }

  void test_order_statistic_out_of_range() {
    auto values = std::vector<int64_t>{{1, 2, -12, 15, -2, -7, 4}};
    auto tree = tree_type{};
    check_tree(tree);
    for (const auto& value : values) {
      tree.insert(Value{value});
    }
    check_tree(tree);
    assert(tree.size() == values.size());
    for (auto idx = values.size(); idx < 2 * values.size(); ++idx) {
      auto node = tree.order_statistic(idx);
      check_tree(tree);
      assert(node == nullptr);
    }
  }

  void test_merge_two_empty_trees() {
    auto lhs_tree = tree_type{};
    check_tree(lhs_tree);
    auto rhs_tree = tree_type{};
    check_tree(rhs_tree);
    lhs_tree.merge(rhs_tree);
    check_tree(lhs_tree);
    check_tree(rhs_tree);
    assert(rhs_tree.empty());
    assert(rhs_tree.root() == nullptr);
  }

  void test_split_left_single_node_tree() {
    {
      auto tree = tree_type{{Value{1}}};
      check_tree(tree);
      auto split_node = tree.order_statistic(0);
      check_tree(tree);
      auto right_tree = tree.split_left(split_node);
      check_tree(tree);
      check_tree(right_tree);
      auto& left_tree = tree;
      assert(right_tree.root() == nullptr);
      assert(left_tree.root() != nullptr);
      assert(left_tree.root()->value == Value{1});
      assert(left_tree.root()->size == 1);
    }
    {
      auto tree = tree_type{{Value{1}}};
      check_tree(tree);
      auto split_node = tree.order_statistic(1);
      check_tree(tree);
      auto right_tree = tree.split_left(split_node);
      check_tree(tree);
      check_tree(right_tree);
      auto& left_tree = tree;
      assert(right_tree.root() == nullptr);
      assert(left_tree.root() != nullptr);
      assert(left_tree.root()->value == Value{1});
      assert(left_tree.root()->size == 1);
    }
  }

  void test_split_right_single_node_tree() {
    {
      auto tree = tree_type{{Value{1}}};
      check_tree(tree);
      auto split_node = tree.order_statistic(0);
      check_tree(tree);
      auto right_tree = tree.split_right(split_node);
      check_tree(tree);
      check_tree(right_tree);
      auto& left_tree = tree;
      assert(left_tree.root() == nullptr);
      assert(right_tree.root() != nullptr);
      assert(right_tree.root()->value == Value{1});
      assert(right_tree.root()->size == 1);
    }
    {
      auto tree = tree_type{{Value{1}}};
      check_tree(tree);
      auto split_node = tree.order_statistic(1);
      check_tree(tree);
      auto right_tree = tree.split_right(split_node);
      check_tree(tree);
      check_tree(right_tree);
      auto& left_tree = tree;
      assert(right_tree.root() == nullptr);
      assert(left_tree.root() != nullptr);
      assert(left_tree.root()->value == Value{1});
      assert(left_tree.root()->size == 1);
    }
  }

  void test_split_left() {
    auto tree = tree_type{
      {Value{1}, Value{4}, Value{3}, Value{2}, Value{7}, Value{0}}};
    check_tree(tree);
    auto split_node = tree.order_statistic(3);
    check_tree(tree);
    auto right_tree = tree.split_left(split_node);
    check_tree(tree);
    auto& left_tree = tree;

    auto left_it = left_tree.root()->leftmost_node();
    auto left_answers = std::vector<int64_t>{{0, 1, 2, 3}};
    auto left_current = std::begin(left_answers);
    while (left_it != nullptr) {
      assert(left_it->value == Value{*left_current});
      left_it = left_it->next_node();
      ++left_current;
    }
    check_tree(left_tree);

    auto right_it = right_tree.root()->leftmost_node();
    auto right_answers = std::vector<int64_t>{{4, 7}};
    auto right_current = std::begin(right_answers);
    while (right_it != nullptr) {
      assert(right_it->value == Value{*right_current});
      right_it = right_it->next_node();
      ++right_current;
    }
    check_tree(right_tree);
  }

  void test_split_right() {
    auto tree = tree_type{
      {Value{1}, Value{4}, Value{3}, Value{2}, Value{7}, Value{0}}};
    check_tree(tree);
    auto split_node = tree.order_statistic(3);
    check_tree(tree);
    auto right_tree = tree.split_right(split_node);
    check_tree(tree);
    auto& left_tree = tree;

    auto left_it = left_tree.root()->leftmost_node();
    auto left_answers = std::vector<int64_t>{{0, 1, 2}};
    auto left_current = std::begin(left_answers);
    while (left_it != nullptr) {
      assert(left_it->value == Value{*left_current});
      left_it = left_it->next_node();
      ++left_current;
    }
    check_tree(left_tree);

    auto right_it = right_tree.root()->leftmost_node();
    auto right_answers = std::vector<int64_t>{{3, 4, 7}};
    auto right_current = std::begin(right_answers);
    while (right_it != nullptr) {
      assert(right_it->value == Value{*right_current});
      right_it = right_it->next_node();
      ++right_current;
    }
    check_tree(right_tree);
  }

  void test_merge_with_empty_tree_left() {
    auto lhs_tree = tree_type{
      {Value{1}, Value{2}, Value{3}}};
    check_tree(lhs_tree);
    auto rhs_tree = tree_type{};
    check_tree(rhs_tree);
    lhs_tree.merge(rhs_tree);
    check_tree(lhs_tree);
    check_tree(rhs_tree);
    assert(!lhs_tree.empty());
    assert(lhs_tree.root() != nullptr);
    assert(lhs_tree.root()->size == 3);
    assert(lhs_tree.root()->value == Value{3});
    assert(lhs_tree.root()->right == nullptr);
    assert(lhs_tree.root()->left != nullptr);
    assert(lhs_tree.root()->left->value == Value{2});
    assert(lhs_tree.root()->left->right == nullptr);
    assert(lhs_tree.root()->left->left != nullptr);
    assert(lhs_tree.root()->left->left->value == Value{1});
    assert(rhs_tree.empty());
    assert(rhs_tree.root() == nullptr);
  }

  void test_merge_with_empty_tree_right() {
    auto lhs_tree = tree_type{};
    check_tree(lhs_tree);
    auto rhs_tree = tree_type{
      {Value{1}, Value{2}, Value{3}}};
    check_tree(rhs_tree);
    lhs_tree.merge(rhs_tree);
    check_tree(lhs_tree);
    check_tree(rhs_tree);
    assert(lhs_tree.root() != nullptr);
    assert(lhs_tree.root()->size == 3);
    assert(lhs_tree.root()->value == Value{3});
    assert(lhs_tree.root()->right == nullptr);
    assert(lhs_tree.root()->left != nullptr);
    assert(lhs_tree.root()->left->value == Value{2});
    assert(lhs_tree.root()->left->right == nullptr);
    assert(lhs_tree.root()->left->left != nullptr);
    assert(lhs_tree.root()->left->left->value == Value{1});
  }

  void test_merge_simple() {
    auto lhs_tree = tree_type{
      {Value{1}, Value{2}, Value{3}}};
    check_tree(lhs_tree);
    auto rhs_tree = tree_type{
      {Value{4}, Value{5}, Value{6}}};
    check_tree(rhs_tree);
    lhs_tree.merge(rhs_tree);
    check_tree(lhs_tree);
    check_tree(rhs_tree);
    auto it = lhs_tree.root()->leftmost_node();
    auto answers = std::vector<int64_t>{{1, 2, 3, 4, 5, 6}};
    auto current = std::begin(answers);
    while (it != nullptr) {
      assert(it->value == Value{*current});
      it = it->next_node();
      ++current;
    }
  }

  void test_erase_root() {
    auto tree = tree_type{{Value{1}}};
    check_tree(tree);
    tree.erase(tree.root());
    check_tree(tree);
    assert(tree.empty());
    assert(tree.root() == nullptr);
  }

  void test_erase_simple() {
    auto tree = tree_type{
      {Value{1}, Value{2}, Value{3}}};
    check_tree(tree);
    tree.erase(tree.root());
    check_tree(tree);
    assert(tree.root() != nullptr);
    assert(tree.root()->size == 2);
    assert(tree.root()->value == Value{2});
    assert(tree.root()->left->parent == tree.root());
    assert(tree.root()->left != nullptr);
    assert(tree.root()->left->value == Value{1});
    assert(tree.root()->left->left == nullptr);
    assert(tree.root()->left->right == nullptr);
  }

  void test_erase_batch() {
    auto values = std::vector<int32_t>{{1, 2, -12, 15, -2, -7, 4}};
    auto tree = tree_type{};
    check_tree(tree);
    for (const auto& value : values) {
      tree.insert(Value{value});
    }
    check_tree(tree);
    std::sort(std::begin(values), std::end(values));
    for (const auto& value : values) {
      auto node = tree.order_statistic(0);
      assert(node != nullptr);
      assert(node->value == Value{value});
      tree.erase(node);
      check_tree(tree);
    }
    assert(tree.root() == nullptr);
  }

  void test_clear_tree() {
    const auto values = std::vector<int32_t>{{1, 2, -12, 15, -2, -7, 4}};
    auto tree = tree_type{};
    check_tree(tree);
    for (const auto& value : values) {
      tree.insert(Value{value});
    }
    check_tree(tree);
    tree.clear();
    check_tree(tree);
    assert(tree.empty());
    assert(tree.root() == nullptr);
  }

  void test_all() {
    test_create_and_destroy_empty_tree();
    test_insert_into_empty_tree();
    test_insert_exact_structure_easy();
    test_insert_exact_structure_hard();
    test_next_node();
    test_prev_node();
    test_copy_empty_tree();
    test_copy_tree();
    test_swap_trees();
    test_order_statistic_empty_tree();
    test_order_statistic();
    test_order_statistic_out_of_range();
    test_merge_two_empty_trees();
    test_split_left_single_node_tree();
    test_split_right_single_node_tree();
    test_split_left();
    test_split_right();
    test_merge_with_empty_tree_left();
    test_merge_with_empty_tree_right();
    test_merge_simple();
    test_erase_root();
    test_erase_simple();
    test_erase_batch();
    test_clear_tree();
  }
};

}  // namespace test
}  // namespace splay


int main() {
  auto splay_tester = splay::test::splay_tree_tester{};
  splay_tester.test_all();
  auto implicit_splay_tester = splay::test::splay_tree_tester{};
  implicit_splay_tester.test_all();
  std::cout << "All tests are passed!\n";
  return 0;
}