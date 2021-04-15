#include <algorithm>
#include <random>

#include "splay_tree.h"

namespace splay {
namespace testing {

template <typename Value>
void debug_print(const splay_tree_node<Value>* node, const std::string& message) {
  std::cout << message << '\n';
  print_subtree(std::cout, node);
  std::cout << '\n';
}

template <typename Value>
struct node_count_check_result {
  bool ok;
  const splay_tree_node<Value>* broken_node;
};

template <typename Value>
node_count_check_result<Value> check_node_count(const splay_tree_node<Value>* node) {
  if (node == nullptr) {
    return node_count_check_result<Value>{true, nullptr};
  }
  const auto left_node_count = node->left != nullptr ? node->left->node_count : uint64_t{0};
  const auto right_node_count = node->right != nullptr ? node->right->node_count : uint64_t{0};
  if (node->node_count != left_node_count + right_node_count + 1) {
    return node_count_check_result<Value>{false, node};
  }
  const auto left_result = check_node_count(node->left);
  const auto right_result = check_node_count(node->right);
  if (!left_result.ok) {
    return left_result;
  }
  if (!right_result.ok) {
    return right_result;
  }
  return node_count_check_result<Value>{true, nullptr};
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
  const splay_tree_node<Value>* broken_node;
};

template <typename Value>
structure_check_result<Value> check_structure(const splay_tree_node<Value>* node) {
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
  const splay_tree_node<Value>* broken_node;
  const splay_tree_node<Value>* min_node;
  const splay_tree_node<Value>* max_node;
};

template <typename Key, typename Value, typename KeyExtractor, typename KeyComparator>
ordering_check_result<Value> check_ordering(
    const splay_tree_node<Value>* node,
    const KeyExtractor& extractor,
    const KeyComparator& comparator) {
  if (node == nullptr) {
    return ordering_check_result<Value>{ordering_type::kBalanced, nullptr, nullptr, nullptr};
  }
  auto check_result = ordering_check_result<Value>{ordering_type::kBalanced, nullptr, node, node};
  // checks
  const auto left_check_result = check_ordering<Key, Value, KeyExtractor, KeyComparator>(
    node->left, extractor, comparator);
  if (left_check_result.type != ordering_type::kBalanced) {
    check_result.type = ordering_type::kUnbalanced;
    check_result.broken_node = left_check_result.broken_node;
  }
  const auto right_check_result = check_ordering<Key, Value, KeyExtractor, KeyComparator>(
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

template <typename Key, typename Value, typename KeyExtractor, typename KeyComparator>
void check_subtree(const splay_tree_node<Value>* node) {
  auto structure_check = check_structure(node);
  if (structure_check.type != structure_type::kOk) {
    std::cout << "structure erorr of type " << static_cast<uint64_t>(structure_check.type) << '\n';
    debug_print(structure_check.broken_node, "Incorrect structure for the node");
  }
  auto count_check = check_node_count(node);
  if (!count_check.ok) {
    debug_print(count_check.broken_node, "Incorrect node counts\n");
  }
  auto order_check = check_ordering<Key, Value, KeyExtractor, KeyComparator>(
    node, KeyExtractor{}, KeyComparator{});
  if (order_check.type != ordering_type::kBalanced) {
    debug_print(order_check.broken_node, "Incorrect node ordering\n");
  }
}

class Int64 {
 public:
  int64_t value;
};

bool operator == (const Int64& lhs, const Int64& rhs) {
  return lhs.value == rhs.value;
}

bool operator != (const Int64& lhs, const Int64& rhs) {
  return lhs.value != rhs.value;
}

std::ostream& operator << (std::ostream& out, const Int64& value) {
  out << value;
  return out;
}

class Int32 {
 public:
  Int32(int32_t value)
    : value{value} {
  }

 public:
  int32_t value;
};

class Int32Extractor {
 public:
  Int32 operator ()(const Int64& value) const {
    return Int32{static_cast<int32_t>(value.value)};
  }
};

class Int32Comparator {
 public:
  bool operator () (const Int32& lhs, const Int32& rhs) const {
    return lhs.value < rhs.value;
  }
};

void test_create_and_destroy_empty_tree() {
  using Value = Int64;
  using Key = Int32;
  using KeyExtractor = Int32Extractor;
  using KeyComparator = Int32Comparator;
  auto tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{};
}

void test_insert_into_empty_tree() {
  using Value = Int64;
  using Key = Int32;
  using KeyExtractor = Int32Extractor;
  using KeyComparator = Int32Comparator;
  auto tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{{Value{1}}};
  assert(tree.root != nullptr);
  assert(tree.root->value == Value{1});
  assert(tree.root->node_count == 1);
}

void test_insert_exact_structure() {
  using Value = Int64;
  using Key = Int32;
  using KeyExtractor = Int32Extractor;
  using KeyComparator = Int32Comparator;
  auto tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{
    {Value{1}, Value{2}, Value{4}, Value{3}}};

  // ((()[1, 1]())[2, 2]())[3, 4](()[4, 1]()))
  assert(tree.root != nullptr);
  assert(tree.root->value == Value{3});
  assert(tree.root->node_count == 4);
  assert(tree.root->parent == nullptr);
  assert(tree.root->right != nullptr);
  assert(tree.root->right != nullptr);

  assert(tree.root->right->parent == tree.root);
  assert(tree.root->right->value == Value{4});
  assert(tree.root->right->node_count == 1);
  assert(tree.root->right->left == nullptr);
  assert(tree.root->right->right == nullptr);

  assert(tree.root->left->parent == tree.root);
  assert(tree.root->left->value == Value{2});
  assert(tree.root->left->node_count == 2);
  assert(tree.root->left->left != nullptr);
  assert(tree.root->left->right == nullptr);

  assert(tree.root->left->left->parent = tree.root->left);
  assert(tree.root->left->left->value == Value{1});
  assert(tree.root->left->left->node_count == 1);
  assert(tree.root->left->left->left == nullptr);
  assert(tree.root->left->left->right == nullptr);
}

void test_insert_batch() {
  using Value = Int64;
  using Key = Int32;
  using KeyExtractor = Int32Extractor;
  using KeyComparator = Int32Comparator;
  auto values = {Value{1}, Value{2}, Value{-12}, Value{15}, Value{-2}, Value{-7}, Value{4}};
  auto tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{values};
  assert(tree.root != nullptr);
  assert(tree.root->node_count == values.size());
}

void test_next_node() {
  using Value = Int64;
  using Key = Int32;
  using KeyExtractor = Int32Extractor;
  using KeyComparator = Int32Comparator;
  auto values = std::vector<int64_t>{{1, 2, -12, 15, -2, -7, 4}};
  auto tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{};
  for (const auto& value : values) {
    tree.insert(Value{value});
  }
  assert(tree.root != nullptr);
  assert(tree.root->node_count == values.size());
  auto it = tree.root->leftmost_node();
  std::sort(std::begin(values), std::end(values));
  auto current = std::begin(values);
  while (it != nullptr) {
    assert(it->value == Value{*current});
    ++current;
    it = it->next_node();
  }
}


void test_prev_node() {
  using Value = Int64;
  using Key = Int32;
  using KeyExtractor = Int32Extractor;
  using KeyComparator = Int32Comparator;
  auto values = std::vector<int64_t>{{1, 2, -12, 15, -2, -7, 4}};
  auto tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{};
  for (const auto& value : values) {
    tree.insert(Value{value});
  }
  assert(tree.root != nullptr);
  assert(tree.root->node_count == values.size());
  auto it = tree.root->rightmost_node();
  std::sort(std::begin(values), std::end(values));
  auto current = std::rbegin(values);
  while (it != nullptr) {
    assert(it->value == Value{*current});
    ++current;
    it = it->prev_node();
  }
}

void test_find_existing_value() {
  using Value = Int64;
  using Key = Int32;
  using KeyExtractor = Int32Extractor;
  using KeyComparator = Int32Comparator;
  auto tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{{Value{1}}};
  auto node = tree.find(Key{1});
  assert(node != nullptr);
  assert(node->value == Value{1});
}

void test_find_missing_value() {
  using Value = Int64;
  using Key = Int32;
  using KeyExtractor = Int32Extractor;
  using KeyComparator = Int32Comparator;
  auto tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{{Value{1}}};
  auto node = tree.find(Key{2});
  assert(node == nullptr);
}

void test_find_candidate_neighbour_values() {
  using Value = Int64;
  using Key = Int32;
  using KeyExtractor = Int32Extractor;
  using KeyComparator = Int32Comparator;
  auto values = std::vector<int64_t>{{60, 20, 30, 50, 10, 40}};
  auto tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{};
  for (const auto& value : values) {
    tree.insert(Value{value});
  }
  std::sort(std::begin(values), std::end(values));
  auto missing_values = std::vector<int64_t>{{5, 15, 25, 35, 45, 55, 65}};
  for (const auto& value : missing_values) {
    auto node = find_candidate<Key, Value, KeyExtractor, KeyComparator>(
      tree.root, value, tree.extractor, tree.comparator);
    assert(node != nullptr);
    assert(node->value != Value{value});
    auto top = std::upper_bound(
      std::begin(values), std::end(values), value, std::less<int64_t>{});
    if (top != std::end(values)) {
      assert(value == *top - 5);
    }
    auto bottom = std::upper_bound(
      std::rbegin(values), std::rend(values), value, std::greater<int64_t>{});
    if (bottom != std::rend(values)) {
      assert(value == *bottom + 5);
    }
  }
}

void test_find_batch() {
  using Value = Int64;
  using Key = Int32;
  using KeyExtractor = Int32Extractor;
  using KeyComparator = Int32Comparator;
  const auto present_values = std::vector<int64_t>{{1, 2, 3, -1, 5, -2}};
  const auto missing_values = std::vector<int64_t>{{100, 200, 300, -100, 500, -200}};
  auto tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{};
  for (const auto& value : present_values) {
    tree.insert(Value{value});
  }
  for (const auto& value : present_values) {
    auto node = tree.find(value);
    assert(node != nullptr);
    assert(node->value == Value{value});
  }
  for (const auto& value : missing_values) {
    auto node = tree.find(value);
    assert(node == nullptr);
  }
}

void test_merge_two_empty_trees() {
  using Value = Int64;
  using Key = Int32;
  using KeyExtractor = Int32Extractor;
  using KeyComparator = Int32Comparator;
  auto tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{};
  auto other_tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{};
  tree.merge(other_tree);
  assert(tree.root == nullptr);
}

void test_merge_with_empty_tree_left() {
  using Value = Int64;
  using Key = Int32;
  using KeyExtractor = Int32Extractor;
  using KeyComparator = Int32Comparator;
  auto tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{{Value{1}, Value{2}, Value{3}}};
  auto other_tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{};
  tree.merge(other_tree);
  assert(tree.root != nullptr);
  assert(tree.root->node_count == 3);
  assert(tree.root->value == Value{3});
  assert(tree.root->right == nullptr);
  assert(tree.root->left != nullptr);
  assert(tree.root->left->value == Value{2});
  assert(tree.root->left->right == nullptr);
  assert(tree.root->left->left != nullptr);
  assert(tree.root->left->left->value == Value{1});
}

void test_merge_with_empty_tree_right() {
  using Value = Int64;
  using Key = Int32;
  using KeyExtractor = Int32Extractor;
  using KeyComparator = Int32Comparator;
  auto tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{};
  auto other_tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{
    {Value{1}, Value{2}, Value{3}}};
  tree.merge(other_tree);
  assert(tree.root != nullptr);
  assert(tree.root->node_count == 3);
  assert(tree.root->value == Value{3});
  assert(tree.root->right == nullptr);
  assert(tree.root->left != nullptr);
  assert(tree.root->left->value == Value{2});
  assert(tree.root->left->right == nullptr);
  assert(tree.root->left->left != nullptr);
  assert(tree.root->left->left->value == Value{1});
}

void test_merge_simple() {
  using Value = Int64;
  using Key = Int32;
  using KeyExtractor = Int32Extractor;
  using KeyComparator = Int32Comparator;
  auto tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{
    {Value{1}, Value{2}, Value{3}}};
  auto other_tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{
    {Value{4}, Value{5}, Value{6}}};
  tree.merge(other_tree);
  auto it = tree.root->leftmost_node();
  auto answers = std::vector<int64_t>{{1, 2, 3, 4, 5, 6}};
  auto current = std::begin(answers);
  while (it != nullptr) {
    assert(it->value == Value{*current});
    it = it->next_node();
    ++current;
  }
}

void test_split_single_node_tree() {
  using Value = Int64;
  using Key = Int32;
  using KeyExtractor = Int32Extractor;
  using KeyComparator = Int32Comparator;
  {
    auto tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{{Value{1}}};
    auto trees = tree.split(Value{0});
    const auto& left_tree = trees.first;
    const auto& right_tree = trees.second;
    assert(left_tree.root == nullptr);
    assert(right_tree.root != nullptr);
    assert(right_tree.root->value == Value{1});
    assert(right_tree.root->node_count == 1);
  }
  {
    auto tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{{Value{1}}};
    auto trees = tree.split(Value{1});
    const auto& left_tree = trees.first;
    const auto& right_tree = trees.second;
    assert(left_tree.root == nullptr);
    assert(right_tree.root != nullptr);
    assert(right_tree.root->value == Value{1});
    assert(right_tree.root->node_count == 1);
  }
  {
    auto tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{{Value{1}}};
    auto trees = tree.split(Value{2});
    const auto& left_tree = trees.first;
    const auto& right_tree = trees.second;
    assert(right_tree.root == nullptr);
    assert(left_tree.root != nullptr);
    assert(left_tree.root->value == Value{1});
    assert(left_tree.root->node_count == 1);
  }
}

void test_split_simple() {
  using Value = Int64;
  using Key = Int32;
  using KeyExtractor = Int32Extractor;
  using KeyComparator = Int32Comparator;
  auto tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{
    {Value{1}, Value{4}, Value{3}, Value{2}, Value{7}, Value{0}}};
  check_subtree<Key, Value, KeyExtractor, KeyComparator>(tree.root);
  auto trees = tree.split(Value{3});
  check_subtree<Key, Value, KeyExtractor, KeyComparator>(tree.root);
  const auto& left_tree = trees.first;
  const auto& right_tree = trees.second;

  auto left_it = left_tree.root->leftmost_node();
  auto left_answers = std::vector<int64_t>{{0, 1, 2}};
  auto left_current = std::begin(left_answers);
  while (left_it != nullptr) {
    assert(left_it->value == Value{*left_current});
    left_it = left_it->next_node();
    ++left_current;
  }

  auto right_it = right_tree.root->leftmost_node();
  auto right_answers = std::vector<int64_t>{{3, 4, 7}};
  auto right_current = std::begin(right_answers);
  while (right_it != nullptr) {
    assert(right_it->value == Value{*right_current});
    right_it = right_it->next_node();
    ++right_current;
  }
}

void test_erase_root() {
  using Value = Int64;
  using Key = Int32;
  using KeyExtractor = Int32Extractor;
  using KeyComparator = Int32Comparator;
  auto tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{{Value{1}}};
  tree.erase(tree.root);
  assert(tree.root == nullptr);
}

void test_erase_simple() {
  using Value = Int64;
  using Key = Int32;
  using KeyExtractor = Int32Extractor;
  using KeyComparator = Int32Comparator;
  auto tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{
    {Value{1}, Value{2}, Value{3}}};
  tree.erase(tree.root);
  assert(tree.root != nullptr);
  assert(tree.root->node_count == 2);
  assert(tree.root->value == Value{2});
  assert(tree.root->left != nullptr);
  assert(tree.root->left->value == Value{1});
}

void test_erase_batch() {
  using Value = Int64;
  using Key = Int32;
  using KeyExtractor = Int32Extractor;
  using KeyComparator = Int32Comparator;
  const auto values = std::vector<int64_t>{{1, 2, -12, 15, -2, -7, 4}};
  auto tree = splay_tree<Key, Value, KeyExtractor, KeyComparator>{};
  for (const auto& value : values) {
    tree.insert(Value{value});
  }
  for (const auto& value : values) {
    auto node = tree.find(value);
    assert(node != nullptr);
    assert(node->value == Value{value});
    tree.erase(node);
    if (tree.root != nullptr) {
      node = tree.find(value);
      assert(node == nullptr);
    }
  }
  assert(tree.root == nullptr);
}

}  // namespace testing
}  // namespace splay


int main() {
  using namespace splay::testing;
  test_create_and_destroy_empty_tree();
  test_insert_into_empty_tree();
  test_insert_exact_structure();
  test_insert_batch();
  test_next_node();
  test_prev_node();
  test_find_existing_value();
  test_find_missing_value();
  test_find_candidate_neighbour_values();
  test_find_batch();
  test_merge_two_empty_trees();
  test_merge_with_empty_tree_left();
  test_merge_with_empty_tree_right();
  test_merge_simple();
  test_split_single_node_tree();
  test_split_simple();
  test_erase_root();
  test_erase_simple();
  test_erase_batch();
  std::cout << "All tests are passed!\n";
  return 0;
};