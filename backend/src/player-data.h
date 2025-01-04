/*
Player data

will be used as the UserData struct in the websocket

*/
#include <memory>

#include "game.h"

template <class ConcreteGame>
class PlayerData;

template <class ConcreteGame>
class PlayerData {
  // TODO better way to identify users and let them reconnect
 public:
  std::string id;
  std::shared_ptr<ConcreteGame> game;

  PlayerData(std::string& id) : id(id) {
  }

  PlayerData(){}

};
