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

#ifndef DATA_HANDLER_H
#define DATA_HANDLER_H

#include <QString>
#include <bitset>
#include <string>
#include "Command_Handler.h"
#include "Process_Handler.h"
#include "Report_Handler.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Data_Handler {
 private:
  json config = {{"report_name", ""},
                 {"report_converter", ""},
                 {"report_srcdirs", ""},
                 {"dynamorio", ""},
                 {"dr_debug", ""},
                 {"dr_args", ""},
                 {"drace", ""},
                 {"detector", ""},
                 {"detector_args", ""},
                 {"flags", ""},
                 {"ext_ctrl", ""},
                 {"configuration", ""},
                 {"executable", ""},
                 {"executable_args", ""},
                 {"msr", ""},
                 {"excl_stack", ""},
                 {"report_is_python", false},
                 {"create_report", false},
                 {"report_auto_open", ""},
                 {"run_separately", false},
                 {"wrap_text_output", true}};

 public:
  /// loads data from handler classes to members
  void get_data(Report_Handler* rh, Command_Handler* ch, Process_Handler* ph);

  /// restores data from members back in the handler classes
  void set_data(Report_Handler* rh, Command_Handler* ch, Process_Handler* ph);

  /// getter for config json object
  const json& get_config() const { return config; }

  /// setter for config json object
  void set_config(json& j) { config = j; }
};
#endif  // !DATA_HANDLER_H
