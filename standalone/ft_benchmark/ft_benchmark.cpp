/*
 * DRace, a dynamic data race detector
 *
 * Copyright 2018 Siemens AG
 *
 * Authors:
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
std::mt19937 gen{ 0 };

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
    // std::this_thread::sleep_for(std::chrono::milliseconds(500));
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
  } catch (...) {
    std::cout << "Something!" << std::endl;
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
    // std::this_thread::sleep_for(std::chrono::milliseconds(500));
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
  } catch (...) {
    std::cout << "Something!" << std::endl;
  }
}

int CountPossibleDataRaces();

/**
 * Test tool to check for memory corruption and layout.
 * To also check the race reporting, we try to enforce data-races
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

  int no_of_data_races = CountPossibleDataRaces();

  std::cout << "No. of possible data races: " << std::setw(3)
            << no_of_data_races << std::endl;
  //std::cin.get();
}

int CountPossibleDataRaces() {
  int result = 0;
  auto it_r = random_reads.begin();
  auto it_w = random_writes.begin();
  while (it_r != random_reads.end() && it_w != random_writes.end()) {
    if (*it_r == *it_w) {
      result++;  //= std::distance(blocks[*it_r].first, blocks[*it_w].second);
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
