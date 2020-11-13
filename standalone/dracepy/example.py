#!/usr/bin/env python
# DRace, a dynamic data race detector
#
# Copyright 2020 Siemens AG
#
# Authors:
#   Felix Moessbauer <felix.moessbauer@siemens.com>
#
# SPDX-License-Identifier: MIT

# This snippet shows how to use the python bindings
# of DRace compatible race detector libraries

import dracepy

def on_race(a1, a2):
  print('Detector found a race:')
  print('  a1: tid: {}'.format(a1.thread_id))
  print('  a1: callstack {}'.format(a1.callstack))
  print('  a2: tid: {}'.format(a2.thread_id))
  print('  a2: callstack {}'.format(a2.callstack))

# load detector from library in given path (second argument)
# this argument can be omitted. Then, default search order applies.
d = dracepy.Detector('drace.detector.fasttrack.standalone', '..')
print('Detector: {}, version {}'.format(d.name(), d.version()))
d.init([], on_race)
t1 = d.fork(1,2)
t2 = d.fork(1,3)
t1.func_enter(42)
t1.write(0xDEAD, 0x42, 0)
t1.func_exit()
# enforce a race
t2.write(0xBEEF, 0x42, 0)
d.finalize()
print('finished execution')
