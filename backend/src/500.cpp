#include "log.h"
#include "server.hpp"

void process_message(cards::Message m) {
  logging::debug() << "In the TEMPLATED worker!" << logging::endl;
  logging::message() << m.test() << logging::endl;
  logging::debug() << "after printing the msg" << logging::endl;
}
int main() {
  server::serve(process_message);
}
