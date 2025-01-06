#include "log.h"
#include "server.hpp"
#include "game.h"

class FiveHundred : public Game<cards::Message> {
  void process_message(cards::Message m) override {
    logging::debug() << "In the TEMPLATED worker!" << logging::endl;
    logging::message() << m.test() << logging::endl;
    logging::debug() << "after printing the msg" << logging::endl;
  }
};
int main() {
  server::serve<FiveHundred, cards::Message>();
}
