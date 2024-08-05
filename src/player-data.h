/*
Player data

will be used as the UserData struct in the websocket

*/

template <typename MessageType>
class PlayerData {
  // TODO better way to identify users and let them reconnect
 public:
  int id;
  struct Game<MessageType> * game;

  PlayerData() {
    this->id = 101;
    this->game = nullptr;
  }

};
