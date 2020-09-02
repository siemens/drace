#ifndef TRIE_HEADER_H
#define TRIE_HEADER_H 1

#include <sstream>
#include <string>

class MemoryTrie {
  // 16 is the maximum number of numbers
  // a hex value represented memory address can have
  static constexpr int MAX_SIZE = 10;

  typedef struct TrieNode {
    TrieNode* values[MAX_SIZE];

    bool endOfAddress;
  } TrieNode;

  TrieNode* _root;

  TrieNode* NewNode() {
    TrieNode* newNode = new TrieNode();
    newNode->endOfAddress = false;
    for (int i = 0; i < MAX_SIZE; i++) newNode->values[i] = nullptr;
    return newNode;
  }

 public:
  MemoryTrie() { _root = NewNode(); }

  void InsertValue(std::string&& _addr) {
    std::string addr = std::move(_addr);
    TrieNode* iter = _root;

    for (int i = 0; i < addr.length(); ++i) {
      int index = addr[i] - 48;
      if (!iter->values[index]) iter->values[index] = NewNode();
      iter = iter->values[index];
    }
    iter->endOfAddress = true;
  }

  bool SearchValue(std::string&& _addr) {
    std::string addr = std::move(_addr);
    TrieNode* iter = _root;

    for (int i = 0; i < addr.length(); ++i) {
      int index = addr[i] - 48;
      if (!iter->values[index]) return false;

      iter = iter->values[index];
    }
    return (iter != nullptr && iter->endOfAddress);
  }
};

class HexMemoryTrie {
  // 16 is the maximum number of characters a hex number can have
  static constexpr int MAX_SIZE = 16;

  typedef struct TrieNode {
    TrieNode* values[MAX_SIZE];
  } TrieNode;
  TrieNode* _root;

  TrieNode* NewNode() {
    TrieNode* newNode = new TrieNode();
    for (int i = 0; i < MAX_SIZE; i++) newNode->values[i] = nullptr;
    return newNode;
  }

 public:
  HexMemoryTrie() { _root = NewNode(); }

  void InsertValue(std::string& addr) {
    TrieNode* iter = _root;

    for (int i = 0; i < MAX_SIZE; ++i) {
      std::ostringstream ret;
      ret << std::dec << addr[i];
      std::string tmp = ret.str();
      int index = -1;
      if (tmp <= "9") {
        index = stoi(tmp);
      } else {
        index = (static_cast<int>(tmp[0]) - 87);
      }
      if (!iter->values[index]) iter->values[index] = NewNode();
      iter = iter->values[index];
    }
  }

  bool SearchValue(std::string& addr) {
    TrieNode* iter = _root;

    for (int i = 0; i < MAX_SIZE; ++i) {
      std::ostringstream ret;
      ret << std::dec << addr[i];
      std::string tmp = ret.str();
      int index = -1;
      if (tmp <= "9") {
        index = stoi(tmp);
      } else {
        index = (static_cast<int>(tmp[0]) - 87);
      }

      if (!iter->values[index]) return false;

      iter = iter->values[index];
    }
    return (iter != nullptr);
  }
};

#endif  // !TRIE_HEADER_H
