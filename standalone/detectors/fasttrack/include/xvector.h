#ifndef XVECTOR_H
#define XVECTOR_H
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

#include <vector>

template <class _Ty, class _Ax = std::allocator<_Ty>>
class xvector : public std::vector<_Ty, _Ax> {
  using std::vector<_Ty, _Ax>::vector;
};

#endif  //! XVECTOR_H
