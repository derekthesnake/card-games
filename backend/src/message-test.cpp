#include <iostream>
#include <fstream>
#include <string>

#include "protos/message.pb.h"

int main(int argc, char* argv[]) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  if (argc != 3) {
    std::cerr << "Usage:  " << argv[0] << " <output file> <test string>" << std::endl;
    return -1;
  }

  cards::Message m;
  m.set_test(argv[2]);
  std::fstream output(argv[1], std::ios::out | std::ios::trunc | std::ios::binary);
  if (!m.SerializeToOstream(&output)) {
    std::cerr << "Failed to write address book." << std::endl;
    return -1;
  }
}
