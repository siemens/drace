/*
 * DRace, a dynamic data race detector
 *
 * Copyright 2018 Siemens AG
 *
 * Authors:
 *   Philip Harr <philip.harr@siemens.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <fasttrack.h>
#include <random>
#include "gtest/gtest.h"

//#include "stacktrace.h"

using ::testing::UnitTest;

//TEST(FasttrackTest, basic_stacktrace) {
//  StackTrace st;
//
//  st.push_stack_element(1);
//  st.set_read_write(100, 1000);
//  st.set_read_write(101, 1001);
//
//  st.push_stack_element(2);
//  st.set_read_write(102, 1002);
//
//  st.push_stack_element(3);
//  st.set_read_write(103, 1004);
//
//  st.pop_stack_element();
//
//  st.push_stack_element(1);
//  st.push_stack_element(2);
//  st.push_stack_element(6);
//  st.set_read_write(104, 1004);
//  st.push_stack_element(7);
//
//  std::list<size_t> list = st.return_stack_trace(104);
//  std::vector<size_t> vec(list.begin(), list.end());
//
//  ASSERT_EQ(vec[0], 1);
//  ASSERT_EQ(vec[1], 2);
//  ASSERT_EQ(vec[2], 1);
//  ASSERT_EQ(vec[3], 2);
//  ASSERT_EQ(vec[4], 6);
//  ASSERT_EQ(vec[5], 1004);
//  ASSERT_EQ(vec.size(), 6);
//}
//
//TEST(FasttrackTest, ItemNotFoundInTrace) {
//  StackTrace st;
//  st.push_stack_element(42);
//  // lookup element 40, which is not in the trace
//  auto list = st.return_stack_trace(40);
//  ASSERT_EQ(list.size(), 0);
//}
//
//TEST(FasttrackTest, stackInitializations) {
//  std::vector<std::shared_ptr<StackTrace>> vec;
//  std::list<size_t> stack;
//  for (int i = 0; i < 100; ++i) {
//    vec.push_back(std::make_shared<StackTrace>());
//  }
//
//  {
//    std::shared_ptr<StackTrace> cp = vec[78];
//    cp->push_stack_element(1);
//    cp->push_stack_element(2);
//    cp->push_stack_element(3);
//    cp->set_read_write(5, 4);
//    stack = cp->return_stack_trace(5);
//  }
//  vec[78]->pop_stack_element();
//  ASSERT_EQ(stack.size(), 4);  // TODO: validate
//  int i = 1;
//  for (auto it = stack.begin(); it != stack.end(); ++it) {
//    ASSERT_EQ(*(it), i);
//    i++;
//  }
//}

TEST(FasttrackTest, Indicate_Write_Write_Race) {
  std::size_t addr = 0x43EFull;
  using namespace drace::detector;
  auto ft = std::make_unique<Fasttrack<std::mutex>>();

  auto rc_clb = [](const Detector::Race* r) {};
  const char* argv_mock[] = {"ft_test"};
  void* tls[2];  // storage for TLS data

  ft->init(1, argv_mock, rc_clb);  
  ft->fork(0, 1, &tls[0]);         // t0
  ft->fork(0, 2, &tls[1]);         // t1

  ft->write(tls[0], (void*)0x78Eull, (void*)addr, 16);
  ft->write(tls[1], (void*)0x3A4Dull, (void*)addr, 16);

  EXPECT_EQ(ft->log_count.wr_race, 0);
  EXPECT_EQ(ft->log_count.rw_ex_race, 0);
  EXPECT_EQ(ft->log_count.ww_race, 1);
  EXPECT_EQ(ft->log_count.rw_sh_race, 0);
  ft->finalize();
}
//
TEST(FasttrackTest, Indicate_Exclusive_Read_Write_Race) {
  std::size_t addr = 0x65BEull;
  using namespace drace::detector;
  auto ft = std::make_unique<Fasttrack<std::mutex>>();

  auto rc_clb = [](const Detector::Race* r) {};
  const char* argv_mock[] = {"ft_test"};
  void* tls[2];  // storage for TLS data

  ft->init(1, argv_mock, rc_clb);  
  ft->fork(0, 1, &tls[0]);         // t0
  ft->fork(0, 2, &tls[1]);         // t1

  ft->read(tls[0], (void*)0x687CDull, (void*)addr, 16);
  ft->write(tls[1], (void*)0x9765Dull, (void*)addr, 16);

  EXPECT_EQ(ft->log_count.wr_race, 0);
  EXPECT_EQ(ft->log_count.rw_ex_race, 1);
  EXPECT_EQ(ft->log_count.ww_race, 0);
  EXPECT_EQ(ft->log_count.rw_sh_race, 0);
  ft->finalize();
}

TEST(FasttrackTest, Indicate_Write_Read_Race) {
  std::size_t addr = 0x342BEull;
  using namespace drace::detector;
  auto ft = std::make_unique<Fasttrack<std::mutex>>();

  auto rc_clb = [](const Detector::Race* r) {};
  const char* argv_mock[] = {"ft_test"};
  void* tls[2];  // storage for TLS data

  ft->init(1, argv_mock, rc_clb);  
  ft->fork(0, 1, &tls[0]);         // t0
  ft->fork(0, 2, &tls[1]);         // t1

  ft->write(tls[1], (void*)0x9868Dull, (void*)addr, 16);
  ft->read(tls[0], (void*)0x434CDull, (void*)addr, 16);

  EXPECT_EQ(ft->log_count.wr_race, 1);
  EXPECT_EQ(ft->log_count.rw_ex_race, 0);
  EXPECT_EQ(ft->log_count.ww_race, 0);
  EXPECT_EQ(ft->log_count.rw_sh_race, 0);
  ft->finalize();
}

TEST(FasttrackTest, Indicate_No_Race_Read_Exclusive) {
  std::size_t addr = 0x42Full;
  using namespace drace::detector;
  auto ft = std::make_unique<Fasttrack<std::mutex>>();

  auto rc_clb = [](const Detector::Race* r) {};
  const char* argv_mock[] = {"ft_test"};
  void* tls[3];  // storage for TLS data

  ft->init(1, argv_mock, rc_clb);  
  ft->fork(0, 1, &tls[0]);         // t0
  ft->fork(0, 2, &tls[1]);         // t1

  ft->acquire(tls[1], (void*)0x01000000, 1, true);
  ft->write(tls[1], (void*)0x5FFull, (void*)addr, 16);
  ft->release(tls[1], (void*)0x01000000, true);

  ft->acquire(tls[0], (void*)0x01000000, 1, false);
  ft->read(tls[0], (void*)0x5F3ull, (void*)addr, 16);
  ft->release(tls[0], (void*)0x01000000, true);

  EXPECT_EQ(ft->log_count.wr_race, 0);
  EXPECT_EQ(ft->log_count.rw_ex_race, 0);
  EXPECT_EQ(ft->log_count.ww_race, 0);
  EXPECT_EQ(ft->log_count.rw_sh_race, 0);
  EXPECT_EQ(ft->log_count.read_exclusive, 1);
  ft->finalize();
}

TEST(FasttrackTest, Indicate_No_Race_Write_Exclusive) {
  std::size_t addr = 0x42Full;
  using namespace drace::detector;
  auto ft = std::make_unique<Fasttrack<std::mutex>>();

  auto rc_clb = [](const Detector::Race* r) {};
  const char* argv_mock[] = { "ft_test" };
  void* tls[3];  // storage for TLS data

  ft->init(1, argv_mock, rc_clb);  
  ft->fork(0, 1, &tls[0]);         // t0
  ft->fork(0, 2, &tls[1]);         // t1

  ft->acquire(tls[1], (void*)0x01000000, 1, true);
  ft->read(tls[1], (void*)0x5F3ull, (void*)addr, 16);
  ft->release(tls[1], (void*)0x01000000, true);

  ft->acquire(tls[0], (void*)0x01000000, 1, false);
  ft->write(tls[0], (void*)0x5FFull, (void*)addr, 16);
  ft->release(tls[0], (void*)0x01000000, true);

  EXPECT_EQ(ft->log_count.wr_race, 0);
  EXPECT_EQ(ft->log_count.rw_ex_race, 0);
  EXPECT_EQ(ft->log_count.ww_race, 0);
  EXPECT_EQ(ft->log_count.rw_sh_race, 0);
  EXPECT_EQ(ft->log_count.read_exclusive, 1);
  EXPECT_EQ(ft->log_count.write_exclusive, 1);
  ft->finalize();
}

//this would have failed with the former condition in write():
//write exclusive with VAR_NOT_INIT
TEST(FasttrackTest, Indicate_Shared_Read_Write_Race) {
  std::size_t addr = 0x42Full;
  using namespace drace::detector;
  auto ft = std::make_unique<Fasttrack<std::mutex>>();
  // t1 and t2 read addr
  // t3 tries to write to addr

  auto rc_clb = [](const Detector::Race* r) {
    ASSERT_EQ(r->first.stack_size, 1);
    ASSERT_EQ(r->second.stack_size, 1);
    // first stack
    EXPECT_EQ(r->first.stack_trace[0], 0x5F3ull);
    // second stack
    EXPECT_EQ(r->second.stack_trace[0], 0x78Eull);
  };
  const char* argv_mock[] = {"ft_test"};
  void* tls[3];  // storage for TLS data

  ft->init(1, argv_mock, rc_clb);
  ft->fork(0, 1, &tls[0]);         // t0
  ft->fork(0, 2, &tls[1]);         // t1
  ft->fork(0, 3, &tls[2]);         // t2

  ft->read(tls[0], (void*)0x5F3ull, (void*)addr, 16);
  ft->read(tls[1], (void*)0x5FFull, (void*)addr, 16);
  ft->write(tls[2], (void*)0x78Eull, (void*)addr, 16);

  EXPECT_EQ(ft->log_count.wr_race, 0);
  EXPECT_EQ(ft->log_count.rw_ex_race, 0);
  EXPECT_EQ(ft->log_count.ww_race, 0);
  EXPECT_EQ(ft->log_count.rw_sh_race, 1);
  ft->finalize();
}

//check that read_shared variables can be dropped
TEST(FasttrackTest, Drop_State_Indicate_Shared_Read_Write_Race) {
  std::size_t addr[] = {0x5678Full, 0x678Full, 0x78Full };

  using namespace drace::detector;
  auto ft = std::make_unique<Fasttrack<std::mutex>>();
  // t1 and t2 read addr
  // t3 tries to write to addr

  auto rc_clb = [](const Detector::Race* r) {};
  const char* argv_mock[] = { "ft_test" , "--size", "2" };
  void* tls[3];  // storage for TLS data

  ft->init(1, argv_mock, rc_clb);
  ft->fork(0, 1, &tls[0]);         // t0
  ft->fork(0, 2, &tls[1]);         // t1
  ft->fork(0, 3, &tls[2]);         // t2

  ft->read(tls[0], (void*)0x5F3ull, (void*)addr[0], 16);
  ft->read(tls[1], (void*)0x5FFull, (void*)addr[0], 16);

  //ft->clearVarState(addr[0]);

  ft->write(tls[2], (void*)0x78Eull, (void*)addr[0], 16);

  EXPECT_EQ(ft->log_count.wr_race, 0);
  EXPECT_EQ(ft->log_count.rw_ex_race, 0);
  EXPECT_EQ(ft->log_count.ww_race, 0);
  EXPECT_EQ(ft->log_count.rw_sh_race, 1);
  ft->finalize();
}

TEST(FasttrackTest, Write_Write_Race) {
  std::size_t addr0 = 0x700ull;
  std::size_t addr1 = 0x701ull;
  std::size_t addr2 = 0x702ull;
  std::size_t addr3 = 0x703ull;
  std::size_t addr4 = 0x704ull;
  std::size_t addr5 = 0x705ull;

  using namespace drace::detector;
  auto ft = std::make_unique<Fasttrack<std::mutex>>();

  auto rc_clb = [](const Detector::Race* r) {};
  const char* argv_mock[] = { "ft_test" , "--size", "10"}; //making size bigger so the first variable will not be removed again
  void* tls[3];  // storage for TLS data
  void* mtx[2] = { (void*)0x123ull, (void*)0x1234ull };

  ft->init(3, argv_mock, rc_clb);
  ft->fork(0, 1, &tls[0]);         // t0
  ft->fork(0, 2, &tls[1]);         // t1
  ft->fork(0, 3, &tls[2]);         // t2

  ft->write(tls[1], (void*)0x78Eull, (void*)addr0, 16);

  ft->acquire(tls[0], mtx[0], 1, true);
  ft->read(tls[0], (void*)0x5F3ull, (void*)addr1, 16);
  ft->release(tls[0], mtx[0], true);
  ft->acquire(tls[0], mtx[0], 1, true);
  ft->read(tls[0], (void*)0x5F3ull, (void*)addr1, 16);
  ft->release(tls[0], mtx[0], true);

  ft->acquire(tls[2], mtx[0], 1, true);
  ft->read(tls[2], (void*)0x5F3ull, (void*)addr2, 16);
  ft->release(tls[2], mtx[0], true);
  ft->acquire(tls[2], mtx[0], 1, true);
  ft->read(tls[2], (void*)0x5F3ull, (void*)addr2, 16);
  ft->release(tls[2], mtx[0], true);

  ft->acquire(tls[1], mtx[1], 1, true);
  ft->read(tls[1], (void*)0x5F3ull, (void*)addr3, 16);
  ft->release(tls[1], mtx[1], true);
  ft->acquire(tls[1], mtx[1], 1, true);
  ft->read(tls[1], (void*)0x5F3ull, (void*)addr3, 16);
  ft->release(tls[1], mtx[1], true);

  ft->acquire(tls[0], mtx[0], 1, true);
  ft->read(tls[0], (void*)0x5F3ull, (void*)addr4, 16);
  ft->release(tls[0], mtx[0], true);
  ft->acquire(tls[0], mtx[0], 1, true);
  ft->read(tls[0], (void*)0x5F3ull, (void*)addr4, 16);
  ft->release(tls[0], mtx[0], true);

  ft->acquire(tls[2], mtx[0], 1, true);
  ft->read(tls[2], (void*)0x5F3ull, (void*)addr5, 16);
  ft->release(tls[2], mtx[0], true);
  ft->acquire(tls[2], mtx[0], 1, true);
  ft->read(tls[2], (void*)0x5F3ull, (void*)addr5, 16);
  ft->release(tls[2], mtx[0], true);

  ft->write(tls[2], (void*)0x78Eull, (void*)addr0, 16); //this is the race

  EXPECT_EQ(ft->log_count.wr_race, 0);
  EXPECT_EQ(ft->log_count.rw_ex_race, 0);
  EXPECT_EQ(ft->log_count.ww_race, 1);
  EXPECT_EQ(ft->log_count.rw_sh_race, 0);
  ft->finalize();
}

TEST(FasttrackTest, FullFtSimpleRace) {
  using namespace drace::detector;

  auto ft = std::make_unique<Fasttrack<std::mutex>>();
  auto rc_clb = [](const Detector::Race* r) {
    ASSERT_EQ(r->first.stack_size, 1);
    ASSERT_EQ(r->second.stack_size, 2);
    // first stack
    EXPECT_EQ(r->first.stack_trace[0], 0x1ull);
    // second stack
    EXPECT_EQ(r->second.stack_trace[0], 0x2ull);
    EXPECT_EQ(r->second.stack_trace[1], 0x3ull);
  };
  const char* argv_mock[] = {"ft_test"};
  void* tls[2];  // storage for TLS data

  ft->init(1, argv_mock, rc_clb);

  ft->fork(0, 1, &tls[0]);
  ft->fork(0, 2, &tls[1]);

  ft->write(tls[0], (void*)0x1ull, (void*)0x42ull, 1);
  ft->func_enter(tls[1], (void*)0x2ull);
  ft->write(tls[1], (void*)0x3ull, (void*)0x42ull, 1);
  // here, we expect the race. Handled in callback
  ft->finalize();
  __debugbreak();
}
