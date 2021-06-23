#ifndef STACKTREE_NODE_H
#define STACKTREE_NODE_H

#include <stdexcept>

class INode {
 public:
  size_t pc = -1;           // 8 bytes
  INode* parent = nullptr;  // 8 bytes

  virtual INode* fast_check(size_t pc) const {
    throw std::runtime_error("Not implemented");
    return nullptr;
  }

  virtual size_t get_size() const {
    throw std::runtime_error("Not implemented");
    return -1;
  }

  virtual ~INode() {}

  virtual bool add_child_node(INode* next, size_t pc) {
    throw std::runtime_error("Not implemented");
    return 1;
  }

  virtual void change_child_node(INode* tmp, INode* _curr_elem) {
    throw std::runtime_error("Not implemented");
  }

  virtual void change_parent_node(INode* tmp) {
    throw std::runtime_error("Not implemented");
  }
};

template <size_t N>
class Node : public INode {
 public:
  std::array<size_t, N> child_values;  // N * 8 bytes
  std::array<INode*, N> child_nodes;   // N * 8 bytes

  ~Node() = default;
  Node& operator=(const Node& other) = default;
  Node(const Node& other) = delete;

  explicit Node() {
    pc = -1;
    parent = nullptr;
    for (int i = 0; i < N; ++i) {
      child_values[i] = -1;
    }
    for (int i = 0; i < N; ++i) {
      child_nodes[i] = nullptr;
    }
  }

  size_t get_size() const final { return N; }

  INode* fast_check(size_t pc) const final {
    for (int i = 0; i < N; ++i) {
      if (child_values[i] == pc) {
        return child_nodes[i];
      }
    }
    return nullptr;
  }

  bool add_child_node(INode* next, size_t pc) final {
    for (int i = 0; i < N; ++i) {
      if (child_values[i] == -1) {
        child_values[i] = pc;
        child_nodes[i] = next;
        return true;
      }
    }
    return false;
  }

  void change_child_node(INode* tmp, INode* _curr_elem) final {
    for (int i = 0; i < N; ++i) {
      if (child_nodes[i] == _curr_elem) {
        child_nodes[i] = tmp;
        return;
      }
    }  // replace new pointer in the parent list
  }

  void change_parent_node(INode* tmp) final {
    for (int i = 0; i < N; ++i) {
      child_nodes[i]->parent = tmp;
    }
  }
};

#endif  //! STACKTREE_NODE_H
