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

#include "gtest/gtest.h"
#include "fasttrack_test.h"
#include <random>

//#include "stacktrace.h"


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

TEST(FasttrackTest, stackAllocations) {
	StackTrace st;
	std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(0, 9);

	for(int i = 0; i < 1000; ++i){

		for(int j = 0; j < 7; ++j){
			st.push_stack_element(j+i);
			st.set_read_write(j, j*100);
		}
		for(int k = 0; k < 5; ++k){
			st.pop_stack_element();
		}
		for(int l = 7; l < 10; ++l){
			st.push_stack_element(l+i);
			st.set_read_write(l, l*100);
		}
		for(int m = 0; m < 5; m++){
			st.pop_stack_element();
		}
		int addr = int(dist(mt));
		st.return_stack_trace(addr);
	}
}

TEST(FasttrackTest, stackInitializations){
	std::vector<std::shared_ptr<StackTrace>> vec;
	std::list<size_t> stack;
	for(int i = 0; i < 100; ++i){
		vec.push_back(std::make_shared<StackTrace>());
	}

	{
		std::shared_ptr<StackTrace> cp = vec[78];
		cp->push_stack_element(1);
		cp->push_stack_element(2);
		cp->push_stack_element(3);
		cp->set_read_write(5,4);
		stack = cp->return_stack_trace(5);
	}
	vec[78]->pop_stack_element();
	ASSERT_EQ(stack.size(), 4);
	int i = 1;
	for(auto it = stack.begin(); it != stack.end(); ++it){
		ASSERT_EQ(*(it), i);
		i++;
	}
}