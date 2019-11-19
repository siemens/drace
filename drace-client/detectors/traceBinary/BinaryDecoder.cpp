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

#include <iostream>
#include <fstream>

#include <ipc/ExtsanData.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>

using namespace ipc::event;

int main(int argc, char** argv){
    auto logger = spdlog::stdout_logger_st("console");
	logger->set_level(spdlog::level::debug);

    std::fstream file("trace.bin", std::ios::in | std::ios::binary);
    auto buf = std::make_unique<BufferEntry>();

    while(file.read(reinterpret_cast<char*>(buf.get()),sizeof(BufferEntry)).good()){
        switch(buf->type){
            case Type::FUNCENTER:
                logger->debug("funcenter, call {}", buf->payload.funcenter.pc);
                break;
            case Type::FUNCEXIT:
                logger->debug("funcexit");
                break;
            case Type::MEMREAD:
                logger->debug("memread, pc {}", buf->payload.memaccess.pc);
                break;
            case Type::MEMWRITE:
                logger->debug("memwrite, pc {}", buf->payload.memaccess.pc);
                break;
            case Type::ACQUIRE:
                logger->debug("aquire, addr {}", buf->payload.mutex.addr);
                break;
        }
    }

}
