#include "protos/message.pb.h"

int main() {
  cards::Message m;
  m.set_test("this is a big ol test");
  std:: string s;
  m.SerializeToString(&s);
  std::cout << s << std::endl;
}
