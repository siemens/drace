#pragma once
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

#include <detector/Detector.h>
#include <gtest/gtest.h>
#include <util/LibLoaderFactory.h>
#include <iostream>
#include <string>
#include <unordered_map>

static unsigned num_races = 0;
static Detector::Race last_race;

class DetectorTest : public ::testing::TestWithParam<const char*> {
 protected:
  /// map to store instances of dll / so loader objects
  static std::unordered_map<std::string, std::shared_ptr<util::LibraryLoader>>
      _libs;
  static std::unordered_map<std::string, Detector*> _detectors;

 public:
  Detector* detector;

  // each callback-call increases num_races by two
  static void callback(const Detector::Race* race, void* context) {
    // copy race to let testing code inspect it later
    last_race = *race;
    ++num_races;
  }

  DetectorTest() {
    num_races = 0;
    last_race = {};

    // we bind the detector lazy, i.e. if it is not loaded yet, load it
    auto det_it = _detectors.find(GetParam());
    if (det_it == _detectors.end()) {
      auto it = _libs.emplace(GetParam(), util::LibLoaderFactory::getLoader());
      std::string name = util::LibLoaderFactory::getModulePrefix() +
                         "drace.detector." + std::string(GetParam()) +
                         util::LibLoaderFactory::getModuleExtension();

      auto& loader = *(it.first->second);
      if (!loader.load(name.c_str()))
        throw std::runtime_error("could not load detector, check lib path");
      decltype(CreateDetector)* create_detector = loader["CreateDetector"];
      det_it = _detectors.emplace(GetParam(), create_detector()).first;

      detector = det_it->second;
      const char* _argv = "drace-tests.exe";
      detector->init(1, &_argv, callback, nullptr);
    } else {
      detector = det_it->second;
    }
  }
  ~DetectorTest() { detector = nullptr; }

  static void SetUpTestCase() {}

  static void TearDownTestCase() {
    for (auto& d : _detectors) {
      d.second->finalize();
    }
    _libs.clear();
  }
};
