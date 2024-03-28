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

#include <log.h>
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
      logging::debug() << "testing debug" << std::endl;
      logging::debug() << "queue size is " << msg_queue.size() << std::endl;
      std::shared_ptr<cards::Message> m = msg_queue.front();
      //      logging::debug() << "m addr in process_msgs: " << std::string(&m) << std::endl;
      logging::debug() << "in loop: m.test.length = " << m->test().length() << std::endl;
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
    logging::debug() << "game queue add received message addr " << &*m << std::endl;
    logging::debug() << "m.test = " << m->test() << std::endl;
    std::unique_lock lk = std::unique_lock(mutex);
    logging::debug() << "acquired the queue lock" << std::endl;
    msg_queue.push(m);
    logging::debug() << "added m to queue" << std::endl;
    lk.unlock();
    logging::debug() << "unlocked lock, notifying waiters" << std::endl;
    notifier.notify_one();
  }  
};
