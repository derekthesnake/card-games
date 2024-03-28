#include "App.h"
#include <thread>
#include <algorithm>

#include "game.h"
#include "protos/message.pb.h"

int main() {
  logging::log_level = logging::DEBUG;
  
  int listen_port = 9001;
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  /* ws->getUserData returns one of these */
  struct PerSocketData {
    //Game game;
  };

  Game g = Game{[](cards::Message m) {
    logging::debug() << "In the worker!" << std::endl;
    logging::message() << m.test() << std::endl;
    logging::debug() << "after printing the msg" << std::endl;
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
	  .open = [](auto */*ws*/) {
	    
	  },
	  .message = [&](auto *ws, std::string_view message, uWS::OpCode opCode) {
	    logging::debug() << "adding message to queue " << std::string{message} << std::endl;
	    std::shared_ptr<cards::Message> m = std::make_shared<cards::Message>();
	    if (m->ParseFromString(std::string{message})) {
	      logging::debug() << "PFS worked" << std::endl;
	    } else {
	      logging::error() << "ParseFromString failed" << std::endl;
	    }
	    logging::debug() << "m test in producer: " + m->test() << std::endl;
	    g.add_message(m);
	    //logging::debug("m addr in producer: " + &*m);
	    ws->send("ack", opCode);
	  },
	  .drain = [](auto */*ws*/) {
	    /* Check getBufferedAmount here */
	  },
	  .ping = [](auto */*ws*/, std::string_view) {
	    
	  },
	  .pong = [](auto */*ws*/, std::string_view) {
	    
	  },
	  .close = [](auto */*ws*/, int /*code*/, std::string_view /*message*/) {
	    
	  }
	}).listen(listen_port, [listen_port](auto *listen_socket) {
	  if (listen_socket) {
	    logging::info() << "Thread " << std::this_thread::get_id() << " listening on port " + listen_port << std::endl;
	  } else {
	    logging::error() << "Thread " << std::this_thread::get_id() << " failed to listen on port " << listen_port << std::endl;
	  }
	}).run();
      
    });
  });
  
  std::for_each(threads.begin(), threads.end(), [](std::thread *t) {
    t->join();
  });
}
