#include <chrono>
#include <iostream>

class ProfTimer {
 public:
  ProfTimer();
  ~ProfTimer();

  void Stop();

 private:
  std::chrono::time_point<std::chrono::high_resolution_clock> m_ProfStartPoint;
};

ProfTimer::ProfTimer() {
  m_ProfStartPoint = std::chrono::high_resolution_clock::now();
}

ProfTimer::~ProfTimer() { Stop(); }

void ProfTimer::Stop() {
  auto ProfEndPoint = std::chrono::high_resolution_clock::now();

  auto Start =
      std::chrono::time_point_cast<std::chrono::microseconds>(m_ProfStartPoint).time_since_epoch().count();
  auto End =
      std::chrono::time_point_cast<std::chrono::microseconds>(ProfEndPoint)
          .time_since_epoch().count();

  auto time = End - Start;
  double time_ms = time * 0.001;

  std::cout << time << "us (" << time_ms << "ms);" << std::endl;
}
