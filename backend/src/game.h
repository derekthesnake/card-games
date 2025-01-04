/*
Representation of a game.

games will have their own threads.

follow producer/consumer model:
websockets will add "actions" to the game queue (thread safety!)
separate thread will consume the queue, updating state and broadcasting action to clients
- could need to send message to only the originator of the action on invalid

games need a way to communicate total state to the clients (think reconnecting)

*/

#ifndef cards_game
#define cards_game

#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>
#include <memory>

#include <log.h>


// Considering the possibility of moving to a priority queue based on message timestamp.

template <typename MessageType>
class Game {
private:
  // TODO: some sort of game state

  std::queue<std::shared_ptr<MessageType>> msg_queue;
  std::mutex mutex;
  std::condition_variable notifier;
  std::function<void(MessageType)> processor;

  //std::function<void(MessageType)>
  void worker_loop() {
    while(true) {
      std::shared_ptr<MessageType> m;
      {
	std::unique_lock lk = std::unique_lock(mutex);
	notifier.wait(lk);
	logging::debug() << "testing debug" << logging::endl;
	logging::debug() << "queue size is " << msg_queue.size() << logging::endl;
	m = msg_queue.front();
	msg_queue.pop();
	logging::debug() << "in loop: m.test.length = " << m->test().length() << logging::endl;
	logging::debug() << "in loop: m.test = " << m->test() << logging::endl;
      }
      logging::debug() << "outside the lock context, lock should be released" << logging::endl;
      logging::debug() << "calling processor function" << logging::endl;
      process_message(*m);
      logging::debug() << "done with processor function, back in the loop" << logging::endl;
    }
  }
  
public:
  Game() {
    //GOOGLE_PROTOBUF_VERIFY_VERSION;
    std::thread worker = std::thread([this] {
	worker_loop();
      });
    worker.detach();
  }
  void process_message(MessageType m) {
    logging::message() << m.DebugString() << logging::endl;
  }
  // is any work in the destructor necessary for the thread? not sure
  //~Game()
  void add_message(std::shared_ptr<MessageType> m) {
    logging::debug() << "game queue add received message addr " << &*m << logging::endl;
    logging::debug() << "m.test = " << m->test() << logging::endl;
    std::unique_lock lk = std::unique_lock(mutex);
    logging::debug() << "acquired the queue lock" << logging::endl;
    msg_queue.push(m);
    logging::debug() << "added m to queue" << logging::endl;
    lk.unlock();
    logging::debug() << "unlocked lock, notifying waiters" << logging::endl;
    notifier.notify_one();
  }

  //  void register_user(
};


template<typename T, typename M>
concept GamePair = std::is_base_of<Game<M>, T>::value;

#endif
//cards_game
