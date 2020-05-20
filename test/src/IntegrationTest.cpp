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

#include "IntegrationTest.h"

#include <signal.h>
#include <sys/types.h>
#include <future>
#include <iostream>
#include <string>

#ifdef DRACE_XML_EXPORTER
#include <tinyxml2.h>
#include <cstdio>
#endif

#ifdef WIN32
// https://docs.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt?view=vs-2019
#include <SDKDDKVer.h>
#endif

#include <boost/process.hpp>

namespace bp = boost::process;

bool Integration::verbose = false;

static auto msr_timeout = std::chrono::minutes(2);

// Test cases
TEST_P(DR, ConcurrentInc) { run(GetParam(), "gp-concurrent-inc", 1, 10); }
TEST_P(DR, IncMutex) { run(GetParam(), "gp-inc-mutex", 0, 0); }
#ifdef WINDOWS
// \todo port to linux
TEST_P(DR, LockKinds) { run(GetParam(), "gp-lock-kinds", 0, 0); }
#endif
TEST_P(DR, EmptyMain) { run(GetParam(), "gp-empty-main", 0, 0); }
TEST_P(DR, Atomics) { run(GetParam(), "gp-atomics", 0, 0); }
// TODO: Redesign test:
//       Currently, many runs miss the race as the race often
//       does not occur.
// TEST_P(FlagMode, RacyAtomics) {
//	run(GetParam(), "gp-atomics racy", 1, 10);
//}
TEST_P(DR, Annotations) { run(GetParam(), "gp-annotations", 0, 0); }
TEST_P(DR, DisabledAnnotations) {
  run(GetParam(), "gp-annotations-racy", 1, 20);
}

// Individual tests

TEST_P(DR, DelayedLookup) {
  // with delayed lookup all races have to be cached,
  // hence make test more difficult by disabling suppressions
  run(std::string(GetParam()) + " --delay-syms --suplevel 0",
      "gp-concurrent-inc", 1, 210);
}

TEST_P(DR, ExclStack) {
  run(std::string(GetParam()) + " --excl-stack", "gp-concurrent-inc", 1, 10);
}

TEST_P(DR, ExcludeRaces) {
  run(std::string(GetParam()) + " -c bin/data/drace_excl.ini",
      "gp-concurrent-inc", 0, 0);
}

TEST_P(DR, FaultInApp) { run(GetParam(), "gp-segfault", 0, 0); }

/// Make sure, the tutorial works as expected
TEST_P(DR, HowToTask) { run(GetParam(), "shoppingrush", 1, 10); }
TEST_P(DR, HowToSolution) { run(GetParam(), "shoppingrush-sol", 0, 0); }

TEST_P(DR, NoIoRaces) {
  // this test only works with Fasttrack due to the 32 bit
  // addr analysis of tsan
  if (std::string(GetParam()).compare("fasttrack") == 0) {
    run(std::string(GetParam()) + " --sup-races bin/data/racesup_incl_io.txt",
        "shoppingrush-sol", 0, 0);
  }
}

#ifdef DRACE_TESTING_DOTNET
// Dotnet Tests
TEST_P(DR, DotnetClrRacy) {
  bp::child msr("bin\\msr --once", bp::std_out > bp::null);

  run(std::string(GetParam()) + " --extctrl", "gp-cs-sync-clr", 1, 30, "none");

  if (!msr.wait_for(msr_timeout)) {
    msr.terminate();
  }
}

TEST_P(DR, DotnetClrMonitor) {
  bp::child msr("bin\\msr --once", bp::std_out > bp::null);

  run(std::string(GetParam()) + " --extctrl", "gp-cs-sync-clr", 0, 0,
      "monitor");

  if (!msr.wait_for(msr_timeout)) {
    msr.terminate();
  }
}

TEST_P(DR, DotnetClrMutex) {
  bp::child msr("bin\\msr --once", bp::std_out > bp::null);

  run(std::string(GetParam()) + " --extctrl", "gp-cs-sync-clr", 0, 0, "mutex");

  if (!msr.wait_for(msr_timeout)) {
    msr.terminate();
  }
}
#endif

#ifdef DRACE_XML_EXPORTER
TEST_P(DR, ReportXML) {
  std::string filename("reportXML.xml");
  run(std::string(GetParam()) + " --xml-file " + filename, "gp-concurrent-inc",
      1, 10);
  {
    tinyxml2::XMLDocument doc;
    ASSERT_EQ(doc.LoadFile(filename.c_str()), tinyxml2::XML_SUCCESS)
        << "File not found";
    const auto errornode =
        doc.FirstChildElement("valgrindoutput")->FirstChildElement("error");
    ASSERT_TRUE(errornode);
    EXPECT_GT(errornode->FirstChildElement("tid")->UnsignedText(), 0u);
    EXPECT_STREQ(errornode->FirstChildElement("kind")->GetText(), "Race");

    const auto status =
        doc.FirstChildElement("valgrindoutput")->LastChildElement("status");
    ASSERT_TRUE(status);
    EXPECT_STREQ(status->FirstChildElement("state")->GetText(), "FINISHED");
    EXPECT_GT(status->FirstChildElement("duration")->UnsignedText(), 0u);
    EXPECT_GT(errornode->FirstChildElement("timestamp")->UnsignedText(), 0u);

    const auto* stack = errornode->FirstChildElement("stack");
    ASSERT_TRUE(stack);
    const auto* frame = stack->FirstChildElement("frame");
    ASSERT_TRUE(frame);
    const auto* offset = frame->FirstChildElement("offset");
    ASSERT_TRUE(offset);
    EXPECT_LE(offset->UnsignedText(), 20u);
  }
  std::remove(filename.c_str());
}
#endif

TEST_P(DR, ReportText) {
  std::string filename("reportText.txt");
  run(std::string(GetParam()) + " --out-file " + filename, "gp-concurrent-inc",
      1, 10);
  {
    std::ifstream fstr(filename);
    if (fstr.good()) {
      std::string data((std::istreambuf_iterator<char>(fstr)),
                       (std::istreambuf_iterator<char>()));
      EXPECT_FALSE(data.find("Access 1 tid") == std::string::npos)
          << "race not found in report";
    } else {
      ADD_FAILURE() << "file not found";
    }
  }
  std::remove(filename.c_str());
}

// Setup value-parameterized tests
#if WINDOWS
INSTANTIATE_TEST_SUITE_P(Integration, DR,
                         ::testing::Values("fasttrack", "tsan"));
#else
INSTANTIATE_TEST_SUITE_P(Integration, DR, ::testing::Values("fasttrack"));
#endif
