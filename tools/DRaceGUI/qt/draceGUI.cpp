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

#include "draceGUI.h"
#include "./ui_draceGUI.h"

DRaceGUI::DRaceGUI(QString config, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::DRaceGUI) {
  ch = new Command_Handler();
  rh = new Report_Handler(ch, this);
  ph = new Process_Handler(this);
  ui->setupUi(this);
  ui->splitter->setSizes(QList<int>({1, 2}));

  if (config != "") {
    // open drace with pre-filled data from the input config file
    Load_Save ls(rh, ch, ph);
    if (ls.load(config.toStdString())) {
      // restore all the data to the boxes if config file is valid
      set_boxes_after_load();
    } else {
      QMessageBox temp;
      temp.setIcon(QMessageBox::Warning);
      temp.setText("File is not valid!");
      temp.exec();
    }
  }

  else {
    // open drace with empty gui
    QString drrun_temp = "drrun.exe";
    QFileInfo drrun = drrun_temp;
    // check if drrun.exe is in the same folder
    if (drrun.exists()) {
      ui->dr_path_input->setText(drrun.absoluteFilePath());
    } else {
      ui->dr_path_input->setText(drrun_temp);
    }

    ui->command_output->setText(ch->get_command());

    // check if drace-client.dll is in the same folder
    QString dr_temp = "drace-client.dll";
    QFileInfo drace_client = dr_temp;
    if (drace_client.exists()) {
      ui->drace_path_input->setText(drace_client.absoluteFilePath());
    }
  }

  connect(ph->get_process(), SIGNAL(readyReadStandardOutput()), this,
          SLOT(read_output()));
  connect(ph->get_process(), &QProcess::stateChanged,
          [=](QProcess::ProcessState state) {
            on_proc_state_label_stateChanged(state);
          });
  connect(ph->get_process(),
          QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
          [=](int exitCode, QProcess::ExitStatus exitStatus) {
            ui->embedded_output->moveCursor(QTextCursor::End);
            if (!ui->embedded_input->text().isEmpty()) {
              ui->embedded_input->clear();
            }
            if (exitCode < 0) {
              on_proc_crash_exit_triggered(exitCode);
            }
          });
  connect(ph->get_process(),
          QOverload<QProcess::ProcessError>::of(&QProcess::errorOccurred),
          [=](QProcess::ProcessError error) { on_proc_errorOccured(error); });
}

DRaceGUI::~DRaceGUI() {
  if (ph->is_starting_or_running()) {
    ph->stop();
  }
  delete rh, ch, ph, ui;
}

// Browse button functions
void DRaceGUI::on_dr_path_btn_clicked() {
  QString path = QFileDialog::getOpenFileName(this, "Open File", dr_pth_cache,
                                              tr("Executable (*.exe)"));
  if (!path.isEmpty()) {
    dr_pth_cache = path;
    ui->dr_path_input->setText(path);
  }
}

void DRaceGUI::on_drace_path_btn_clicked() {
  QString path = QFileDialog::getOpenFileName(
      this, "Open File", drace_pth_cache, tr("Dll (*.dll)"));
  if (!path.isEmpty()) {
    drace_pth_cache = path;
    ui->drace_path_input->setText(path);
  }
}

void DRaceGUI::on_detector_options_btn_clicked() {
  Detector_Options_Dialog *detector_options_window;
  detector_options_window = new Detector_Options_Dialog(ch, this);
  detector_options_window->exec();
  ui->command_output->setText(ch->get_command());
}

void DRaceGUI::on_dr_options_btn_clicked() {
  DR_Options_Dialog *dr_options_window;
  dr_options_window = new DR_Options_Dialog(ch, this);
  dr_options_window->exec();
  ui->command_output->setText(ch->get_command());
}

void DRaceGUI::on_exe_path_btn_clicked() {
  QString selfilter = tr("Executable (*.exe)");
  QString path = QFileDialog::getOpenFileName(
      this, "Open File", exe_pth_cache,
      tr("All files (*.*);;Executable (*.exe)"), &selfilter);

  if (!path.isEmpty()) {
    exe_pth_cache = path;
    ui->exe_input->setText(path);
  }
}

void DRaceGUI::on_config_browse_btn_clicked() {
  QString selfilter = tr("INI (*.ini)");
  QString path = QFileDialog::getOpenFileName(
      this, "Open File", config_pth_cache, tr("All files (*.*);;INI (*.ini)"),
      &selfilter);

  if (!path.isEmpty()) {
    config_pth_cache = path;
    ui->config_file_input->setText(path);
  }
}

// push button functions
void DRaceGUI::on_run_button_clicked() {
  if (ch->command_is_valid()) {
    const bool embedded = !(ph->get_run_separately());
    if (ph->is_starting_or_running() && embedded) {
      QMessageBox temp;
      temp.setIcon(QMessageBox::Warning);
      temp.setText("A process is already running.");
      temp.exec();
      return;
    }
    QString msr = ch->get_msr_path();
    if (msr != "") {
      Executor::launch_msr(
          msr.toStdString());  // returns when msr is (should be :D) set up
    }

    if (embedded) {
      ui->embedded_output->clear();
      Executor::execute_embedded(ph->get_process(), ch->get_command(), this);
    } else {
      Executor::execute((ch->get_command()).toStdString(), this);
    }
    return;
  }

  QMessageBox temp;
  temp.setIcon(QMessageBox::Warning);
  temp.setText("The command is not valid!");
  temp.exec();
}

void DRaceGUI::on_copy_button_clicked() {
  clipboard->setText(ch->get_command());
}

QString DRaceGUI::get_latest_report_folder() {
  QDir dir(QDir::currentPath(), "drace_report_*", QDir::Name | QDir::Reversed,
           QDir::Dirs);
  QStringList reports = dir.entryList();

  if (reports.isEmpty()) {
    return "";
  } else {
    QDir latest_report_folder = reports.at(0);
    return latest_report_folder.absolutePath();
  }
}

void DRaceGUI::on_report_folder_open_clicked() {
  QString latest_report_folder = get_latest_report_folder();

  if (latest_report_folder == "") {
    QMessageBox temp;
    temp.setIcon(QMessageBox::Warning);
    temp.setText("Current directory does not contain a DRace report.");
    temp.exec();
  } else {
    QDesktopServices::openUrl(QUrl(latest_report_folder, QUrl::TolerantMode));
  }
}

void DRaceGUI::on_report_open_clicked() {
  QString latest_report_folder = get_latest_report_folder();

  if (latest_report_folder == "") {
    QMessageBox temp;
    temp.setIcon(QMessageBox::Warning);
    temp.setText("Current directory does not contain a DRace report.");
    temp.exec();
  } else {
    QFileInfo report(latest_report_folder, "index.html");
    if (report.exists()) {
      QDesktopServices::openUrl(
          QUrl(report.absoluteFilePath(), QUrl::TolerantMode));
    } else {
      QMessageBox temp;
      temp.setIcon(QMessageBox::Warning);
      temp.setText(
          "The latest DRace report folder does not contain an HTML report.");
      temp.exec();
    }
  }
}

void DRaceGUI::on_stop_button_clicked() {
  if (ph->is_starting_or_running()) {
    ph->stop();
  }
  ui->embedded_output->append("Process terminated prematurely.");
}

void DRaceGUI::on_clear_output_clicked() { ui->embedded_output->clear(); }

// radio buttons
void DRaceGUI::on_tsan_btn_clicked() {
  ui->command_output->setText(
      ch->make_entry("tsan", Command_Handler::DETECTOR, "-d"));
}

void DRaceGUI::on_dummy_btn_clicked() {
  ui->command_output->setText(
      ch->make_entry("dummy", Command_Handler::DETECTOR, "-d"));
}

void DRaceGUI::on_fasttrack_btn_clicked() {
  ui->command_output->setText(
      ch->make_entry("fasttrack", Command_Handler::DETECTOR, "-d"));
}

// tick box
void DRaceGUI::on_dr_debug_stateChanged(int arg1) {
  if (arg1 == 0) {
    ui->command_output->setText(ch->make_entry("", Command_Handler::DR_DEBUG));
  } else {
    ui->command_output->setText(
        ch->make_entry("-debug", Command_Handler::DR_DEBUG));
  }
}

void DRaceGUI::set_auto_report_checkable(int checked) {
  if (checked == 0) {
    ui->report_auto_open_box->setChecked(false);
    ui->report_auto_open_box->setEnabled(false);
  } else {
    ui->report_auto_open_box->setEnabled(true);
  }
}

void DRaceGUI::on_report_creation_stateChanged(int arg1) {
  rh->set_create_state(arg1);
  if (rh->set_report_command() || !arg1) {
    ui->command_output->setText(ch->get_command());
    set_auto_report_checkable(arg1);
    return;
  } else {  // if command is not set and shall be set, open report converter
            // dialog
    Report_Config *report_window;
    report_window = new Report_Config(rh);
    report_window->exec();
  }
  if (rh->set_report_command()) {  // now check again if converter is now set
                                   // correctly
    ui->command_output->setText(ch->get_command());
    set_auto_report_checkable(arg1);
    return;
  }

  // print error message if it still isn't set
  QMessageBox temp;
  temp.setIcon(QMessageBox::Warning);
  temp.setText("ReportConverter is not set correctly");
  temp.exec();
  ui->report_creation->setChecked(false);
  ui->report_creation->setCheckState(Qt::Unchecked);
}

void DRaceGUI::on_msr_box_stateChanged(int arg1) {
  if (!(ch->validate_and_set_msr(arg1, this)) && arg1) {
    QMessageBox temp;
    temp.setIcon(QMessageBox::Warning);
    temp.setText(
        "DRace client path is not set or \"msr.exe\" is not at DRace path.");
    temp.exec();
    ui->msr_box->setChecked(false);
    ui->msr_box->setCheckState(Qt::Unchecked);
  }
  ui->command_output->setText(ch->get_command());
}

void DRaceGUI::on_excl_stack_box_stateChanged(int arg1) {
  if (arg1 == 0) {
    ui->command_output->setText(
        ch->make_entry("", Command_Handler::EXCL_STACK));
  } else {
    ui->command_output->setText(
        ch->make_entry("--excl-stack", Command_Handler::EXCL_STACK));
  }
}

void DRaceGUI::on_report_auto_open_box_stateChanged(int arg1) {
  if (arg1 == 0) {
    rh->set_is_auto_open(false);
  } else {
    rh->set_is_auto_open(true);
  }

  if (rh->set_report_command()) {
    ui->command_output->setText(ch->get_command());
  }
  ui->command_output->setText(ch->make_report_auto_open_entry(arg1));
}

void DRaceGUI::on_run_separate_box_stateChanged(int arg1) {
  if (arg1 == 0) {
    ph->set_run_separately(false);
    if (ph->is_starting_or_running() && ui->run_button->isEnabled()) {
      ui->run_button->setEnabled(false);
    }
  } else {
    ph->set_run_separately(true);
    if (!ui->run_button->isEnabled()) {
      ui->run_button->setEnabled(true);
    }
  }
}

void DRaceGUI::on_wrap_text_box_stateChanged(int arg1) {
  if (arg1 == 0) {
    ph->set_wrap_text_output(false);
    ui->embedded_output->setLineWrapMode(QTextEdit::NoWrap);
  } else {
    ph->set_wrap_text_output(true);
    ui->embedded_output->setLineWrapMode(QTextEdit::WidgetWidth);
  }
}

// textbox functions
void DRaceGUI::on_dr_path_input_textChanged(const QString &arg1) {
  ui->command_output->setText(ch->make_entry(arg1, Command_Handler::DYNAMORIO));
  QPalette pal;
  pal.setColor(QPalette::Base, Qt::red);

  if (Executor::exe_drrun(
          arg1, this)) {  // if drrun is found set green, otherwise set red
    pal.setColor(QPalette::Base, Qt::green);
  }

  ui->dr_path_input->setPalette(pal);
}

void DRaceGUI::on_drace_path_input_textChanged(const QString &arg1) {
  ui->command_output->setText(
      ch->make_entry(arg1, Command_Handler::DRACE, "-c"));

  QPalette pal;
  pal.setColor(QPalette::Base, Qt::red);

  if (arg1.endsWith("drace-client.dll") && QFileInfo(arg1).exists()) {
    pal.setColor(QPalette::Base, Qt::green);

    if (ui->config_file_input->text()
            .isEmpty()) {  // try to set config file, when empty
      auto path_to_config = arg1;
      path_to_config.replace(QString("drace-client.dll"), QString("drace.ini"));
      ui->config_file_input->setText(path_to_config);
    }
  }

  ui->drace_path_input->setPalette(pal);
}

void DRaceGUI::on_exe_input_textChanged(const QString &arg1) {
  ui->command_output->setText(
      ch->make_entry(arg1, Command_Handler::EXECUTABLE, "--"));
}

void DRaceGUI::on_exe_args_input_textChanged(const QString &arg1) {
  ui->command_output->setText(
      ch->make_text_entry(arg1, Command_Handler::EXECUTABLE_ARGS));
}

void DRaceGUI::on_flag_input_textChanged(const QString &arg1) {
  ui->command_output->setText(ch->make_flag_entry(arg1));
}

void DRaceGUI::on_config_file_input_textChanged(const QString &arg1) {
  ui->command_output->setText(
      ch->make_entry(arg1, Command_Handler::CONFIGURATION, "-c"));

  QPalette pal;
  pal.setColor(QPalette::Base, Qt::red);

  std::ifstream config(arg1.toStdString());

  if (config.good()) {
    pal.setColor(QPalette::Base, Qt::green);
  }

  ui->config_file_input->setPalette(pal);
}

// action button functions
void DRaceGUI::on_actionAbout_triggered() {
  About_Dialog newWindow;
  newWindow.exec();
}

void DRaceGUI::on_actionLoad_Config_triggered() {
  QString temp_sel =
      "DRace Configuration File (*" + Options_Dialog::CONFIG_EXT + ")";
  const char *selected_filter = temp_sel.toStdString().c_str();
  QString selfilter = tr(selected_filter);
  QString temp_filter = "All files(*.*);; DRace Configuration File (*" +
                        Options_Dialog::CONFIG_EXT + ")";
  const char *filter = temp_filter.toStdString().c_str();
  QString path = QFileDialog::getOpenFileName(this, "Open File", load_save_path,
                                              tr(filter), &selfilter);
  if (path != "") {
    load_save_path = path;
    Load_Save ls(rh, ch, ph);
    if (ls.load(path.toStdString())) {
      set_boxes_after_load();  // restore all the data to the boxes
    } else {
      QMessageBox temp;
      temp.setIcon(QMessageBox::Warning);
      temp.setText("File is not valid!");
      temp.exec();
    }
  }
}

void DRaceGUI::on_actionSave_Config_triggered() {
  QString temp_sel =
      "DRace Configuration File (*" + Options_Dialog::CONFIG_EXT + ")";
  const char *selected_filter = temp_sel.toStdString().c_str();
  QString selfilter = tr(selected_filter);
  QString temp_filter = "All files(*.*);; DRace Configuration File (*" +
                        Options_Dialog::CONFIG_EXT + ")";
  const char *filter = temp_filter.toStdString().c_str();
  QString path = QFileDialog::getSaveFileName(this, "Save file", load_save_path,
                                              tr(filter), &selfilter);
  if (path != "") {
    load_save_path = path;
    Load_Save ls(rh, ch, ph);
    ls.save(path.toStdString());
  }
}

void DRaceGUI::on_actionConfigure_Report_triggered() {
  Report_Config *report_window;
  report_window = new Report_Config(rh);
  report_window->exec();
  ui->command_output->setText(ch->get_command());
}

void DRaceGUI::on_actionHelp_triggered() {
  QMessageBox temp;
  temp.setIcon(QMessageBox::Warning);
  temp.setText("This feature is not avaliable!");
  temp.exec();
}

void DRaceGUI::on_actionOptions_triggered() {
  Options_Dialog options_window;
  options_window.exec();
}

// text browser functions
void DRaceGUI::read_output() {
  QProcess *proc_drc = dynamic_cast<QProcess *>(sender());
  if (proc_drc) {
    ui->embedded_output->moveCursor(QTextCursor::End);
    ui->embedded_output->insertPlainText(proc_drc->readAllStandardOutput());
  }
}

// process functions
void DRaceGUI::on_proc_state_label_stateChanged(
    const QProcess::ProcessState state) {
  switch (state) {
    case QProcess::NotRunning:
      ui->proc_state_label->setStyleSheet(
          QString("border: 2px solid #a6a8a6 ; border-radius: 10px ; "
                  "background-color: #696969"));
      ui->embedded_input->setEnabled(false);
      ui->stop_button->setEnabled(false);
      ui->run_button->setEnabled(true);
      ui->run_button->setFocus();
      break;
    case QProcess::Running:
      ui->proc_state_label->setStyleSheet(
          QString("border: 2px solid #a6a8a6 ; border-radius: 10px ; "
                  "background-color: #00e100"));
      ui->embedded_input->setEnabled(true);
      ui->run_button->setEnabled(false);
      ui->stop_button->setEnabled(true);
      ui->stop_button->setFocus();
      break;
    case QProcess::Starting:
      ui->proc_state_label->setStyleSheet(
          QString("border: 2px solid #a6a8a6 ; border-radius: 10px ; "
                  "background-color: #f0ac00"));
      ui->embedded_input->setEnabled(false);
      ui->run_button->setEnabled(false);
      ui->stop_button->setEnabled(true);
      ui->stop_button->setFocus();
      break;
  }
}

void DRaceGUI::on_proc_crash_exit_triggered(int arg1) {
  QMessageBox temp;
  temp.setIcon(QMessageBox::Warning);
  if (arg1 == -2) {
    temp.setText("PowerShell process cannot be started.");
  } else if (arg1 == -1) {
    temp.setText("PowerShell process was interrupted.");
  } else {
    temp.setText("An unexpected error occured.");
  }
  temp.exec();
}

void DRaceGUI::on_proc_errorOccured(QProcess::ProcessError error) {
  QMessageBox temp;
  temp.setIcon(QMessageBox::Warning);
  switch (error) {
    case QProcess::FailedToStart:
      temp.setText(
          "Failed to start process. It might either be missing or you do not "
          "have the right permissions.");
      break;
    case QProcess::Crashed:
      temp.setText("Process was interrupted.");
      break;
    case QProcess::Timedout:
      temp.setText("The used synchronous function(s) timed out.");
      break;
    case QProcess::WriteError:
      temp.setText("Failed to write to process.");
      break;
    case QProcess::ReadError:
      temp.setText("Failed to read from process.");
      break;
    default:
      temp.setText("An unknown error has occured.");
      break;
  }
  temp.exec();
}

void DRaceGUI::on_embedded_input_returnPressed() {
  QString data = ui->embedded_input->text();
  ph->write(data);
  ui->embedded_output->moveCursor(QTextCursor::End);
  ui->embedded_output->insertPlainText(data + "\n");
  ui->embedded_input->clear();
}

QString DRaceGUI::remove_quotes(QString str) {
  if ((*(str.begin()) == "'" && *(str.begin()) == "'") ||
      (*(str.begin()) == "\"" && *(str.begin()) == "\"")) {
    str.remove(0, 1);
    str.remove(str.length() - 1, 1);
  }
  return str;
}

/// after a load all necessary data is restored in the handler classes
/// this functions puts the data bacak in the text and check boxes
void DRaceGUI::set_boxes_after_load() {
  // reset text boxes
  ui->dr_path_input->setText(
      remove_quotes(ch->command[Command_Handler::DYNAMORIO]));

  QString config =
      ch->command[Command_Handler::CONFIGURATION];  // should be set before
                                                    // drace_client, to not
                                                    // interfere with the
                                                    // automatic set of the
                                                    // config file
  config.remove(0, 3);  // config cmd is -c CONFIGFILE -> remove "-c "
  ui->config_file_input->setText(remove_quotes(config));

  QString drace_cl = ch->command[Command_Handler::DRACE];
  drace_cl.remove(0, 3);  // drace cmd is -c DRACECLIENT -> remove "-c "
  ui->drace_path_input->setText(remove_quotes(drace_cl));

  QString exe = ch->command[Command_Handler::EXECUTABLE];
  exe.remove(0, 3);  // exe cmd is -- EXECUTEABLE -> remove "-- "
  ui->exe_input->setText(remove_quotes(exe));

  ui->exe_args_input->setText(ch->command[Command_Handler::EXECUTABLE_ARGS]);

  ui->flag_input->setText(ch->command[Command_Handler::FLAGS]);

  // set debug check box
  if (ch->command[Command_Handler::DR_DEBUG] != "") {
    ui->dr_debug->setChecked(true);
    ui->dr_debug->setCheckState(Qt::Checked);
  } else {
    ui->dr_debug->setChecked(false);
    ui->dr_debug->setCheckState(Qt::Unchecked);
  }

  // set report check box
  if (rh->command_valid()) {
    ui->report_creation->setChecked(true);
    ui->report_creation->setCheckState(Qt::Checked);
  } else {
    ui->report_creation->setChecked(false);
    ui->report_creation->setCheckState(Qt::Unchecked);
  }

  // set msr check box
  if (ch->get_msr_path() != "") {
    ui->msr_box->setChecked(true);
    ui->msr_box->setCheckState(Qt::Checked);
  } else {
    ui->msr_box->setChecked(false);
    ui->msr_box->setCheckState(Qt::Unchecked);
  }

  // set exclude stack accesses check box
  if (ch->command[Command_Handler::EXCL_STACK] != "") {
    ui->excl_stack_box->setChecked(true);
    ui->excl_stack_box->setCheckState(Qt::Checked);
  } else {
    ui->excl_stack_box->setChecked(false);
    ui->excl_stack_box->setCheckState(Qt::Unchecked);
  }

  // set auto open report check box
  if (ch->command[Command_Handler::REPORT_OPEN_CMD] != "") {
    ui->report_auto_open_box->setChecked(true);
    ui->report_auto_open_box->setCheckState(Qt::Checked);
  } else {
    ui->report_auto_open_box->setChecked(false);
    ui->report_auto_open_box->setCheckState(Qt::Unchecked);
  }

  // set detector back end
  QString detector = ch->command[Command_Handler::DETECTOR];
  if (detector.contains("tsan", Qt::CaseInsensitive)) {
    ui->tsan_btn->setChecked(true);
  }
  if (detector.contains("dummy", Qt::CaseInsensitive)) {
    ui->dummy_btn->setChecked(true);
  }
  if (detector.contains("fasttrack", Qt::CaseInsensitive)) {
    ui->fasttrack_btn->setChecked(true);
  }

  // set misc options checkboxes
  const auto &options = ph->get_options_array();
  ui->run_separate_box->setChecked(
      (bool)options[Process_Handler::RUN_SEPARATELY]);
  ui->wrap_text_box->setChecked(
      (bool)options[Process_Handler::WRAP_TEXT_OUTPUT]);

  ui->command_output->setText(ch->make_command());
}
