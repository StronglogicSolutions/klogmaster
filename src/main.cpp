#include "server.hpp"

int main(int argc, char** argv)
{
  kiq::log::klogger::init("klogmaster", "trace");
  server s{6933};
  s.run();
  return 0;
}