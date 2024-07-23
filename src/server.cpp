#include "App.h"
#include <thread>
#include <algorithm>

#include "game.h"
#include "player-data.h"
#include "protos/message.pb.h"

int main() {
  logging::log_level = logging::DEBUG;

  {
    std::time_t now = std::time(nullptr);
    logging::info() << "Game server started at " << std::asctime(std::localtime(&now)) << logging::endl;
  }
  
  int listen_port = 9001;
  logging::debug() << "Listen port set to " << listen_port << logging::endl;
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  
  /* ws->getUserData returns one of these */
  struct PerSocketData {
    Game * game;
    PlayerData playerData;
  };

  Game g = Game{[](cards::Message m) {
    logging::debug() << "In the worker!" << logging::endl;
    logging::message() << m.test() << logging::endl;
    logging::debug() << "after printing the msg" << logging::endl;
  }};
  /* Simple echo websocket server, using multiple threads */
  std::vector<std::thread *> threads(std::thread::hardware_concurrency());
  
  std::transform(threads.begin(), threads.end(), threads.begin(), [&](std::thread */*t*/) {
    return new std::thread([&]() {
      
      /* Very simple WebSocket echo server */
      uWS::App().ws<PerSocketData>("/*", {
	  /* Settings */
	  .compression = uWS::SHARED_COMPRESSOR,
	  .maxPayloadLength = 16 * 1024,
	  .idleTimeout = 10,
	  .maxBackpressure = 1 * 1024 * 1024,
	  /* Handlers */
	  .upgrade = nullptr,
	  .open = [](auto * ws) {
	    logging::info() << "open() called" << logging::endl;
	    PlayerData p = ws->getUserData()->playerData;
	    logging::debug() << "about to dereference playerdata" << logging::endl;
	    logging::debug() << "playerdata.id = " << p.id << logging::endl;
	    logging::debug() << "it worked" << logging::endl;
	  },
	  .message = [&](auto *ws, std::string_view message, uWS::OpCode opCode) {
	    // auth: discard all messages from sockets where the user in userdata hasn't been set
	    // except for the login message
	    logging::info() << "message(" << ws << ", " << message << ", " << opCode << ") called" << logging::endl;
	    logging::debug() << "adding message to queue " << std::string{message} << logging::endl;
	    std::shared_ptr<cards::Message> m = std::make_shared<cards::Message>();
	    if (m->ParseFromString(std::string{message})) {
	      logging::debug() << "PFS worked" << logging::endl;
	    } else {
	      logging::error() << "ParseFromString failed" << logging::endl;
	    }
	    logging::debug() << "m test in producer: " + m->test() << logging::endl;
	    g.add_message(m);
	    //logging::debug("m addr in producer: " + &*m);
	    ws->send("ack", opCode);
	  },
	  .drain = [](auto */*ws*/) {
	    /* Check getBufferedAmount here */
	    logging::info() << "drain called" << logging::endl;
	  },
	  .ping = [](auto */*ws*/, std::string_view) {
	    logging::info() << "ping called" << logging::endl;
	  },
	  .pong = [](auto */*ws*/, std::string_view) {
	    logging::info() << "pong called" << std:: endl;
	  },
	  .close = [](auto *ws, int code, std::string_view message) {
	    logging::info() << "close(" << ws << ", " << code << ", " << message << ") called" << logging::endl;
	  }
	}).listen(listen_port, [listen_port](auto *listen_socket) {
	  if (listen_socket) {
	    logging::info() << "Thread " << std::this_thread::get_id() << " listening on port " << listen_port << logging::endl;
	  } else {
	    logging::error() << "Thread " << std::this_thread::get_id() << " failed to listen on port " << listen_port << logging::endl;
	  }
	}).run();
      
    });
  });
  
  std::for_each(threads.begin(), threads.end(), [](std::thread *t) {
    t->join();
  });
}
