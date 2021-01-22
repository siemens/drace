/*
 * DRace, a dynamic data race detector
 *
 * Copyright 2021 Siemens AG
 *
 * Authors:
 *   Mohamad Kanj <mohamad.kanj@siemens.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DETECTOR_OPTIONS_DIALOG_H
#define DETECTOR_OPTIONS_DIALOG_H

#include <QDialog>
#include "Command_Handler.h"

namespace Ui {
class Detector_Options_Dialog;
}

class Detector_Options_Dialog : public QDialog {
  Q_OBJECT

 public:
  /// displays the detector options dialog when called
  explicit Detector_Options_Dialog(Command_Handler *ch,
                                   QWidget *parent = nullptr);
  ~Detector_Options_Dialog();

 private slots:
  /// Push buttons
  void on_buttonBox_rejected();
  void on_buttonBox_accepted();

  /// Checkboxes
  void on_heap_only_box_stateChanged(int arg1);

 private:
  Ui::Detector_Options_Dialog *ui;
  Command_Handler *c_handler;

  QString heap_only = "--heap-only";
};

#endif  // DETECTOR_OPTIONS_DIALOG_H