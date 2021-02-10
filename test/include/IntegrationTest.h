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

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <thread>

#include "globals.h"

class Integration : public ::testing::Test {
 private:
  /// number of retries in case of sporadic crash
  static constexpr int startup_retries = 5;
  static std::string drrun;
  static std::string drclient;
  static std::string exe_suffix;
  static bool verbose;

  std::string logfile;

 public:
  Integration()
      : logfile(
            ::testing::UnitTest::GetInstance()->current_test_info()->name()) {
    std::replace(logfile.begin(), logfile.end(), '/', '_');
    logfile += ".log";
  }

  ~Integration() { std::remove(logfile.c_str()); }

  void run(const std::string& client_args, const std::string& exe, int min,
           int max, const std::string& app_args = "") {
    std::stringstream command;
    command << drrun << " -c " << drclient << " -d " << client_args
            << " --xml-file " << exe << ".xml"
            << " -- "
            << "bin/samples/" << exe << exe_suffix << " " << app_args << " > "
            << logfile << " 2>&1";
    if (verbose)
      std::cout << ">> Issue Command: " << command.str() << std::endl;

    // retry if DRace crashed sporadically
    // (e.g. because TSAN could not alloc at desired addr)
    // Note: in case of detected races, immediately return
    for (int i = 0; i < startup_retries; ++i) {
      std::system(command.str().c_str());

      // TODO: parse output
      std::ifstream filestream(logfile);
      std::stringstream file_content;
      file_content << filestream.rdbuf();
      std::string output(file_content.str());

      std::regex expr("found (\\d+) possible data-races");
      std::smatch races_match;
      if (std::regex_search(output, races_match, expr)) {
        std::string num_races_str;
        std::copy(races_match[1].first, races_match[1].second,
                  std::back_inserter(num_races_str));

        if (verbose)
          std::cout << ">>> Detected Races: " << num_races_str << std::endl;

        int num_races = std::stoi(num_races_str);
        if (num_races < min || num_races > max) {
          ADD_FAILURE() << "Expected [" << min << "," << max
                        << "] Races, found " << num_races << " in " << exe;
          if (verbose) {
            std::cout << output << std::endl;
          }
        }
        // we got a full execution, exit retry loop, but cleanup
        i = startup_retries;
      } else {
        if (verbose) {
          std::cout << output << std::endl;
        }

        if (i < startup_retries - 1) {
          std::cout << "DRace crashed sporadically, probably to allocation "
                       "error. Retry "
                    << startup_retries - i - 1 << std::endl;
          std::ofstream dst(logfile + ".retry" + std::to_string(i));
          dst << output;
          std::this_thread::sleep_for(std::chrono::seconds(15));
        } else {
          ADD_FAILURE() << "Race-Summary not found";
        }
      }
      // TODO: cleanup stuck processes, requires boost::processes i#36
    }
  }

  // TSAN can only be initialized once, even after a finalize
  static void SetUpTestCase() {
    for (int i = 1; i < t_argc; ++i) {
      if (i < (t_argc - 1) && strncmp(t_argv[i], "--dr", 8) == 0) {
        drrun = t_argv[i + 1];
      } else if (i < (t_argc - 1) && strncmp(t_argv[i], "-c", 8) == 0) {
        drclient = t_argv[i + 1];
      } else if (strncmp(t_argv[i], "-v", 4) == 0) {
        verbose = true;
      }
    }
    // insert quotation marks if paths contain whitespace
    if (drrun.find(' ') != std::string::npos) {
      drrun = "\"" + drrun + "\"";
    }
    if (drclient.find(' ') != std::string::npos) {
      drclient = "\"" + drclient + "\"";
    }

    if (verbose) {
      std::cout << "> Use DynamoRio in: " << drrun << std::endl;
      std::cout << "> Use DRace in:     " << drclient << std::endl;
    }
    // TODO: Check if drrun is present
    std::string dr_test(drrun + " -h > dr_avail.log");
    int ret = std::system(dr_test.c_str());
    if (ret) {
      throw std::runtime_error("drrun.exe not found");
    }
    std::ifstream f(drclient.c_str());
    if (!f.good()) {
      throw std::runtime_error("DRace not found");
    }
    f.close();
  }

  static void TearDownTestCase() { std::remove("dr_avail.log"); }
};

class DR : public Integration,
           public ::testing::WithParamInterface<const char*> {};
