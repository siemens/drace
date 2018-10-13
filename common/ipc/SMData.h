#pragma once

#include <cstdint>
#include <string>
#include <array>
#include <atomic>

#define DRACE_SMR_NAME "drace-msr"
#define DRACE_SMR_CB_NAME "drace-cb"
#define DRACE_SMR_MAXLEN 1024

namespace ipc {

	/* Protocol Message IDs */
	enum class SMDataID : uint8_t {
		IP,
		SYMBOL,
		STACK,
		READY,
		CONNECT,
		ATTACH,
		ATTACHED,
		LOADSYMS,
		UNLOADSYMS,
		SEARCHSYMS,

		WAIT,
		CONFIRM,

		EXIT
	};

	/* Resolved Symbol Information */
	struct SymbolInfo {
		std::array<char, 128> module;
		std::array<char, 128> function;
		std::array<char, 256> path;
		std::array<char, 128>  line;
	};

	/* Basic Information for attaching */
	struct BaseInfo {
		// process id
		int pid;
		// Path to *clr.dll
		std::array<char,256> path;
	};

	/* Request symbols for this module */
	struct SymbolRequest {
		uint64_t base;
		size_t   size;
		bool     full{ false };
		std::array<char, 256> path;
		std::array<char, 256> match;
	};

	struct SymbolResponse {
		size_t size{0};
		std::array<uint64_t, 64> adresses;
	};

	struct MachineContext {
		int      threadid;
		uint64_t rbp;
		uint64_t rsp;
		uint64_t rip;
	};

	struct SMData {
		static constexpr unsigned BUFFER_SIZE = DRACE_SMR_MAXLEN - 16;

		// Message IDs
		SMDataID id;
		// Raw data buffer
		byte buffer[BUFFER_SIZE];
	};

	struct ClientCB {
		std::atomic<bool>     enabled{ true };
		std::atomic<uint32_t> sampling_rate;
	};

} // namespace ipc
