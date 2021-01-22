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

#include "Data_Handler.h"

void Data_Handler::get_data(Report_Handler* rh, Command_Handler* ch,
                            Process_Handler* ph) {
  // Get report configuration data
  config.at("report_name") = rh->get_rep_name().toStdString();
  config.at("report_converter") = rh->get_rep_conv_cmd().toStdString();
  config.at("report_srcdirs") = rh->get_rep_srcdirs().toStdString();
  config.at("report_is_python") = rh->get_is_python();
  config.at("create_report") = rh->get_create_report();

  // Get DRaceGUI configuration data
  auto data = ch->get_command_array();
  config.at("dynamorio") = data[Command_Handler::DYNAMORIO].toStdString();
  config.at("dr_debug") = data[Command_Handler::DR_DEBUG].toStdString();
  config.at("dr_args") = data[Command_Handler::DR_ARGS].toStdString();
  config.at("drace") = data[Command_Handler::DRACE].toStdString();
  config.at("detector") = data[Command_Handler::DETECTOR].toStdString();
  config.at("detector_args") =
      data[Command_Handler::DETECTOR_ARGS].toStdString();
  config.at("flags") = data[Command_Handler::FLAGS].toStdString();
  config.at("ext_ctrl") = data[Command_Handler::EXT_CTRL].toStdString();
  config.at("configuration") =
      data[Command_Handler::CONFIGURATION].toStdString();
  config.at("executable") = data[Command_Handler::EXECUTABLE].toStdString();
  config.at("executable_args") =
      data[Command_Handler::EXECUTABLE_ARGS].toStdString();
  config.at("excl_stack") = data[Command_Handler::EXCL_STACK].toStdString();
  config.at("report_auto_open") =
      data[Command_Handler::REPORT_OPEN_CMD].toStdString();

  config.at("msr") = ch->get_msr_path().toStdString();

  // Get DRaceGUI misc checkboxes states
  const auto& options = ph->get_options_array();
  config.at("run_separately") = (bool)options[Process_Handler::RUN_SEPARATELY];
  config.at("wrap_text_output") =
      (bool)options[Process_Handler::WRAP_TEXT_OUTPUT];
}

void Data_Handler::set_data(Report_Handler* rh, Command_Handler* ch,
                            Process_Handler* ph) {
  // reset report handler members
  rh->set_report_name(QString(config.value("report_name", "").c_str()));
  rh->set_report_converter(
      QString(config.value("report_converter", "").c_str()));
  rh->set_report_srcdirs(QString(config.value("report_srcdirs", "").c_str()));
  rh->set_create_state(config.value("create_report", false));
  rh->set_is_python(config.value("report_is_python", false));

  // report handler sets now the new report command in the commandhandler
  rh->set_report_command();

  // reset command handler members
  ch->command[Command_Handler::DYNAMORIO] =
      QString(config.value("dynamorio", "").c_str());
  ch->command[Command_Handler::DR_DEBUG] =
      QString(config.value("dr_debug", "").c_str());
  ch->command[Command_Handler::DR_ARGS] =
      QString(config.value("dr_args", "").c_str());
  ch->command[Command_Handler::DRACE] =
      QString(config.value("drace", "").c_str());
  ch->command[Command_Handler::DETECTOR] =
      QString(config.value("detector", "").c_str());
  ch->command[Command_Handler::DETECTOR_ARGS] =
      QString(config.value("detector_args", "").c_str());
  ch->command[Command_Handler::FLAGS] =
      QString(config.value("flags", "").c_str());
  ch->command[Command_Handler::EXT_CTRL] =
      QString(config.value("ext_ctrl", "").c_str());
  ch->command[Command_Handler::CONFIGURATION] =
      QString(config.value("configuration", "").c_str());
  ch->command[Command_Handler::EXECUTABLE] =
      QString(config.value("executable", "").c_str());
  ch->command[Command_Handler::EXECUTABLE_ARGS] =
      QString(config.value("executable_args", "").c_str());
  ch->command[Command_Handler::EXCL_STACK] =
      QString(config.value("excl_stack", "").c_str());
  ch->command[Command_Handler::REPORT_OPEN_CMD] =
      QString(config.value("report_auto_open", "").c_str());

  ch->set_msr(QString(config.value("msr", "").c_str()));

  std::bitset<Process_Handler::_END_OF_MISC_OPTIONS_> options;
  options[Process_Handler::RUN_SEPARATELY] =
      config.value("run_separately", false);
  options[Process_Handler::WRAP_TEXT_OUTPUT] =
      config.value("wrap_text_output", true);
  ph->set_options_array(options);
}
