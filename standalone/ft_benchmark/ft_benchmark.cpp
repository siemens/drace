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
#include <thread>
#include <vector>
#include <set>

std::mutex mx;

void generate_block(int i,
                    std::vector<std::pair<uintptr_t*, uintptr_t*>>* blocks) {

  std::mt19937_64 gen(42 + i);
  uintptr_t* ptr = new uintptr_t[i];

  // fill block
  std::generate(ptr, ptr + i, [&]() { return gen(); });
  blocks->emplace_back(ptr, ptr + i);
}

void read_from_block(uintptr_t* begin, uintptr_t* end) {
  uintptr_t* iter = begin;
  while (iter != end) {
    auto tmp = *iter;
    iter++;
  }
  //std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void write_to_block(uintptr_t* begin, uintptr_t* end, int i) {
  uintptr_t* iter = begin;
  while (iter != end) {
    *iter = i;
    iter++;
  }
  //std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

int CountPossibleDataRaces(std::set<int>& random_reads, std::set<int>& random_writes, std::vector<std::pair<uintptr_t*, uintptr_t*>>& blocks);

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
      for (int j = 2; j <= (64 * 1024); j *= 2) {
        generate_block(j, &blocks);
        elem_allocated += j;
      }
    }
    mx.unlock();
  }

  auto alloc_mb = (elem_allocated * sizeof(uintptr_t)) / (1024 * 1024);
  std::cout << "Allocated " << alloc_mb << " MiB" << std::endl;

  int no_of_blocks = 200;
  std::vector<std::thread> readers;
  std::vector<std::thread> writers;
  std::random_device rd{};
  std::mt19937 gen{ (unsigned)no_of_blocks };// seed with 
  std::uniform_int_distribution<int> dist(0, blocks.size() - 1);
  std::set<int> random_reads;
  std::set<int> random_writes;

  for (int i = 0; i < no_of_blocks; ++i) {
    if (i % 2 == 0) {
      int rnd = dist(gen);
      readers.emplace_back(read_from_block, blocks[rnd].first,
                           blocks[rnd].second);
      random_reads.emplace(rnd);
    } else {
      int rnd = dist(gen);
      writers.emplace_back(write_to_block, blocks[rnd].first,
                           blocks[rnd].second, i);
      random_writes.emplace(rnd);
    }
  }

  for (auto& t : readers) {
    t.join();
  }
  for (auto& t : writers) {
    t.join();
  }

  int no_of_data_races = CountPossibleDataRaces(random_reads, random_writes, blocks);
  std::cout << "No. of possible data races: " << std::setw(3) << no_of_data_races << std::endl;
}

int CountPossibleDataRaces(std::set<int>& random_reads, std::set<int>& random_writes, std::vector<std::pair<uintptr_t*, uintptr_t*>>& blocks) {
  int result = 0;
  auto it_r = random_reads.begin();
  auto it_w = random_writes.begin();
  while(it_r != random_reads.end() && it_w != random_writes.end()){
    if (*it_r == *it_w) {
      result ++; //= std::distance(blocks[*it_r].first, blocks[*it_w].second);
      it_r++;
      it_w++;
      continue;
    }
    if (*it_r > *it_w) {
      it_w++;
    }
    else {
      it_r++;
    }
  }
  return result;
}
