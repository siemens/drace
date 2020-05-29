#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

std::mutex mtx;

#define STR(x) #x

template <typename T>
void increment(T& ptr, int NUM) {
  for (int j = 0; j < NUM; j++) {
    //mtx.lock();
    ptr++;
    //mtx.unlock();
  }
}

int main() {
  std::vector<std::thread> threads;
  size_t size = 2;
  threads.reserve(size);

  int NUM = 5000;
  int i = 0;

  for (size_t j = 0; j < size; ++j) {
    threads.emplace_back(std::thread(increment<int>, std::ref(i), NUM));
  }

  for (int j = 0; j < NUM; j++) {
    mtx.lock();
    i++;
    mtx.unlock();
  }

  for (size_t i = 0; i < size; ++i) {
    threads[i].join();
  }

  std::cout << STR(i) << " =  " << i << std::endl;

  std::cin >> i;

  return 0;
}
