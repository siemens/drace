#include "prof.h"
#include <random>
#include <fasttrack.h>

#define deb(x) std::cout << #x << " = " << std::setw(3) << x << " "


struct VC_ID {
  uint32_t TID : 10, Clock : 22;
};

int main() {

  {
    //ProfTimer timer;
  }

  VC_ID test;

  test.TID = 10;
  test.Clock = 4194305;

  deb(sizeof(test));
  deb(test.TID);
  deb(test.Clock);


  std::cin.get();
  return 0;
}
