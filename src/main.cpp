#include <cassert>
#include <iostream>
#include <vector>

#include "splay_tree.h"

template <typename T>
class fast_range_counter {
 private:

  template <typename Key>
  struct default_key_extractor {
    const Key& operator () (const Key& t) const {
      return t;
    }
  };

  template <typename Key>
  struct default_key_comparator {
    bool operator ()(const Key& lhs, const Key& rhs) const {
      return lhs < rhs;
    }
  };

 public:
  using tree_type = splay::splay_tree<T, T, default_key_comparator<T>, default_key_extractor<T>>;

  fast_range_counter()
    : tree{}
  {}

  void add(const T& number) {
    if (debug) {
      std::cout << "==========================" << '\n';
      std::cout << "+ " << number << '\n';
    }
    tree.insert(number);
    if (debug) {
      std::cout << tree << '\n';
    }
  }

  void remove(const T& number) {
    if (debug) {
      std::cout << "==========================" << '\n';
      std::cout << "- " << number << '\n';
    }
    auto node = tree.find(number);
    if (node != nullptr) {
      tree.erase(node);
    }
    if (debug) {
      std::cout << tree << '\n';
    }
  }

  bool contains(const T& number) {
    if (debug) {
      std::cout << "==========================" << '\n';
      std::cout << "? " << number << '\n';
    }

    auto node = tree.find(number);

    if (debug) {
      std::cout << tree << '\n';
    }

    return (node != nullptr);
  }

  size_t count(const T& low, const T& high) {
    assert(low <= high);
    if (debug) {

      std::cout << "==========================" << '\n';
      std::cout << "s " << low << ' ' << high << '\n';
    }

    auto low_node = tree.lower_bound(low);
    auto middle_right_tree = tree.split_right(low_node);
    auto& left_tree = tree;

    if (debug) {
      std::cout << "left_tree = " << left_tree << '\n';
      std::cout << "middle+rirgt_tree = " << middle_right_tree << '\n';
    }

    auto high_node = middle_right_tree.upper_bound(high);
    auto right_tree = middle_right_tree.split_right(high_node);
    auto& middle_tree = middle_right_tree;

    if (debug) {
      std::cout << "left_tree = " << left_tree << '\n';
      std::cout << "middle_tree = " << middle_tree << '\n';
      std::cout << "right_tree = " << right_tree << '\n';
    }

    const auto total = (!middle_tree.empty() ? middle_tree.size() : size_t{0});

    if (debug) {
      std::cout << "total = " << total << '\n';
    }

    middle_tree.merge(right_tree);

    left_tree.merge(middle_tree);

    if (debug) {
      std::cout << "tree = " << tree << '\n';
    }

    return total;
  }

  const tree_type& get_tree() const {
    return tree;
  }

 private:
  tree_type tree;
  const bool debug = false;
};

void run() {
  std::ostream& out = std::cout;
  std::istream& in = std::cin;
  out << "Demonstration of splay tree functionality\n"
      << "Available actions:\n"
      << "\tadd NUMBER      - insert NUMBER into the tree\n"
      << "\tremove NUMBER   - erase NUMBER from the tree\n"
      << "\tcontains NUMBER - check if NUMBER is present in the tree\n"
      << "\tcount FROM TO   - count how many elements of the tree are in the range [FROM, TO]\n"
      << "\tfinish          - stop and exit\n\n";

  using Value = int64_t;

  auto counter = fast_range_counter<Value>{};

  out << "Initial tree: " << counter.get_tree() << '\n';
  while (true)
  {
    out << "Enter action: \n";
    std::string action;
    in >> action;
    if (action == "add") {
      auto value = Value{};
      in >> value;
      counter.add(value);
      out << "Tree: " << counter.get_tree() << '\n';
    } else if (action == "remove") {
      auto value = Value{};
      in >> value;
      counter.remove(value);
      out << "Tree: " << counter.get_tree() << '\n';
    } else if (action == "contains") {
      auto value = Value{};
      in >> value;
      if (counter.contains(value)) {
        out << "True\n";
      } else {
        out << "False\n";
      }
      out << "Tree: " << counter.get_tree() << '\n';
    } else if (action == "count") {
      auto begin = Value{};
      in >> begin;
      auto end = Value{};
      in >> end;
      if (begin > end) {
        out << "Illegal range. Range start must be less or equal to the range end\n";
      } else {
        out << counter.count(begin, end) << '\n';
      }
    } else if (action == "finish") {
      break;
    } else {
      out << "Unknown action\n";
    }
  }
}

int main() {
  run();
  return 0;
}