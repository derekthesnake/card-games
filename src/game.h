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

#include <protos/message.pb.h>

struct Game {
private:
  // TODO: some sort of game state

  std::queue<cards::Message> msg_queue;
  std::mutex mutex;
  std::condition_variable notifier;
  std::function<void(cards::Message)> processor;

  std::function<void(cards::Message)> process_messages() {
    while(true) {
      std::unique_lock lk = std::unique_lock(mutex);
      notifier.wait(lk);
      processor(msg_queue.front());
      msg_queue.pop();
    }
  }
  
public:
Game(void (* fn)(cards::Message)) : processor(fn) {
    std::thread worker = std::thread([this] {
	process_messages();
      });
    worker.detach();
  }
  // is any work in the destructor necessary for the thread? not sure
  //~Game()
  void add_message(cards::Message m) {
    std::unique_lock lk = std::unique_lock(mutex);
    msg_queue.push(m);
    std::cout << "added m to queue" << std::endl;
    lk.unlock();
    notifier.notify_one();
  }  
};
