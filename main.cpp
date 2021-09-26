#include "atom.hpp"
#include "logger/logger.hpp"
#include <exception>

using namespace stx_logger;
using namespace stx_converter;

int main() {
  try {
    atom_server server;
    // server.run();
  } catch (const std::exception &e) {
    err() << e.what();
  } catch (...) {
    err() << " {!} ERROR  unexpected error. ";
  }
  return 0;
}
