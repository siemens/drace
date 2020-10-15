#ifndef PROF_HEADER_H
#define PROF_HEADER_H 1
#pragma once

//---------------------------------------------------------------------
// Header file that defines the profiling class ProfTimer used to
// benchmark code on the principle of RAII along with other useful
// commands used for debugging and profiling
//---------------------------------------------------------------------

#include <ipc/spinlock.h>
#include <chrono>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <string>
#include "parallel_hashmap/phmap.h"

#define deb(x) std::cout << #x << " = " << std::setw(3) << std::dec << x << " "
#define deb_hex(x) \
  std::cout << #x << " = 0x" << std::hex << x << std::dec << " "
#define deb_long(x) \
  std::cout << std::setw(50) << #x << " = " << std::setw(12) << x << " "
#define deb_short(x) \
  std::cout << std::setw(25) << #x << " = " << std::setw(5) << x << " "
#define newline() std::cout << std::endl

#define LINE() __LINE__
// printf("Not logical value at line number %d in file %s\n", __LINE__,
// __FILE__);

#define PRINT_TID()                                     \
  std::thread::id this_id = std::this_thread::get_id(); \
  std::cout << "thread ID is: " << this_id << "; ";

#define DEB_FUNCTION()         \
  std::string func = __func__; \
  std::cout << "Function " << func << " called\n";

#define SLEEP_THREAD()                                    \
  static int count = 0;                                   \
  if (count == 10) {                                      \
    std::this_thread::sleep_for(std::chrono::seconds(2)); \
    count = 0;                                            \
  }                                                       \
  count++;

#define CONCATENATE_IMPL(s1, s2) s1##s2
#define CONCATENATE(s1, s2) CONCATENATE_IMPL(s1, s2)

#ifdef __COUNTER__
#define ANONYMOUS_VARIABLE(str) CONCATENATE(str, __COUNTER__)
#else
#define ANONYMOUS_VARIABLE(str) CONCATENATE(str, __COUNTER__)
#endif

#define ANONYMOUS_NAME ANONYMOUS_VARIABLE(var)

#define PROF_START_BLOCK(name) \
  {                            \
    ProfTimer ANONYMOUS_NAME(name);
#define PROF_END_BLOCK }

#define PROF_FUNCTION()        \
  std::string func = __func__; \
  ProfTimer ANONYMOUS_NAME(func);

#define PROF_FUNCTION_W_NAME(name) ProfTimer ANONYMOUS_NAME(name);

// class ProfTimer {
//  public:
//   ProfTimer(std::string name);
//   ~ProfTimer();

//   void Stop();
//   static void Print();

//  private:
//   std::string name_;
//   std::chrono::time_point<std::chrono::high_resolution_clock>
//   startTimePoint_; static phmap::flat_hash_map<std::string, double>
//   TimeTable; static ipc::spinlock TimeTableSPL;
// };

// phmap::flat_hash_map<std::string, double> ProfTimer::TimeTable;
// ipc::spinlock ProfTimer::TimeTableSPL;

// ProfTimer::ProfTimer(std::string name) : name_(name) {
//   startTimePoint_ = std::chrono::high_resolution_clock::now();
// }

// ProfTimer::~ProfTimer() { Stop(); }

// void ProfTimer::Stop() {
//   auto endTimePoint = std::chrono::high_resolution_clock::now();
//   auto start =
//       std::chrono::time_point_cast<std::chrono::microseconds>(startTimePoint_)
//           .time_since_epoch()
//           .count();
//   auto end =
//       std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint)
//           .time_since_epoch()
//           .count();
//   double duration = end - start;
//   std::lock_guard<ipc::spinlock> lg(ProfTimer::TimeTableSPL);
//   ProfTimer::TimeTable[name_] += duration * 0.001;
// }

// void ProfTimer::Print() {
//   double totalTime = 0;
//   for (auto& x : ProfTimer::TimeTable) {
//     totalTime += x.second;
//   }
//   std::cout << "====----------- Profiling Information: -----------===="
//             << std::endl;
//   for (auto& x : ProfTimer::TimeTable) {
//     double percentage = x.second / totalTime * 100;
//     double millis = x.second;
//     x.second /= 1000;
//     int mins = x.second / 60;
//     double seconds = x.second - mins * 60;
//     printf("%25s = %15.3f ms: %4d min, %4.2f s, %6.3f %% \n",
//     x.first.c_str(),
//            millis, mins, seconds, percentage);
//   }
// }
#endif
