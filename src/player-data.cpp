/*
Player data

will be used as the UserData struct in the websocket

*/

struct PlayerData {
  // TODO better way to identify users and let them reconnect
  int id;
  struct * GameQueue game_queue;
}
