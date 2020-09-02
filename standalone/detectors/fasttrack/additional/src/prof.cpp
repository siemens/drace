#include <fasttrack.h>
#include <deque>
#include <execution>
#include <fstream>
#include <iostream>
#include <mutex>
#include <numeric>
#include <queue>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include "parallel_hashmap/phmap.h"

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

/*
---------------------------------------------------------------------
Small application developed to check the time used to find the memory addresses
in different data structures
---------------------------------------------------------------------
*/

struct VC_ID {
  uint32_t TID : 10, Clock : 22;
};
void test_overflow();
void test_vector();
void test_memoryTrie(std::vector<size_t>& addr);

typedef struct TH_NO {
  uint32_t num : 10;
} TH_NO;

template <class K, class V>
using phmap_parallel_node_hash_map_no_mtx = phmap::parallel_node_hash_map<
    K, V, phmap::container_internal::hash_default_hash<K>,
    phmap::container_internal::hash_default_eq<K>,
    std::allocator<std::pair<const K, V>>, 4, phmap::NullMutex>;

phmap_parallel_node_hash_map_no_mtx<size_t, VarState> vars;

int main() {
  std::string file_name = "hash_values.txt";
  std::ifstream file;

  file.open(file_name);
  if (!file.good()) {
    std::cerr << "File not found: " << file_name << std::endl;
    return 1;
  }

  std::vector<size_t> addr;
  std::vector<size_t> hashes;
  std::string line;
  while (std::getline(file, line)) {
    std::size_t value;
    std::stringstream lineStream(line);
    lineStream >> value;
    addr.emplace_back(value);
    std::size_t hash_value;
    lineStream >> hash_value;
    hashes.emplace_back(hash_value);
  }

  // for (int i = 0; i < 20; ++i) {
  //  std::ostringstream ret;
  //  ret << std::hex << std::setfill('0') << std::setw(16) << addr[i];
  //  addr_str.emplace_back(ret.str());
  //   std::cout << (void*)addr[i] << ", " << sizeof(addr[i]) << std::endl;
  //  // Print a memory address
  //   printf("variable addr[i] is at address: %p\n", (void*)&addr[i]);
  //}

  __debugbreak();
  int run_count = 0;
  while (run_count < 100) {
    for (int i = 0; i != addr.size(); i++) {
      auto it = vars.find(addr[i], hashes[i]);
      if (it == vars.end()) {
        it = vars.emplace(addr[i], VarState()).first;
      }
      // var = &(it->second);
    }
    run_count++;
  }

  __debugbreak();
  MemoryTrie* _memoryTrie = new MemoryTrie();
  run_count = 0;
  while (run_count < 100) {
    for (int i = 0; i != addr.size(); i++) {
      if (!_memoryTrie->SearchValue(std::to_string(addr[i]))) {
        _memoryTrie->InsertValue(std::to_string(addr[i]));
      }
    }
    run_count++;
  }

  __debugbreak();
  // HexMemoryTrie* _hexMemoryTrie = new HexMemoryTrie();
  // std::vector<std::string> addr_str;
  // run_count = 0;
  // while (run_count < 100) {
  //  for (int i = 0; i != addr.size(); i++) {
  //    std::ostringstream ret;
  //    ret << std::hex << std::setfill('0') << std::setw(16) << addr[i];
  //    std::string tmp = ret.str();

  //    if (!_hexMemoryTrie->SearchValue(tmp)) {
  //      _hexMemoryTrie->InsertValue(tmp);
  //    }
  //  }
  //  run_count++;
  //}

  //__debugbreak();
  std::cin.get();
  return 0;
}

void test_memoryTrie(std::vector<size_t>& addr) {
  __debugbreak();
  MemoryTrie* _memoryTrie = new MemoryTrie();
  for (int i = 0; i < 10; i++) {
    std::cout << addr[i] << std::endl;
    _memoryTrie->InsertValue(std::to_string(addr[i]));
  }

  int i = 2;
  // std::string tmp = std::to_string(addr[i]);
  std::cout << std::boolalpha
            << _memoryTrie->SearchValue(std::to_string(addr[i])) << std::endl;
  i = 7;
  std::cout << std::boolalpha
            << _memoryTrie->SearchValue(std::to_string(addr[i])) << std::endl;
  i = 9;
  std::cout << std::boolalpha
            << _memoryTrie->SearchValue(std::to_string(addr[i])) << std::endl;
  i = 15;
  std::cout << std::boolalpha
            << _memoryTrie->SearchValue(std::to_string(addr[i])) << std::endl;
  i = 18;
  std::cout << std::boolalpha
            << _memoryTrie->SearchValue(std::to_string(addr[i])) << std::endl;
}

void test_vector() {
  std::vector<int> vec;
  // vec.reserve(10);
  // vec.emplace_back(3);
  // deb(vec.capacity());

  // vec.reserve(50);
  // deb(vec.capacity());

  // vec.reserve(2);
  // deb(vec.capacity());

  deb(vec.size());
  deb(vec.capacity());
  vec.emplace_back(3);
  vec.emplace_back(4);
  vec.emplace_back(5);
  vec.emplace_back(6);
  vec.reserve(10);
  std::vector<int> vec2(100, 0);
  // vec2 = std::move(vec);
  vec.at(8);
  deb(vec2.size());
  deb(vec2.capacity());
  deb(vec.size());
  deb(vec.capacity());

  for (auto& x : vec) {
    deb(x);
  }
}

void test_overflow() {
  VC_ID test;

  test.TID = 10;
  test.Clock = 4194305;

  deb(sizeof(test));
  deb(test.TID);
  deb(test.Clock);
}
