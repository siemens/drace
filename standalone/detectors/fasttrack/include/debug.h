#ifndef DEBUG_HEADER_H
#define DEBUG_HEADER_H
#pragma once

#include <iostream>
#include <iomanip>

#define DEBUG_INFO false

#define deb(x) std::cout << #x << " = " << std::setw(3) << std::dec << x << " "
#define deb_hex(x) \
  std::cout << #x << " = 0x" << std::hex << x << std::dec << " "
#define deb_long(x) \
  std::cout << std::setw(50) << #x << " = " << std::setw(12) << x << " "
#define deb_short(x) \
  std::cout << std::setw(25) << #x << " = " << std::setw(5) << x << " "
#define newline() std::cout << std::endl

#endif // !DEBUG_HEADER_H
