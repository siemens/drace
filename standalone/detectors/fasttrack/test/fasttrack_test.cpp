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

#include "stacktrace.h"

using ::testing::UnitTest;

TEST(FasttrackTest, basic_stacktrace) {
  StackTrace st;

  st.push_stack_element(1);
  st.set_read_write(100, 1000);
  st.set_read_write(101, 1001);

  st.push_stack_element(2);
  st.set_read_write(102, 1002);

  st.push_stack_element(3);
  st.set_read_write(103, 1004);

  st.pop_stack_element();

  st.push_stack_element(1);
  st.push_stack_element(2);
  st.push_stack_element(6);
  st.set_read_write(104, 1004);
  st.push_stack_element(7);

  std::list<size_t> list = st.return_stack_trace(104);
  std::vector<size_t> vec(list.begin(), list.end());

  ASSERT_EQ(vec[0], 1);
  ASSERT_EQ(vec[1], 2);
  ASSERT_EQ(vec[2], 1);
  ASSERT_EQ(vec[3], 2);
  ASSERT_EQ(vec[4], 6);
  ASSERT_EQ(vec[5], 1004);
  ASSERT_EQ(vec.size(), 6);
}

TEST(FasttrackTest, IndicateRaces3) {
  auto t1 = std::make_shared<ThreadState>(1);
  auto t2 = std::make_shared<ThreadState>(2);
  auto t3 = std::make_shared<ThreadState>(3);

  auto v1 = std::make_shared<VarState>(1);
  std::size_t addr = 0x4EA3ull;
  using namespace drace::detector;
  auto ft = std::make_unique<Fasttrack<std::mutex>>();
  // t1 and t2 read v1
  ft->read(t1.get(), v1.get(), addr);
  ft->read(t2.get(), v1.get(), addr);
  // ft->write(t2.get(), v1.get(), addr); //should be false after this

  xvector<VectorClock<>::VC_ID>* shared_vc;
  auto it = ft->shared_vcs.find(addr);
  if (it != ft->shared_vcs.end()) {  // if it exists get a pointer to the vector
    shared_vc = &(it->second);
  } else {  // if I find it is read_shared. else it is not
    shared_vc = nullptr;
  }

  ASSERT_FALSE(v1->is_wr_race(t3.get()));
  ASSERT_FALSE(v1->is_rw_ex_race(t3.get()));
  ASSERT_FALSE(v1->is_ww_race(t3.get()));
  ASSERT_TRUE(v1->is_rw_sh_race(t3.get(), shared_vc));
}

TEST(FasttrackTest, ItemNotFoundInTrace) {
  StackTrace st;
  st.push_stack_element(42);
  // lookup element 40, which is not in the trace
  auto list = st.return_stack_trace(40);
  ASSERT_EQ(list.size(), 0);
}

// TEST(FasttrackTest, stackAllocations) {
//  StackTrace st;
//  std::random_device rd;
//  std::mt19937 mt(rd());
//  std::uniform_real_distribution<float> dist(0, 9);
//
//  for (int i = 0; i < 1000; ++i) {
//    for (int j = 0; j < 7; ++j) {
//      st.push_stack_element(j + i);
//      st.set_read_write(j, j * 100);
//    }
//    for (int k = 0; k < 5; ++k) {
//      st.pop_stack_element();
//    }
//    for (int l = 7; l < 10; ++l) {
//      st.push_stack_element(l + i);
//      st.set_read_write(l, l * 100);
//    }
//    for (int m = 0; m < 5; m++) {
//      st.pop_stack_element();
//    }
//    int addr = int(dist(mt));
//    st.return_stack_trace(addr);
//  }
//}

TEST(FasttrackTest, stackInitializations) {
  std::vector<std::shared_ptr<StackTrace>> vec;
  std::list<size_t> stack;
  for (int i = 0; i < 100; ++i) {
    vec.push_back(std::make_shared<StackTrace>());
  }

  {
    std::shared_ptr<StackTrace> cp = vec[78];
    cp->push_stack_element(1);
    cp->push_stack_element(2);
    cp->push_stack_element(3);
    cp->set_read_write(5, 4);
    stack = cp->return_stack_trace(5);
  }
  vec[78]->pop_stack_element();
  ASSERT_EQ(stack.size(), 4);  // TODO: validate
  int i = 1;
  for (auto it = stack.begin(); it != stack.end(); ++it) {
    ASSERT_EQ(*(it), i);
    i++;
  }
}

// TEST(FasttrackTest, IndicateRaces1) {
//  auto t1 = std::make_shared<ThreadState>(1);
//  auto t2 = std::make_shared<ThreadState>(2);
//
//  auto v1 = std::make_shared<VarState>(1);
//
//  // t1 writes to v1
//  v1->update(true, t1->return_own_id());
//
//  ASSERT_TRUE(v1->is_wr_race(t2.get()));
//  ASSERT_FALSE(v1->is_rw_ex_race(t2.get()));
//  ASSERT_TRUE(v1->is_ww_race(t2.get()));
//}
//
// TEST(FasttrackTest, IndicateRaces2) {
//  auto t1 = std::make_shared<ThreadState>(1);
//  auto t2 = std::make_shared<ThreadState>(2);
//
//  auto v1 = std::make_shared<VarState>(1);
//
//  // t1 reads v1
//  v1->update(false, t1->return_own_id());
//
//  ASSERT_FALSE(v1->is_wr_race(t2.get()));
//  ASSERT_TRUE(v1->is_rw_ex_race(t2.get()));
//  ASSERT_FALSE(v1->is_ww_race(t2.get()));
//}

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
