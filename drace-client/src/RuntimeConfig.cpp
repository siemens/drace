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

#include "RuntimeConfig.h"
#include "DrFile.h"
#include "globals.h"
#include "shadow-stack.h"
#include "version/version.h"

#include <clipp.h>
#include <dr_api.h>

namespace drace {

void RuntimeConfig::print_config() const {
  dr_fprintf(log_target,
             "< Runtime Configuration:\n"
             "< Race Detector:\t%s\n"
             "< Sampling Rate:\t%i\n"
             "< Instr. Rate:\t\t%i\n"
             "< Lossy:\t\t%s\n"
             "< Lossy-Flush:\t\t%s\n"
             "< Exclude Traces:\t%s\n"
             "< Exclude Stack:\t%s\n"
             "< Exclude Master:\t%s\n"
             "< Annotation Sup.:\t%s\n"
             "< Delayed Sym Lookup:\t%s\n"
             "< Config File:\t\t%s\n"
             "< Output File:\t\t%s\n"
             "< XML File:\t\t%s\n"
             "< Stack-Size:\t\t%i\n"
             "< External Ctrl:\t%s\n"
             "< Log Target:\t\t%s\n"
             "< Private Caches:\t%s\n",
             detector.c_str(), sampling_rate, instr_rate, lossy ? "ON" : "OFF",
             lossy_flush ? "ON" : "OFF", excl_traces ? "ON" : "OFF",
             excl_stack ? "ON" : "OFF", exclude_master ? "ON" : "OFF",
             annotations ? "ON" : "OFF", delayed_sym_lookup ? "ON" : "OFF",
             config_file.c_str(), out_file != "" ? out_file.c_str() : "OFF",
#ifdef DRACE_XML_EXPORTER
             xml_file != "" ? xml_file.c_str() : "OFF",
#else
             "Unsupported (build without XML exporter)",
#endif
             stack_size, extctrl ? "ON" : "OFF", logfile.c_str(),
             dr_using_all_private_caches() ? "ON" : "OFF");
}

void RuntimeConfig::parse_args(int argc_, const char **argv_) {
  argc = argc_;
  argv = argv_;

  bool display_help = false;
  auto drace_cli = clipp::group(
      (clipp::option("-c", "--config") & clipp::value("config", config_file)) %
          ("config file (default: " + config_file + ")"),
      (clipp::option("-d", "--detector") & clipp::value("detector", detector) &
       clipp::opt_values("detector-options", detector_options)) %
          ("race detector (default: " + detector + ")"),
      ((clipp::option("-s", "--sample-rate") &
        clipp::integer("sample-rate", sampling_rate)) %
           "sample each nth instruction (default: no sampling)",
       (clipp::option("-i", "--instr-rate") &
        clipp::integer("instr-rate", instr_rate)) %
           "instrument each nth instruction (default: no sampling, 0: no "
           "instrumentation)") %
          "sampling options",
      ((clipp::option("--lossy").set(lossy) %
        "dynamically exclude fragments using lossy counting") &
           (clipp::option("--lossy-flush").set(lossy_flush) %
            "de-instrument flushed segments (only with --lossy)"),
       clipp::option("--excl-traces").set(excl_traces) %
           "exclude dynamorio traces",
       clipp::option("--excl-stack").set(excl_stack) % "exclude stack accesses",
       clipp::option("--excl-master").set(exclude_master) %
           "exclude first thread") %
          "analysis scope",
      (clipp::option("--stacksz") & clipp::integer("stacksz", stack_size)) %
          ("size of callstack used for race-detection (must be in [1," +
           std::to_string(ShadowStack::maxSize()) +
           "], default: " + std::to_string(stack_size) + ")"),
      clipp::option("--no-annotations").set(annotations, false) %
          "disable code annotation support",
      clipp::option("--delay-syms").set(delayed_sym_lookup) %
          "perform symbol lookup after application shutdown",
      (clipp::option("--suplevel") &
       clipp::integer("level", suppression_level)) %
          "suppress similar races (0=detector-default, 1=unique "
          "top-of-callstack entry, default: 1)",
      (clipp::option("--sup-races") & clipp::value("sup-races", filter_file)) %
          ("race suppression file (default: " + filter_file + ")"),
      (
#ifdef DRACE_XML_EXPORTER
          (clipp::option("--xml-file", "-x") &
           clipp::value("filename", xml_file)) %
              "log races in valkyries xml format in this file",
#endif
          (clipp::option("--out-file", "-o") &
           clipp::value("filename", out_file)) %
              "log races in human readable format in this file") %
          "data race reporting",
      (clipp::option("--logfile", "-l") & clipp::value("filename", logfile)) %
          "write all logs to this file (can be null, stdout, stderr, or "
          "filename)",
      clipp::option("--extctrl").set(extctrl) %
          "use second process for symbol lookup and state-controlling "
          "(required for Dotnet)",
      // for testing reasons only. Abort execution after the first race was
      // detected
      clipp::option("--brkonrace").set(break_on_race) %
          "abort execution after first race is found (for testing purpose "
          "only)",
      clipp::option("--stats").set(stats_show) %
          "display per-thread statistics on thread-exit",
      (clipp::option("--version")([]() {
         dr_printf(
             "DRace, a dynamic data race detector\nVersion: %s\nHash: %s\n",
             DRACE_BUILD_VERSION, DRACE_BUILD_HASH);
         dr_abort();
       }) %
       "display version information"),
      clipp::option("-h", "--usage").set(display_help) % "display help");
  auto detector_cli = clipp::group(
      // we just name the options here to provide a well-defined cli.
      // The detector-specific options are parsed from argv by the detector
      // itself
      clipp::option("--heap-only") %
      "only analyze heap memory (not supported currently)");
  auto cli =
      ((drace_cli % "DRace Options"), (detector_cli % ("Detector Options")));

  if (!clipp::parse(argc, const_cast<char **>(argv), cli) || display_help) {
    std::stringstream out;
    out << clipp::make_man_page(cli, util::basename(argv[0])) << std::endl;
    std::string outstr = out.str();
    // dr_fprintf does not print anything if input buffer
    // is too large hence, chunk it
    const auto chunks = util::split(outstr, "\n");
    for (const auto &chunk : chunks) {
      dr_write_file(STDERR, chunk.c_str(), chunk.size());
      dr_write_file(STDERR, "\n", 1);
    }
    dr_abort();
  }
}

}  // namespace drace
