#ifndef APSC143__PARSER_H
#define APSC143__PARSER_H

#include <stdbool.h>
#include "board.h"

// Read a move from standard input. The initial contents of *move are ignored
// and can be uninitialized. If there is a syntax error or the end of the input
// is reached, returns false; the contents of *move in such a case are
// unspecified.
bool parse_move(struct chess_move *move);

#endif
