#include "Engine.h"
#include <ios>
#include <iostream>
#include <chrono>


int main(int argc, char* argv[]) {
  std::ios_base::sync_with_stdio(false);  // Disable C/C++ sync
  std::cin.tie(nullptr);  // Untie cin from cout

  auto start = std::chrono::high_resolution_clock::now();

  Engine::Run();

  auto end = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double> elapsed = end - start;
  std::cout << "Run took: " << elapsed.count() << " seconds" << std::endl;
  return 0;
}
