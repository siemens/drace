/*
 * DRace, a dynamic data race detector
 *
 * Copyright 2020 Siemens AG
 *
 * Authors:
 *   Mihai Robescu <mihai-gabriel.robescu@siemens.com>
 *   Felix Moessbauer <felix.moessbauer@siemens.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <algorithm>
#include <atomic>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <random>
#include <set>
#include <thread>
#include <vector>

std::mutex mx;
static std::set<int> random_reads;
static std::set<int> random_writes;
static std::random_device rd{};
std::mt19937 gen{0};

void generate_block(int i,
                    std::vector<std::pair<uintptr_t*, uintptr_t*>>* blocks) {
  std::mt19937_64 gen(42 + i);
  uintptr_t* ptr = new uintptr_t[i];

  // fill block
  std::generate(ptr, ptr + i, [&]() { return gen(); });
  blocks->emplace_back(ptr, ptr + i);
}

void read_from_block(std::vector<std::pair<uintptr_t*, uintptr_t*>>* blocks) {
  try {
    std::uniform_int_distribution<int> dist(0, blocks->size() - 1);
    int rnd = dist(gen);
    random_reads.emplace(rnd);
    uintptr_t* begin = (*blocks)[rnd].first;
    uintptr_t* end = (*blocks)[rnd].second;

    uintptr_t* iter = begin;
    while (iter != end) {
      auto tmp = *iter;
      iter++;
    }
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
  } catch (...) {
    std::cout << "Unexpected failure occurred from ft_benchmark." << std::endl;
  }
}

void write_to_block(std::vector<std::pair<uintptr_t*, uintptr_t*>>* blocks) {
  try {
    std::uniform_int_distribution<int> dist(0, blocks->size() - 1);
    int rnd = dist(gen);
    random_writes.emplace(rnd);
    uintptr_t* begin = (*blocks)[rnd].first;
    uintptr_t* end = (*blocks)[rnd].second;

    uintptr_t* iter = begin;
    while (iter != end) {
      *iter = -1;
      iter++;
    }
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
  } catch (...) {
    std::cout << "Failure!" << std::endl;
  }
}

int count_possible_data_races();

/**
 * \brief benchmarking program used to test the performance of the FastTrack2
 * algorithm implementation
 */
int main(int argc, char** argv) {
  std::vector<std::pair<uintptr_t*, uintptr_t*>> blocks;
  size_t elem_allocated = 0;
  {
    mx.lock();
    for (int i = 0; i < 32; ++i) {
      for (int j = 2; j <= (32 * 1024); j *= 2) {
        generate_block(j, &blocks);
        elem_allocated += j;
      }
    }
    mx.unlock();
  }

  auto alloc_mb = (elem_allocated * sizeof(uintptr_t)) / (1024 * 1024);
  std::cout << "Allocated " << alloc_mb << " MiB" << std::endl;

  int no_of_blocks = 100;
  std::vector<std::thread> readers;
  std::vector<std::thread> writers;

  for (int i = 0; i < no_of_blocks; ++i) {
    readers.emplace_back(read_from_block, &blocks);
    writers.emplace_back(write_to_block, &blocks);
  }

  for (auto& t : readers) {
    t.join();
  }
  for (auto& t : writers) {
    t.join();
  }

  int no_of_data_races = count_possible_data_races();

  std::cout << "No. of possible data races: " << std::setw(3)
            << no_of_data_races << std::endl;
}

int count_possible_data_races() {
  int result = 0;
  auto it_r = random_reads.begin();
  auto it_w = random_writes.begin();
  while (it_r != random_reads.end() && it_w != random_writes.end()) {
    if (*it_r == *it_w) {
      result++;
      it_r++;
      it_w++;
      continue;
    }
    if (*it_r > *it_w) {
      it_w++;
    } else {
      it_r++;
    }
  }
  return result;
}
