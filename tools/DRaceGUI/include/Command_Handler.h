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

#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <QDir>
#include <QList>
#include <QRegExp>
#include <QString>
#include "Executor.h"

class Command_Handler {
 public:
  enum Command_Type : uint {
    DYNAMORIO,
    DR_DEBUG,
    DR_ARGS,
    DRACE,
    DETECTOR,
    DETECTOR_ARGS,
    FLAGS,
    REPORT_FLAG,
    EXT_CTRL,
    EXCL_STACK,
    CONFIGURATION,
    EXECUTABLE,
    EXECUTABLE_ARGS,
    REPORT_CMD,
    REPORT_OPEN_CMD,
    _END_OF_COMMAND_TYPE_
  };

 private:
  QString entire_command;

  QString msr_path;

  /// evaluates and returns the msr command when valid
  QString eval_msr_path(QObject* parent);

  /// sets the external control flag (needed for msr execution)
  void make_extctrl(bool set);

  /// sets the QString at the position in command array and returns the updated
  /// command
  QString updateCommand(const QString& arg1, uint position);

 public:
  Command_Handler(QString default_detector = "tsan");

  /// the command array holds each part of the command as a QString
  QString command[_END_OF_COMMAND_TYPE_];

  /// sets the flag entry
  QString make_flag_entry(const QString& arg1);

  /// sets the argument(s) (if any) of the executable under test
  QString make_text_entry(const QString& arg1, uint position);

  /// sets the automatic report opening command
  QString make_report_auto_open_entry(const int& arg1);

  /// makes a command entry into the command array and updates the
  /// entire_command
  QString make_entry(const QString& path, uint postion, QString prefix = "",
                     bool no_quotes = false);

  /// updates the entire command  and returns it
  QString make_command();

  QString get_command() { return entire_command; }
  QString get_msr_path() { return msr_path; }
  QString* get_command_array() { return command; }

  void set_msr(QString path) { msr_path = path; }

  /// returns true when command is not found invalid
  bool command_is_valid();

  /// searches in the drace path for the msr
  /// if found returns true and sets msr path
  bool validate_and_set_msr(bool on, QObject* parent);
};

#endif  // !COMMAND_HANDLER_H
