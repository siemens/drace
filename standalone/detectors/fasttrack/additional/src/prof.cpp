#include <fasttrack.h>
#include <numeric>
#include <queue>
#include <random>
#include <deque>
#include "parallel_hashmap/phmap.h"
#include <unordered_map>

struct VC_ID {
  uint32_t TID : 10, Clock : 22;
};
void test_overflow();

typedef struct TH_NO {
  uint32_t num:10;
} TH_NO;

int main() {
  phmap::parallel_node_hash_map<std::size_t, std::size_t> vars;

  // vars.reserve(10000);

  // std::array<int ,3> arr{1,2,3};
  // auto it = arr.begin();
  // for (; it != arr.end(); ++it) {
  //  std::cout<<*it << " ";
  //}
  // std::cout << std::endl;

  // arr[2] = int();

  //

  // for (it = arr.begin(); it != arr.end(); ++it) {
  //  std::cout << *it << " ";
  //}
  // std::cout << std::endl;

  //std::deque<int> q;
  //q.resize(1023);
  //int k = 1;
  //for (auto it = q.begin(); it != q.end(); ++it) {
  //  *it = k;
  //  k++;
  //}

  //for (auto it = q.begin(); it != q.end(); ++it) {
  //  std::cout << *it << " ";
  //}
  //std::cout << std::endl;

  //uint32_t value = 1;
  //TH_NO th_no;
  //th_no.num = value;
  //std::cout << th_no.num << std::endl;
  //value = 1025;
  //th_no.num = value;
  //std::cout << th_no.num << std::endl;

  std::vector<int> vec;
  //vec.reserve(10);
  //vec.emplace_back(3);
  //deb(vec.capacity());

  //vec.reserve(50);
  //deb(vec.capacity());

  //vec.reserve(2);
  //deb(vec.capacity());

  deb(vec.size());
  deb(vec.capacity());
  vec.emplace_back(3);
  vec.emplace_back(4);
  vec.emplace_back(5);
  vec.emplace_back(6);
  vec.reserve(10);
  std::vector<int> vec2(100,0);
  //vec2 = std::move(vec);
  vec.at(8);
  deb(vec2.size());
  deb(vec2.capacity());
  deb(vec.size());
  deb(vec.capacity());
  
  for (auto& x : vec) {
    deb(x);
  }

  std::cin.get();
  return 0;
}

void test_overflow() {
  VC_ID test;

  test.TID = 10;
  test.Clock = 4194305;

  deb(sizeof(test));
  deb(test.TID);
  deb(test.Clock);
}
