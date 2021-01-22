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

#include "Detector_Options_Dialog.h"
#include "ui_Detector_Options_Dialog.h"

Detector_Options_Dialog::Detector_Options_Dialog(Command_Handler *ch,
                                                 QWidget *parent)
    : QDialog(parent), ui(new Ui::Detector_Options_Dialog), c_handler(ch) {
  ui->setupUi(this);
  ui->detector_args_input->setText(
      c_handler->command[Command_Handler::DETECTOR_ARGS]);
  ui->heap_only_box->setChecked(
      ui->detector_args_input->text().contains(heap_only));
}

Detector_Options_Dialog::~Detector_Options_Dialog() { delete ui; }

/// push button functions
void Detector_Options_Dialog::on_buttonBox_rejected() { this->close(); }

void Detector_Options_Dialog::on_buttonBox_accepted() {
  c_handler->make_text_entry(ui->detector_args_input->text(),
                             Command_Handler::DETECTOR_ARGS);
  this->close();
}

/// checkbox functions
void Detector_Options_Dialog::on_heap_only_box_stateChanged(int arg1) {
  if (arg1 == 0) {
    QString resulting_text;
    if (ui->detector_args_input->text().contains(heap_only)) {
      resulting_text = ui->detector_args_input->text().remove(heap_only);
    }
    ui->detector_args_input->setText(resulting_text.trimmed());

  } else {
    QString suffix = "";
    if (!ui->detector_args_input->text().isEmpty()) {
      suffix = " ";
    }
    if (!ui->detector_args_input->text().contains(heap_only)) {
      ui->detector_args_input->setText(heap_only + suffix +
                                       ui->detector_args_input->text());
    }
  }
}