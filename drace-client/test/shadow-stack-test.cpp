// /*
//  * DRace, a dynamic data race detector
//  *
//  * Copyright 2020 Siemens AG
//  *
//  * Authors:
//  *   Felix Moessbauer <felix.moessbauer@siemens.com>
//  *
//  * SPDX-License-Identifier: MIT
//  */

// #include "shadow-stack-test.h"
// #include "shadow-stack.h"

// // #include "func_def.h"

// #include <memory>

// using namespace drace;
// using namespace testing;

// std::unique_ptr<MockDetector> ShadowStackTest::detector;

// TEST_F(ShadowStackTest, PushPop) {
//   ShadowStack stack(ShadowStackTest::detector.get());

//   ASSERT_TRUE(stack.isEmpty());

//   void* addr = (void*)0x42ull;
//   EXPECT_CALL(*ShadowStackTest::detector, func_enter(_, addr)).Times(1);
//   stack.push(addr, nullptr);

//   EXPECT_CALL(*ShadowStackTest::detector, func_exit(_)).Times(1);
//   ASSERT_EQ(stack.pop(nullptr), addr);
// }

// TEST_F(ShadowStackTest, StackOverflow) {
//   ShadowStack stack(ShadowStackTest::detector.get());
//   intptr_t num_pushes = ShadowStack::maxSize() * 2;

//   EXPECT_CALL(*ShadowStackTest::detector, func_enter(_, _))
//       .Times((int)ShadowStack::maxSize());
//   for (intptr_t i = 0; i < num_pushes; ++i) {
//     // this must not crash
//     stack.push((void*)i, nullptr);
//   }
//   ASSERT_EQ(stack.size(), ShadowStack::maxSize());

//   EXPECT_CALL(*ShadowStackTest::detector, func_exit(_))
//       .Times((int)ShadowStack::maxSize());

//   intptr_t value = static_cast<intptr_t>(ShadowStack::maxSize() - 1);
//   while (!stack.isEmpty()) {
//     auto got_value = stack.pop(nullptr);
//     auto exp_value = (void*)value;
//     EXPECT_EQ(got_value, exp_value);
//     --value;
//   }
// }
