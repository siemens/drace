#pragma once

#include <detector/Detector.h>

// // forward decls
// class Detector;

/// DRace instrumentation framework
namespace drace {

extern decltype(Read_Nvrt) *Read_Nvrt_Ptr;
extern decltype(Write_Nvrt) *Write_Nvrt_Ptr;
// extern void (*Write_Nvrt_Ptr)(void *, void *, void *, void *, size_t);

extern void (*FuncEnter_Nvrt_Ptr)(void *, void *, void *);
extern void (*FuncExit_Nvrt_Ptr)(void *, void *);

}  // namespace drace