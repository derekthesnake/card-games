/*
Representation of a game.

games will have their own threads.

follow producer/consumer model:
websockets will add "actions" to the game queue (thread safety!)
separate thread will consume the queue, updating state and broadcasting action to clients
- could need to send message to only the originator of the action on invalid

games need a way to communicate total state to the clients (think reconnecting)

*/
#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>
#include <memory>

#include <protos/message.pb.h>

struct Game {
private:
  // TODO: some sort of game state

  std::queue<std::shared_ptr<cards::Message>> msg_queue;
  std::mutex mutex;
  std::condition_variable notifier;
  std::function<void(cards::Message)> processor;

  std::function<void(cards::Message)> process_messages() {
    while(true) {
      std::unique_lock lk = std::unique_lock(mutex);
      notifier.wait(lk);
      std::cout << "queue size is " << msg_queue.size() << std::endl;
      std::shared_ptr<cards::Message> m = msg_queue.front();
      std::cout << "m addr in process_msgs: " << &*m << std::endl;
      std::cout << "in loop: m.test.length = " << m->test().length() << std::endl;
      processor(*m);
      msg_queue.pop();
    }
  }
  
public:
Game(void (* fn)(cards::Message)) : processor(fn) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
    std::thread worker = std::thread([this] {
	process_messages();
      });
    worker.detach();
  }
  // is any work in the destructor necessary for the thread? not sure
  //~Game()
  void add_message(std::shared_ptr<cards::Message> m) {
    std::cout << "game queue add rcvd " << &*m << std::endl;
    std::cout << "in GQ: m.test = " << m->test() << std::endl;
    std::unique_lock lk = std::unique_lock(mutex);
    msg_queue.push(m);
    std::cout << "added m to queue" << std::endl;
    lk.unlock();
    notifier.notify_one();
  }  
};
