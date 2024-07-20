/*
Player data

will be used as the UserData struct in the websocket

*/

struct PlayerData {
  // TODO better way to identify users and let them reconnect
  int id;
  struct Game * game;

  PlayerData() {
    this->id = 101;
    this->game = nullptr;
  }

};
