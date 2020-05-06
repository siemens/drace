#include "prof.h"

#include <easy/profiler.h>
#include <random>

FastTrackProf::FastTrackProf() {}

FastTrackProf::~FastTrackProf() {}

void add_remove_stacktrace() {
  EASY_FUNCTION(profiler::colors::Magenta);
  {
    ProfTimer prof_timer;

    StackTrace st;
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(0, 9);
    
    for (int i = 0; i < 1000; ++i) {
      EASY_BLOCK("adding elements")
      for (int j = 0; j < 20; ++j) {
        st.push_stack_element(j + i);
        st.set_read_write(j, j * 100);
      }
      EASY_END_BLOCK;

      EASY_BLOCK("popping out elements")
      for (int m = 0; m < 20; m++) {
        st.pop_stack_element();
      }
      EASY_END_BLOCK;

      int addr = int(dist(mt));
      st.return_stack_trace(addr);
    }
  }
}

int main() {
  // std::cout << "vec[4]= " << vec[5] << std::endl;

  // std::cout << "vec.size() = " << vec.size() << std::endl;

  EASY_PROFILER_ENABLE;

  EASY_MAIN_THREAD;  // Give a name to main thread, same as EASY_THREAD("Main")

  add_remove_stacktrace();

  profiler::dumpBlocksToFile("fasttrack_test.prof");

  std::cout << "Did it work again_v4??" << std::endl;
  int x;
  std::cin >> x;

  return 0;
}
