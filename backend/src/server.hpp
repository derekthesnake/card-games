#ifndef cards_server
#define cards_server

#include "App.h"
#include <thread>
#include <algorithm>
#include <type_traits>

#include "game.h"
#include "player-data.h"
#include "protos/message.pb.h"

namespace server {
  template <class ConcreteGame, class ProtoBufType> requires GamePair <ConcreteGame, ProtoBufType>
  void serve() {
    //static_assert(std::is_base_of_v<Game, ConcreteGame>, "Argument must be a subclass of Game");
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
      PlayerData<Game<ProtoBufType>> playerData;
    };

    /* Simple echo websocket server, using multiple threads */
    std::vector<std::thread *> threads(std::thread::hardware_concurrency());
  
    std::transform(threads.begin(), threads.end(), threads.begin(), [&](std::thread */*t*/) {
      return new std::thread([&]() {
	/* Very simple WebSocket echo server */
	uWS::App().ws<PlayerData<Game<ProtoBufType>>>("/*", {
	    /* Settings */
	    .compression = uWS::SHARED_COMPRESSOR,
	      .maxPayloadLength = 16 * 1024,
	      .idleTimeout = 10,
	      .maxBackpressure = 1 * 1024 * 1024,
	      /* Handlers */
	      .upgrade = [](auto *res, auto *req, auto *context) {
		/* when reading from req, must COPY data out. req dies soon. */
		logging::debug() << "Upgrade hook called" << logging::endl;
		std::string hwid = std::string(req->getHeader("card-games-hwid"));
		res->template upgrade<PerSocketData>({
		    PlayerData<Game<ProtoBufType>>(hwid)},
		  req->getHeader("sec-websocket-key"),
		  req->getHeader("sec-websocket-protocol"),
		  req->getHeader("sec-websocket-extensions"),
		  context);
	      },
	      .open = [](auto * ws) {
		// temporary, figure out lobby stuff
		std::shared_ptr<ConcreteGame> g = std::make_shared<ConcreteGame>();
		
		
		logging::info() << "open() called" << logging::endl;
		/*
		  PlayerData<cards::Message> p = ws->getUserData()->playerData;
		  logging::debug() << "about to dereference playerdata" << logging::endl;
		  logging::debug() << "playerdata.id = " << p.id << logging::endl; */
		logging::debug() << "it worked" << logging::endl;
		if (g->register_player(ws->getUserData()->id, ws)) {
		  ws->getUserData()->game = g;
		}
		
	      },
	      .message = [&](auto *ws, std::string_view message, uWS::OpCode opCode) {
		// auth: discard all messages from sockets where the user in userdata hasn't been set
		// except for the login message
		logging::info() << "message(" << ws << ", " << message << ", " << opCode << ") called" << logging::endl;
		if (ws->getUserData()->game) {
		  logging::debug() << "Socket has associated Game." << logging::endl;
		  logging::debug() << "adding message to queue " << std::string{message} << logging::endl;
		  std::shared_ptr<ProtoBufType> m = std::make_shared<ProtoBufType>();
		  if (m->ParseFromString(std::string{message})) {
		    logging::debug() << "PFS worked" << logging::endl;
		  } else {
		    logging::error() << "ParseFromString failed" << logging::endl;
		  }
		  logging::debug() << "m test in producer: " + m->test() << logging::endl;
		  ws->getUserData()->game->add_message(m);
		} else {
		  logging::debug() << "Socket has no associated Game." << logging::endl;
		  logging::debug() << "Hopefully it is a server-targeted message." << logging::endl;
		}
		ws->send(ws->getUserData()->id, opCode);
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
	      .close = [](auto * ws, int code, std::string_view message) {
		logging::info() << "close(" << ws << ", " << code << ", " << message << ") called" << logging::endl;
		auto game = ws->getUserData()->game;
		auto id = ws->getUserData()->id;
		if (game) {
		  logging::debug() << "Removing player '" << id << "' from game." << logging::endl; 
		  if (game->deregister_player(id)) {
		    logging::debug() << "Successfully removed player '" << id << "' from game." << logging::endl;
		    ws->getUserData()->game.reset();
		  } else {
		    logging::error() << "Failed to remove player '" << id << "' from game for unknown reason." << logging::endl;
		    // TODO: tell user?
		  }
		} else {
		  logging::debug() << "Player '" << id << "' not associated with a game. Not removing them from anything." << logging::endl;
		}
	      }}).listen(listen_port, [listen_port](auto *listen_socket) {
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
}

#endif
