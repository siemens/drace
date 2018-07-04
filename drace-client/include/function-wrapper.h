#pragma once

#include <dr_api.h>

namespace funwrap {
	bool init();
	void finalize();

	/* Wrap mutex aquire and release */
	void wrap_mutexes(const module_data_t *mod);
	/* Wrap heap alloc and free */
	void wrap_allocations(const module_data_t *mod);
	/* Wrap excluded functions */
	void wrap_excludes(const module_data_t *mod);
}