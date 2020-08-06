#include <execution>
#include <fstream>
#include <iostream>
#include <mutex>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include "parallel_hashmap/phmap.h"

/*
---------------------------------------------------------------------
Small application developed to check the time used to find the memory addresses
in different data structures
---------------------------------------------------------------------
*/

class VarState {
 public:
  static constexpr uint32_t VAR_NOT_INIT = 0;
  static constexpr uint32_t R_ID_SHARED = -1;

  uint32_t w_id{VAR_NOT_INIT};
  uint32_t r_id{VAR_NOT_INIT};

  VarState() = default;
};

template <class K, class V>
using phmap_parallel_node_hash_map_no_mtx = phmap::parallel_node_hash_map<
    K, V, phmap::container_internal::hash_default_hash<K>,
    phmap::container_internal::hash_default_eq<K>,
    std::allocator<std::pair<const K, V>>, 1, phmap::NullMutex>;

phmap_parallel_node_hash_map_no_mtx<size_t, VarState> vars;
void removeRandomVarStates();

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

  __debugbreak();
  std::cout << "addr.size() = " << addr.size() << std::endl;
  int run_count = 0;
  while (run_count < 100) {
    for (int i = 0; i != addr.size(); i++) {
      auto it = vars.find(addr[i]);
      if (it == vars.end()) {
        it = vars.emplace(addr[i], VarState()).first;
      }
      // var = &(it->second);
    }
    std::cout << "vars.size() = " << vars.size() << std::endl;
    removeRandomVarStates();
    std::cout << "vars.size() = " << vars.size() << std::endl;
    run_count++;
  }
  __debugbreak();

  std::cin.get();
  return 0;
}

void removeRandomVarStates() {
  auto it = vars.begin();
  // number of VarStates to consider at once for choosing
  std::size_t no_VarStates = 4;
  std::size_t pos = 0;
  std::size_t vsize = vars.size();

  auto now = std::chrono::high_resolution_clock::now();
  auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now)
                .time_since_epoch()
                .count();
  std::mt19937 _gen{(unsigned int)ms};
  std::uniform_int_distribution<int> dist(0, no_VarStates - 1);

  while (pos < vsize - 1) {
    std::size_t random = dist(_gen);
    // TODO: needs adjustment, the threshold. might be too low
    pos += random;
    if (pos > vsize - 1) break;

    std::advance(it, random);  // might skip vars.end();
                               // this is why we have the pos

    // remove the variable, we don't care if it is read_shared or not
    auto random_it = it;
    pos++;
    if (pos <= vsize - 1) {
      it++;
    }
    vars.erase(random_it);
  }
}
