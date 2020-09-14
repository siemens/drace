#pragma once
/*
 * DRace, a dynamic data race detector
 *
 * Copyright 2020 Siemens AG
 *
 * Authors:
 *   Felix Moessbauer <felix.moessbauer@siemens.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "../detectors/dummy/dummy.cpp"
#include "detector/Detector.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
// #include "func_def.h"
#include <memory>

class MockDetector : public drace::detector::Dummy {
 public:
  MOCK_METHOD(void, func_enter, (tls_t tls, void* pc), (override));
  MOCK_METHOD(void, func_exit, (tls_t tls), (override));
};

class ShadowStackTest : public ::testing::Test {
 protected:
  static std::unique_ptr<MockDetector> detector;

  virtual void SetUp() {
    detector.reset(new MockDetector());
    // we do not init the detector, as it's only a stub
  }

  virtual void TearDown() { detector.reset(); }
};
