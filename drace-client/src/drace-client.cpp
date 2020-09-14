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

/**
 \brief DynamoRIO client for Race-Detection
 */

#include <dr_api.h>
#include <drmgr.h>
#include <drreg.h>
#include <drutil.h>
#include <drwrap.h>

#include <atomic>
#include <fstream>
#include <memory>
#include <string>

#include "DrFile.h"
#include "DrThreadLocal.h"
#include "Module.h"
#include "drace-client.h"
#include "function-wrapper.h"
#include "globals.h"
#include "memory-tracker.h"
#include "race-collector.h"
#include "race-filter.h"
#include "shadow-stack.h"
#include "statistics.h"
#include "symbols.h"
#include "util/DrModuleLoader.h"
#include "util/LibLoaderFactory.h"

#include "sink/hr-text.h"
#ifdef DRACE_XML_EXPORTER
#include "sink/valkyrie.h"
#endif
#ifdef WINDOWS
#include "MSR.h"
#endif

#include <clipp.h>
#include <detector/Detector.h>

std::unique_ptr<::util::LibraryLoader> detector_loader;
std::shared_ptr<drace::DrFile> log_file;
// Start time of the application
std::chrono::system_clock::time_point app_start;
std::chrono::system_clock::time_point app_stop;
std::unique_ptr<drace::RaceCollector> race_collector;
std::shared_ptr<drace::Statistics> stats;

/// DRace main entry point
DR_EXPORT void dr_client_main(client_id_t id, int argc, const char *argv[]) {
  using namespace drace;

  dr_set_client_name("Race-Detection Tool 'DRace'",
                     "https://github.com/siemens/drace/issues");

#ifdef WINDOWS
  dr_enable_console_printing();
#endif

  // Parse runtime arguments and print generated configuration
  params.parse_args(argc, argv);
  // setup logging (IO) target
  log_file = std::make_shared<DrFile>(params.logfile, DR_FILE_WRITE_OVERWRITE);
  if (log_file->good()) {
    log_target = log_file->get();
  }

  params.print_config();

  // time app start
  app_start = std::chrono::system_clock::now();

  bool success = config.loadfile(params.config_file, argv[0]);
  if (!success) {
    LOG_ERROR(-1, "error loading config file");
    dr_flush_file(drace::log_target);
    dr_abort_with_code(1);
  }
  LOG_NOTICE(-1, "size of ShadowThreadState %i bytes",
             sizeof(ShadowThreadState));

  // Init DRMGR, Reserve registers
  if (!drmgr_init() || !drutil_init()) DR_ASSERT(false);

  // Register Events
  dr_register_exit_event(event_exit);
  drmgr_register_thread_init_event(event_thread_init);
  drmgr_register_thread_exit_event(event_thread_exit);

  // Setup Function Wrapper
  DR_ASSERT(funwrap::init());

  // Setup Statistics Collector
  stats = std::make_unique<Statistics>(0);
  auto symbol_table = std::make_shared<symbol::Symbols>();
  // Setup Module Tracking
  module_tracker = std::make_unique<drace::module::Tracker>(symbol_table);
  // Setup Memory Tracing
  memory_tracker = std::make_unique<MemoryTracker>(stats);

  std::shared_ptr<RaceFilter> filter =
      std::make_shared<RaceFilter>(params.filter_file, argv[0]);
  // Setup Race Collector and bind lookup function
  race_collector = std::make_unique<RaceCollector>(params.delayed_sym_lookup,
                                                   symbol_table, filter);
  register_report_sinks();

  // Initialize Detector
  register_detector(argc, argv, params.detector);

  LOG_INFO(-1, "application pid: %i", dr_get_process_id());

  // \todo port to linux
  // if we try to access a non-existing SHM,
  // DR will spuriously fail some time later
  if (params.extctrl) {
#if WINDOWS
    if (!MSR::connect()) {
      LOG_ERROR(-1, "MSR not available (required for --extctrl)");
      dr_abort();
    }
#else
    LOG_ERROR(-1, "--extctrl is not supported on linux yet.");
    dr_abort();
#endif
  }
}

namespace drace {

static void register_detector(int argc, const char **argv,
                              const std::string &detector_name) {
  decltype(CreateDetector) *create_detector = nullptr;
  // decltype(Read_Nvrt) *Read_Nvrt_Ptr = nullptr;
  std::string detector_lib(::util::LibLoaderFactory::getModulePrefix() +
                           "drace.detector." + detector_name +
                           ::util::LibLoaderFactory::getModuleExtension());

  /*
   * i#59 TSAN uses TLS which prohibits late-loading of the library.
   * Hence, we load TSAN early, but do not initialize it except if
   * the detector is requested (using -d tsan)
   */
  if (util::is_unix() || detector_name.compare("tsan") != 0) {
    detector_loader =
        std::make_unique<::drace::util::DrModuleLoader>(detector_lib.c_str());
    if (!detector_loader->loaded()) {
      LOG_ERROR(0, "could not load library %s", detector_lib.c_str());
      dr_abort();
    }
    create_detector = (*detector_loader)["CreateDetector"];
    Read_Nvrt_Ptr = (*detector_loader)["Read_Nvrt"];
  } else {
    // tsan is loaded during startup, hence symbol is available
    create_detector = CreateDetector;
    // Read_Nvrt_Ptr = Read_Nvrt;
  }

  detector = std::unique_ptr<Detector>(create_detector());
  detector->init(argc, argv, RaceCollector::race_collector_add_race);
  LOG_INFO(0, "Detector %s initialized", detector_lib.c_str());
}

static void register_report_sinks() {
  if (drace::log_target != 0) {
    // register the console printer
    race_collector->register_sink(std::make_shared<sink::HRText>(log_file));
  }

  // when using direct lookup, we stream the races
  if (!params.delayed_sym_lookup) {
#ifdef DRACE_XML_EXPORTER
    if (params.xml_file != "") {
      auto target =
          std::make_shared<DrFile>(params.xml_file, DR_FILE_WRITE_OVERWRITE);
      if (target->good()) {
        race_collector->register_sink(std::make_shared<sink::Valkyrie>(
            target, params.argc, params.argv, dr_get_application_name(),
            app_start, app_stop));
        LOG_NOTICE(0, "Stream data-races to %s in XML format",
                   params.xml_file.c_str());
      } else {
        LOG_WARN(0, "Cannot create XML-report file");
      }
    }
#endif
    if (params.out_file != "") {
      auto target =
          std::make_shared<DrFile>(params.out_file, DR_FILE_WRITE_OVERWRITE);
      if (target->good()) {
        race_collector->register_sink(std::make_shared<sink::HRText>(target));
        LOG_NOTICE(0, "Stream data-races to %s in text format",
                   params.out_file.c_str());
      } else {
        LOG_WARN(0, "Cannot create text-report file");
      }
    }
  }
}

// Events
static void event_exit() {
  app_stop = std::chrono::system_clock::now();

  if (!drmgr_unregister_thread_init_event(event_thread_init) ||
      !drmgr_unregister_thread_exit_event(event_thread_exit))
    DR_ASSERT(false);

  // Generate summary while information is still present
  generate_summary();

  if (params.stats_show) {
    // TODO: workaround for i#9. After print_summary is fixed, remove this guard
    stats->print_summary(log_target);
  }

  LOG_INFO(-1, "found %i possible data-races", race_collector->num_races());

  // Cleanup all drace modules
  module_tracker.reset();
  memory_tracker.reset();
  race_collector.reset();
  stats.reset();

  funwrap::finalize();

  // Finalize Detector
  detector->finalize();
  detector.reset();

  detector_loader.reset();

  // call destructor prior to DR shutdown
  // as internally, drmgr is used
  thread_state.~DrThreadLocal();

  // Cleanup DR extensions
  drutil_exit();
  drmgr_exit();

  LOG_INFO(-1, "DRace exit");

  log_file.reset();
}

static void event_thread_init(void *drcontext) {
  thread_id_t tid = dr_get_thread_id(drcontext);
  // assume that the first event comes from the runtime thread
  unsigned empty_tid = 0;
  if (runtime_tid.compare_exchange_weak(empty_tid, (unsigned)tid,
                                        std::memory_order_relaxed)) {
    LOG_INFO(tid, "Runtime Thread tagged");
  }
  ShadowThreadState &data = thread_state.addSlot(drcontext, drcontext);

  memory_tracker->event_thread_init(drcontext, data);
  LOG_INFO(tid, "Thread started");
}

static void event_thread_exit(void *drcontext) {
  ShadowThreadState &data = thread_state.getSlot(drcontext);
  memory_tracker->event_thread_exit(drcontext, data);

  // deconstruct thread state
  thread_state.removeSlot(drcontext);
  LOG_INFO(dr_get_thread_id(drcontext), "Thread exited");
}

static void generate_summary() {
  // with delayed lookup we cannot stream, hence we write
  // the report here
  if (params.delayed_sym_lookup) {
    race_collector->resolve_all();

    if (params.out_file != "") {
      auto race_text_report =
          std::make_shared<DrFile>(params.out_file, DR_FILE_WRITE_OVERWRITE);
      if (race_text_report->good()) {
        sink::HRText hr_sink(race_text_report);
        hr_sink.process_all(race_collector->get_races());
      } else {
        LOG_ERROR(-1, "Could not open race-report file: %c",
                  params.out_file.c_str());
      }
    }
#ifdef DRACE_XML_EXPORTER
    // Write XML output
    if (params.xml_file != "") {
      auto race_xml_report =
          std::make_shared<DrFile>(params.xml_file, DR_FILE_WRITE_OVERWRITE);
      if (race_xml_report->good()) {
        sink::Valkyrie v_sink(race_xml_report, params.argc, params.argv,
                              dr_get_application_name(), app_start, app_stop);
        v_sink.process_all(race_collector->get_races());
      }
    }
#endif
  }
}
}  // namespace drace
