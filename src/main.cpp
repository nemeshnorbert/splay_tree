#include <cassert>
#include <iostream>
#include <vector>

#include "splay_tree.h"

template <typename Key>
struct identity_key_extractor {
  const Key& operator () (const Key& t) const {
    return t;
  }
};

void run() {
  std::ostream& out = std::cout;
  std::istream& in = std::cin;
  out << "Demonstration of splay tree functionality\n"
      << "Available actions: insert NUMBER, delete NUMBER, find NUMBER, finish.\n";

  using Value = int;
  using Key = int;
  using KeyExtractor = identity_key_extractor<Key>;
  using KeyComparator = std::less<Key>;
  auto tree = splay::splay_tree<Key, Value, KeyExtractor, KeyComparator>{};

  out << "Initial tree: " << tree << '\n';
  while (true)
  {
    out << "Enter action: \n";
    std::string action;
    in >> action;
    if (action == "insert") {
      auto value = Value{};
      in >> value;
      tree.insert(value);
      out << "Tree: " << tree << '\n';
    } else if (action == "delete") {
      auto key = Key{};
      in >> key;
      auto node = tree.find(key);
      if (node != nullptr) {
        out << tree.erase(node) << '\n';
      }
      out << "Tree: " << tree << '\n';
    } else if (action == "find") {
      auto key = Key{};
      in >> key;
      auto node = tree.find(key);
      if (node != nullptr) {
        out << "Node: " << *node << '\n';
      } else {
        out << "Key not found\n";
      }
      out << "Tree: " << tree << '\n';
    } else if (action == "finish") {
      break;
    } else {
      out << "Unknown action\n";
    }
  }
}

int main()
{
  run();
  return 0;
}