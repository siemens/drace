
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

#ifndef RACEFILTER_H
#define RACEFILTER_H

#include <vector>
#include <string>
#include <chrono>
#include <detector/Detector.h>
#include <race/DecoratedRace.h>

namespace drace{

    class  RaceFilter{
        std::vector<std::string> filter_list;
    
    public:
        RaceFilter(std::string filename);
        bool check_suppress(const drace::race::DecoratedRace & race);
        void print_list();

    };

}

#endif //RACEFILTER_H