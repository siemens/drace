#ifndef PROF_HEADER_H
#define PROF_HEADER_H 1
#pragma once

#include <ipc/spinlock.h>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <string>
#include "parallel_hashmap/phmap.h"

#define PROF_START_BLOCK(name) \
  {                            \
    ProfTimer(name);
#define PROF_END_BLOCK }

#define PROF_FUNCTION() ProfTimer(std::string(__func__));

class ProfTimer {
 public:
  ProfTimer(std::string name);
  ~ProfTimer();

  void Stop();
  static void Print();

 private:
  std::string name_;
  std::chrono::time_point<std::chrono::high_resolution_clock> startTimePoint_;
  static phmap::parallel_flat_hash_map<std::string, double> TimeTable;
  static ipc::spinlock TimeTableSPL;
};

phmap::parallel_flat_hash_map<std::string, double> ProfTimer::TimeTable;
ipc::spinlock ProfTimer::TimeTableSPL;

ProfTimer::ProfTimer(std::string name) : name_(name) {
  std::lock_guard<ipc::spinlock> lg(ProfTimer::TimeTableSPL);
  if (ProfTimer::TimeTable.find(name_) == ProfTimer::TimeTable.end()) {
    ProfTimer::TimeTable[name_] = 0;
  }
  startTimePoint_ = std::chrono::high_resolution_clock::now();
}

ProfTimer::~ProfTimer() { Stop(); }

void ProfTimer::Stop() {
  auto endTimePoint = std::chrono::high_resolution_clock::now();
  auto start =
      std::chrono::time_point_cast<std::chrono::microseconds>(startTimePoint_)
          .time_since_epoch()
          .count();
  auto end =
      std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint)
          .time_since_epoch()
          .count();
  double duration = end - start;

  std::lock_guard<ipc::spinlock> lg(ProfTimer::TimeTableSPL);
  ProfTimer::TimeTable[name_] += duration * 0.001;
}

void ProfTimer::Print() {
  double totalTime = 0;
  for (auto& x : ProfTimer::TimeTable) {
    totalTime += x.second;
  }
  std::cout << "====----------- Profiling Information: -----------===="
            << std::endl;
  for (auto& x : ProfTimer::TimeTable) {
    std::cout << std::setw(15) << x.first << " = " << std::setw(5) << x.second
              << std::setw(8) << x.second / totalTime * 100 << "%" << std::endl;
  }
}
#endif
