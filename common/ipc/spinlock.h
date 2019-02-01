#pragma once
/*
 * DRace, a dynamic data race detector
 *
 * Copyright (c) Siemens AG, 2018
 *
 * Authors:
 *   Felix Moessbauer <felix.moessbauer@siemens.com>
 *
 * This work is licensed under the terms of the MIT license.  See
 * the LICENSE file in the top-level directory.
 */

#include <atomic>
#include <thread>
#ifdef DEBUG
#include <iostream>
#endif

namespace ipc {
	/**
	* Simple mutex implemented as a spinlock
	* implements interface of std::mutex
	*/
	class spinlock {
		std::atomic_flag _flag = ATOMIC_FLAG_INIT;
	public:
		inline void lock() noexcept
		{
			unsigned cnt = 0;
			while (_flag.test_and_set(std::memory_order_acquire)) {
				if (++cnt == 100) {
					// congestion on the lock
					std::this_thread::yield();
#ifdef DEBUG
					std::cout << "spinlock congestion" << std::endl;
#endif
				}
			}
		}

		inline bool try_lock() noexcept
		{
			return !(_flag.test_and_set(std::memory_order_acquire));
		}

		inline void unlock() noexcept
		{
			_flag.clear(std::memory_order_release);
		}
	};

} // namespace ipc
