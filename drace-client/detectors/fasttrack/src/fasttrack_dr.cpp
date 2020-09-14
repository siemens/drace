#include "fasttrack.h"
#include "fasttrack_dr_export.h"
#include "ipc/DrLock.h"

using FasttrackDR = drace::detector::Fasttrack<DrLock>;

extern "C" FASTTRACK_DR_EXPORT Detector* CreateDetector() {
  return new drace::detector::Fasttrack<DrLock>();  // NOLINT
}

extern "C" FASTTRACK_DR_EXPORT void Read_Nvrt(void* detector, void* tls,
                                              void* pc, void* addr,
                                              size_t size) {
  FasttrackDR* det = reinterpret_cast<FasttrackDR*>(detector);
  // det->clearVarState(1);
  det->read(tls, pc, addr, size);
}

extern "C" FASTTRACK_DR_EXPORT void Write_Nvrt(void* detector, void* tls,
                                               void* pc, void* addr,
                                               size_t size) {
  FasttrackDR* det = reinterpret_cast<FasttrackDR*>(detector);
  det->write(tls, pc, addr, size);
}

extern "C" FASTTRACK_DR_EXPORT void FuncEnter_Nvrt(void* detector, void* tls,
                                                   void* pc) {
  FasttrackDR* det = reinterpret_cast<FasttrackDR*>(detector);
  det->func_enter(tls, pc);
}

extern "C" FASTTRACK_DR_EXPORT void FuncExit_Nvrt(void* detector, void* tls) {
  FasttrackDR* det = reinterpret_cast<FasttrackDR*>(detector);
  det->func_exit(tls);
}
