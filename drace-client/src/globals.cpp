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

#include "globals.h"
#include "DrThreadLocal.h"
#include "Module.h"
#include "RuntimeConfig.h"
#include "ShadowThreadState.h"
#include "memory-tracker.h"
#include "symbols.h"

#if WIN32
#include "ipc/MtSyncSHMDriver.h"
#include "ipc/SharedMemory.h"
#endif

#include <detector/Detector.h>

namespace drace {

InstrumentationConfig config;
RuntimeConfig params;

std::atomic<unsigned> runtime_tid{0};

DrThreadLocal<ShadowThreadState> thread_state;
std::unique_ptr<MemoryTracker> memory_tracker;
std::unique_ptr<module::Tracker> module_tracker;

#if WIN32
std::unique_ptr<ipc::MtSyncSHMDriver<true, true>> shmdriver;
std::unique_ptr<ipc::SharedMemory<ipc::ClientCB, true>> extcb;
#endif

std::unique_ptr<Detector> detector;

decltype(Read_Nvrt) *Read_Nvrt_Ptr = nullptr;
void (*Write_Nvrt_Ptr)(void *, void *, void *, void *, size_t) = nullptr;
void (*FuncEnter_Nvrt_Ptr)(void *, void *, void *) = nullptr;
void (*FuncExit_Nvrt_Ptr)(void *, void *) = nullptr;
}  // namespace drace
