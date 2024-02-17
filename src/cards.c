/*
Cards utilities

Cards are represented as an 8 bit unsigned int.
Upper 4 bits are the suit, lower four are the rank.

Typically trump suits will be represented as a separate suit from the original card.
*/

uint8 get_suit(uint8 card) {
  return card >> 4;
}

uint8 get_rank(uint8 card) {
  return card >> 4 << 4;
}

uint8 make_card(uint8 suit, uint8 rank) {
  return suit << 4 + rank;
}
