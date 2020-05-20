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

#include <dr_api.h>
#include <drsyms.h>
#include <cstring>

#include "symbol/SymbolLocation.h"

namespace drace {
namespace symbol {

std::string SymbolLocation::get_pretty() const {
  // we use c-style formatting here, as we cannot
  // use std::stringstream (see i#9)
  constexpr int bufsize = 512;
  char* strbuf = (char*)dr_thread_alloc(dr_get_current_drcontext(), bufsize);

  dr_snprintf(strbuf, bufsize, "%p ", pc);
  if (nullptr != mod_base) {
    size_t len = strlen(strbuf);
    dr_snprintf(strbuf + len, bufsize - len, "(rel: %#010x)\n",
                (void*)(pc - mod_base));
  } else {
    size_t len = strlen(strbuf);
    dr_snprintf(strbuf + len, bufsize - len, "(dynamic code)\n");
  }
  if (mod_name != "") {
    size_t len = strlen(strbuf);
    dr_snprintf(strbuf + len, bufsize - len, "\tModule %s\n", mod_name.c_str());

    if (sym_name != "") {
      size_t len_name = strlen(strbuf);
      dr_snprintf(strbuf + len_name, bufsize - len_name, "\tSymbol %s\n",
                  sym_name.c_str());
    }

    if (file != "") {
      size_t len_file = strlen(strbuf);
      dr_snprintf(strbuf + len_file, bufsize - len_file, "\tFile %s:%i + %i\n",
                  file.c_str(), (int)line, (int)line_offs);
    }
  }
  auto str = std::string(strbuf);
  dr_thread_free(dr_get_current_drcontext(), strbuf, bufsize);
  return str;
}

}  // namespace symbol
}  // namespace drace
