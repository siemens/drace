#include <shared_mutex>
#include "fasttrack.h"
#include "fasttrack_st_export.h"

#include <iostream>

extern "C" FASTTRACK_ST_EXPORT Detector* CreateDetector() {
  return new drace::detector::Fasttrack<std::shared_mutex>();  // NOLINT
}

// extern "C" FASTTRACK_ST_EXPORT void Read_Nvrt(Detector& detector, void* tls,
//                                               void* pc, void* addr,
//                                               size_t size) {
//   detector.read(tls, pc, addr, size);
// }