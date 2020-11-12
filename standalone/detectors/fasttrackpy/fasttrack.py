#!/usr/bin/env python
# DRace, a dynamic data race detector
#
# Copyright 2020 Siemens AG
#
# Authors:
#   Felix Moessbauer <felix.moessbauer@siemens.com>
#
# SPDX-License-Identifier: MIT

import fasttrackpy

def on_race(a1, a2):
  print('Detector found a race:')
  print('  a1: tid: {}'.format(a1.thread_id))
  print('  a2: tid: {}'.format(a2.thread_id))

d = fasttrackpy.Detector('fasttrack')
d.init('', on_race)
t1 = d.fork(1,2)
t2 = d.fork(1,3)
# do some stuff
d.func_enter(t1, 42)
d.func_exit(t1)

# enforce race
d.write(t1, 0xDEAD, 0x42, 0)
d.write(t2, 0xBEEF, 0x42, 0)
d.finalize()
print('finished execution')
