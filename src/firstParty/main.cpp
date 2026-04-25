#include "Engine.h"
#include <ios>
#include <iostream>
#include <chrono>


int main(int argc, char* argv[]) {
  std::ios_base::sync_with_stdio(false);  // Disable C/C++ sync
  std::cin.tie(nullptr);  // Untie cin from cout
  Engine::Run();
  return 0;
}
