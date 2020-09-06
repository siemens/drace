#ifndef STACK_TRIE_HEADER_H
#define STACK_TRIE_HEADER_H 1

#include <string>

class StackTraceTrie {
  static constexpr int MAX_SIZE = 10;

  typedef struct TrieNode {
    TrieNode* values[MAX_SIZE];
    bool endOfAddress;
  } TrieNode;

  typedef struct LastTrieNode : public TrieNode {
    LastTrieNode(TrieNode _node, size_t parent_pc) {
      for (int i = 0; i < MAX_SIZE; ++i) {
        this->values[i] = _node.values[i];
      }
      this->endOfAddress = true;
      this->parent_pc = parent_pc;
    }

    size_t parent_pc;
  } LastTrieNode;

  TrieNode* _root;

  TrieNode* NewNode() {
    TrieNode* newNode = new TrieNode();
    newNode->endOfAddress = false;
    for (int i = 0; i < MAX_SIZE; i++) newNode->values[i] = nullptr;
    return newNode;
  }

 public:
  StackTraceTrie() { _root = NewNode(); }

  void InsertValue(std::string& _addr, size_t parent_pc) {
    TrieNode* iter = _root;
    TrieNode* last = nullptr;
    int index = -1;

    for (int i = 0; i < _addr.length(); ++i) {
      index = _addr[i] - 48;
      if (!iter->values[index]) {
        iter->values[index] = NewNode();
      }
      last = iter;
      iter = iter->values[index];
    }
    if (iter->endOfAddress = true) return;
    iter->endOfAddress = true;
    LastTrieNode* _lastTrieNode = new LastTrieNode(*iter, parent_pc);
    last->values[index] = _lastTrieNode;
    delete iter;
  }

  void RemoveValue(std::string& _addr) {}

  bool SearchValue(std::string& _addr) {
    TrieNode* iter = _root;

    for (int i = 0; i < _addr.length(); ++i) {
      int index = _addr[i] - 48;
      if (!iter->values[index]) return false;

      iter = iter->values[index];
    }
    return (iter != nullptr && iter->endOfAddress);
  }
};

#endif
