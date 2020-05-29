#include "prof.h"
#include <fasttrack.h>
#include <random>
#include <unordered_map>
#include "parallel_hashmap/phmap.h"

#define deb(x) std::cout << #x << " = " << std::setw(3) << x << " "

struct VC_ID {
  uint32_t TID : 10, Clock : 22;
};
void test_overflow();

int main() {
  phmap::parallel_node_hash_map<std::size_t, std::size_t> vars;

  vars.reserve(10000);

  std::array<int ,3> arr{1,2,3};
  auto it = arr.begin();
  for (; it != arr.end(); ++it) {
    std::cout<<*it << " ";
  }
  std::cout << std::endl;

  arr[2] = int();

  

  for (it = arr.begin(); it != arr.end(); ++it) {
    std::cout << *it << " ";
  }
  std::cout << std::endl;

  std::cin.get();
  return 0;
}

void test_overflow() {
  ProfTimer timer;
  VC_ID test;

  test.TID = 10;
  test.Clock = 4194305;

  deb(sizeof(test));
  deb(test.TID);
  deb(test.Clock);
}
