#include "fasttrack.h"
#include "fasttrack_dr_export.h"
#include "ipc/DrLock.h"

extern "C" FASTTRACK_DR_EXPORT Detector* CreateDetector() {
  return new drace::detector::Fasttrack<DrLock>();  // NOLINT
}

/*---------------------------------------------------------------------
I still need to access data which belongs to the detector instance
+ I don't need to pass the detector to the function
I can make the objects public, OK, although not the best practice, but I 
would still require the actual Fasttrack object

I cannot overload the function with say drace::detector::Fasttrack<DrLock> 
instead of Detector due to name mangling

maybe use dynamic cast? --> might be slow 

And I plan to use the same strategy for read(), write(), func_enter(), func_exit()
---------------------------------------------------------------------*/
extern "C" FASTTRACK_DR_EXPORT void Read_Nvrt(Detector& detector, void* tls,
                                              void* pc, void* addr,
                                              size_t size) {

  //detector.clearVarState(1); doesn't work, because it is not a FT instance
  detector.read(tls, pc, addr, size);
}

extern "C" FASTTRACK_DR_EXPORT void Write_Nvrt(Detector& detector, void* tls,
                                              void* pc, void* addr,
                                              size_t size) {
                                                
}

extern "C" FASTTRACK_DR_EXPORT void FuncEnter_Nvrt(Detector& detector, void* tls,
                                               void* pc, void* addr,
                                               size_t size) {

}

extern "C" FASTTRACK_DR_EXPORT void FuncExit_Nvrt(Detector& detector,
                                                   void* tls, void* pc,
                                                   void* addr, size_t size) {

}
