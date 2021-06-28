#include "fasttrack.h"

namespace drace {
namespace detector {

template <class LockT>
class FasttrackWrapper : public Fasttrack<LockT> {
 public:
  explicit FasttrackWrapper(bool log = true) { log_flag = log; };

  log_counters& getData() { return log_count; }

  void clear_var_state_helper(std::size_t addr) { clear_var_state(addr); }
};

}  // namespace detector
}  // namespace drace