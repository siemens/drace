#ifndef STACK_TRIE_HEADER_H
#define STACK_TRIE_HEADER_H 1

class StackTraceTrie {
  static constexpr int MAX_SIZE = 10;

  class TrieNode {
   public:
    TrieNode* values[MAX_SIZE];
    bool endOfAddress;

    virtual ~TrieNode() = default;
  };

  class LastTrieNode : public TrieNode {
   public:
    LastTrieNode(TrieNode _node, size_t parent_pc) {
      for (int i = 0; i < MAX_SIZE; ++i) {
        this->values[i] = _node.values[i];
      }
      this->endOfAddress = true;
      this->parent_pc = parent_pc;
    }

    size_t parent_pc;
  };

  TrieNode* _root;

  TrieNode* NewNode() {
    TrieNode* newNode = new TrieNode();
    newNode->endOfAddress = false;
    for (int i = 0; i < MAX_SIZE; i++) newNode->values[i] = nullptr;
    return newNode;
  }

 public:
  StackTraceTrie() { _root = NewNode(); }

  void InsertValue(size_t _addr, size_t parent_pc) {
    TrieNode* iter = _root;
    TrieNode* last = nullptr;
    int index = -1;
    LastTrieNode* _lastTrieNode;

    while (_addr != 0){
      index = _addr % 10;
      if (!iter->values[index]) {
        iter->values[index] = NewNode();
      }
      last = iter;
      iter = iter->values[index];
      _addr /= 10;
    }
    if (iter->endOfAddress == true) return;
    iter->endOfAddress = true;
    _lastTrieNode = new LastTrieNode(*iter, parent_pc);
    last->values[index] = _lastTrieNode;
    // std::cout << "_lastTrieNode->parent_pc= " << _lastTrieNode->parent_pc
    //          << std::endl;
    delete iter;
  }

  void MultiThreadedInsert(std::string& _addr, size_t parent_pc) {
  //TODO: Reader lock
    TrieNode* iter = _root;
    int index = -1;

    for (int i = 0; i < _addr.length(); ++i) {
      index = _addr[i] - 48;
      if (!iter->values[index]) { // need to modify actual element => lock
        Insert(iter, _addr, i, parent_pc);
        return;
      }
      iter = iter->values[index];
    }
  }

  mutable ipc::spinlock _read_write_lock;
  void Insert(TrieNode* iter, std::string& _addr, int i, size_t parent_pc) {
    // TODO: Writer lock; Search & MakeTrace also Reader Lock
    std::lock_guard<ipc::spinlock> lg(_read_write_lock);
    LastTrieNode* _lastTrieNode;
    TrieNode* last = nullptr;
    int index = -1;
    for (; i < _addr.length(); ++i) {
      index = _addr[i] - 48;
      iter->values[index] = NewNode();

      last = iter;
      iter = iter->values[index];
    }
    iter->endOfAddress = true;
    _lastTrieNode = new LastTrieNode(*iter, parent_pc);
    last->values[index] = _lastTrieNode;
    delete iter;
  }


  // maybe I will need it in the future
  void RemoveValue(std::string& _addr) {}

  bool SearchValue(size_t _addr) {
    TrieNode* iter = _root;
    int index = -1;

    while (_addr != 0) {
      index = _addr % 10;
      if (!iter->values[index]) {
        iter->values[index] = NewNode();
      }
      iter = iter->values[index];
      _addr /= 10;
    }
    return (iter != nullptr && iter->endOfAddress);
  }

  std::list<size_t> MakeTrace(const std::pair<size_t, size_t>& data) const {
    std::list<size_t> this_stack;
    this_stack.push_front(data.first);

    size_t parent = data.second;
    size_t _addr;
    TrieNode* iter;

    do {
      this_stack.push_front(parent);
      _addr = parent;
      iter = _root;
      int index = -1;

      while (_addr != 0) {
        index = _addr % 10;
        iter = iter->values[index];  // It must exist; We should check though
        _addr /= 10;
      }

      LastTrieNode* _lastTrieNode = dynamic_cast<LastTrieNode*>(iter);
      parent = _lastTrieNode->parent_pc;
    } while (parent != 0);
    return this_stack;
  }
};

#endif
