#include "board.h"

#include <stddef.h>
#include <stdio.h>

#include "panic.h"

const char *player_string(enum chess_player player) {
    switch (player) {
        case PLAYER_WHITE:
            return "white";
        case PLAYER_BLACK:
            return "black";
    }
}

// Helper: check clear path for a rook (no pieces between (sx,sy) and (tx,ty))
static bool rook_path_clear(const struct chess_board *board, int sx, int sy, int tx, int ty) {
    if (sx == tx) {
        int dy = (ty > sy) ? 1 : -1;
        for (int y = sy + dy; y != ty; y += dy) {
            if (board->board_array[y][sx].piece_type != PIECE_EMPTY) return false;
        }
        return true;
    } else if (sy == ty) {
        int dx = (tx > sx) ? 1 : -1;
        for (int x = sx + dx; x != tx; x += dx) {
            if (board->board_array[sy][x].piece_type != PIECE_EMPTY) return false;
        }
        return true;
    }
    return false; // not a straight move
}



const char *piece_string(enum piece_type piece) {
    switch (piece) {
        case PIECE_PAWN:
            return "pawn";
        case PIECE_KNIGHT:
            return "knight";
        case PIECE_BISHOP:
            return "bishop";
        case PIECE_ROOK:
            return "rook";
        case PIECE_QUEEN:
            return "queen";
        case PIECE_KING:
            return "king";
    }
}

struct chess_piece empty_piece = {
    .piece_type = PIECE_EMPTY,
    .colour = PLAYER_EMPTY,
};

void board_initialize(struct chess_board *board) {
    board->next_move_player = PLAYER_WHITE;

    // Initialize all squares as empty
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            board->board_array[y][x] = empty_piece;
        }
    }

    // White pawns (rank 2)
    for (int i = 0; i < 8; i++) {
        board->board_array[1][i].piece_type = PIECE_PAWN;
        board->board_array[1][i].colour = PLAYER_WHITE;
    }

    // Black pawns (rank 7)
    for (int i = 0; i < 8; i++) {
        board->board_array[6][i].piece_type = PIECE_PAWN;
        board->board_array[6][i].colour = PLAYER_BLACK;
    }

    // White back rank (rank 1)
    board->board_array[0][0].piece_type = PIECE_ROOK;
    board->board_array[0][0].colour = PLAYER_WHITE;
    board->board_array[0][1].piece_type = PIECE_KNIGHT;
    board->board_array[0][1].colour = PLAYER_WHITE;
    board->board_array[0][2].piece_type = PIECE_BISHOP;
    board->board_array[0][2].colour = PLAYER_WHITE;
    board->board_array[0][3].piece_type = PIECE_QUEEN;
    board->board_array[0][3].colour = PLAYER_WHITE;
    board->board_array[0][4].piece_type = PIECE_KING;
    board->board_array[0][4].colour = PLAYER_WHITE;
    board->board_array[0][5].piece_type = PIECE_BISHOP;
    board->board_array[0][5].colour = PLAYER_WHITE;
    board->board_array[0][6].piece_type = PIECE_KNIGHT;
    board->board_array[0][6].colour = PLAYER_WHITE;
    board->board_array[0][7].piece_type = PIECE_ROOK;
    board->board_array[0][7].colour = PLAYER_WHITE;

    // Black back rank (rank 8)
    board->board_array[7][0].piece_type = PIECE_ROOK;
    board->board_array[7][0].colour = PLAYER_BLACK;
    board->board_array[7][1].piece_type = PIECE_KNIGHT;
    board->board_array[7][1].colour = PLAYER_BLACK;
    board->board_array[7][2].piece_type = PIECE_BISHOP;
    board->board_array[7][2].colour = PLAYER_BLACK;
    board->board_array[7][3].piece_type = PIECE_QUEEN;
    board->board_array[7][3].colour = PLAYER_BLACK;
    board->board_array[7][4].piece_type = PIECE_KING;
    board->board_array[7][4].colour = PLAYER_BLACK;
    board->board_array[7][5].piece_type = PIECE_BISHOP;
    board->board_array[7][5].colour = PLAYER_BLACK;
    board->board_array[7][6].piece_type = PIECE_KNIGHT;
    board->board_array[7][6].colour = PLAYER_BLACK;
    board->board_array[7][7].piece_type = PIECE_ROOK;
    board->board_array[7][7].colour = PLAYER_BLACK;
}

void board_complete_move(const struct chess_board *board, struct chess_move *move) {
    const struct chess_piece target = board->board_array[move->target_square_y][move->target_square_x];

    // Error if target square contains a piece of the same colour
    if (target.piece_type != PIECE_EMPTY && target.colour == board->next_move_player) {
        panicf("move completion error: %s %s to %c%d (same colour on target)",
               player_string(board->next_move_player),
               piece_string(move->piece_type),
               'a' + move->target_square_x,
               move->target_square_y + 1);
    }

    // --- Pawn moves ---
    if (move->piece_type == PIECE_PAWN) {
        if (move->capture) {
            if (target.piece_type == PIECE_EMPTY) {
                panicf("move completion error: %s %s to %c%d (capture on empty square)",
                       player_string(board->next_move_player),
                       piece_string(move->piece_type),
                       'a' + move->target_square_x,
                       move->target_square_y + 1);
            }

            int candidates[2] = {-1, -1};
            int count = 0;

            if (board->next_move_player == PLAYER_WHITE) {
                if (move->target_square_x > 0 &&
                    board->board_array[move->target_square_y - 1][move->target_square_x - 1].piece_type == PIECE_PAWN &&
                    board->board_array[move->target_square_y - 1][move->target_square_x - 1].colour == PLAYER_WHITE) {
                    candidates[count++] = move->target_square_x - 1;
                }
                if (move->target_square_x < 7 &&
                    board->board_array[move->target_square_y - 1][move->target_square_x + 1].piece_type == PIECE_PAWN &&
                    board->board_array[move->target_square_y - 1][move->target_square_x + 1].colour == PLAYER_WHITE) {
                    candidates[count++] = move->target_square_x + 1;
                }
            } else {
                if (move->target_square_x > 0 &&
                    board->board_array[move->target_square_y + 1][move->target_square_x - 1].piece_type == PIECE_PAWN &&
                    board->board_array[move->target_square_y + 1][move->target_square_x - 1].colour == PLAYER_BLACK) {
                    candidates[count++] = move->target_square_x - 1;
                }
                if (move->target_square_x < 7 &&
                    board->board_array[move->target_square_y + 1][move->target_square_x + 1].piece_type == PIECE_PAWN &&
                    board->board_array[move->target_square_y + 1][move->target_square_x + 1].colour == PLAYER_BLACK) {
                    candidates[count++] = move->target_square_x + 1;
                }
            }

            if (count == 0) {
                panicf("move completion error: %s %s to %c%d (no pawn can capture)",
                       player_string(board->next_move_player),
                       piece_string(move->piece_type),
                       'a' + move->target_square_x,
                       move->target_square_y + 1);
            } else if (count > 1 && move->source_x == -1) {
                panicf("move completion error: %s %s to %c%d (ambiguous capture, source file not specified)",
                       player_string(board->next_move_player),
                       piece_string(move->piece_type),
                       'a' + move->target_square_x,
                       move->target_square_y + 1);
            } else {
                int src_file = (count == 1) ? candidates[0] : move->source_x;
                int src_rank = (board->next_move_player == PLAYER_WHITE)
                                   ? move->target_square_y - 1
                                   : move->target_square_y + 1;

                move->source_x = src_file;
                move->source_y = src_rank;
                move->moving_piece = board->board_array[src_rank][src_file];
            }
        } else {
            if (board->next_move_player == PLAYER_WHITE) {
                if (board->board_array[move->target_square_y - 1][move->target_square_x].piece_type == PIECE_PAWN &&
                    board->board_array[move->target_square_y - 1][move->target_square_x].colour == PLAYER_WHITE) {
                    move->source_x = move->target_square_x;
                    move->source_y = move->target_square_y - 1;
                    move->moving_piece = board->board_array[move->source_y][move->source_x];
                } else if (move->target_square_y == 3 &&
                           board->board_array[2][move->target_square_x].piece_type == PIECE_EMPTY &&
                           board->board_array[1][move->target_square_x].piece_type == PIECE_PAWN &&
                           board->board_array[1][move->target_square_x].colour == PLAYER_WHITE) {
                    move->source_x = move->target_square_x;
                    move->source_y = 1;
                    move->moving_piece = board->board_array[move->source_y][move->source_x];
                } else {
                    panicf("move completion error: WHITE PAWN to %c%d (no pawn can move)",
                           'a' + move->target_square_x, move->target_square_y + 1);
                }
            } else {
                if (board->board_array[move->target_square_y + 1][move->target_square_x].piece_type == PIECE_PAWN &&
                    board->board_array[move->target_square_y + 1][move->target_square_x].colour == PLAYER_BLACK) {
                    move->source_x = move->target_square_x;
                    move->source_y = move->target_square_y + 1;
                    move->moving_piece = board->board_array[move->source_y][move->source_x];
                } else if (move->target_square_y == 4 &&
                           board->board_array[5][move->target_square_x].piece_type == PIECE_EMPTY &&
                           board->board_array[6][move->target_square_x].piece_type == PIECE_PAWN &&
                           board->board_array[6][move->target_square_x].colour == PLAYER_BLACK) {
                    move->source_x = move->target_square_x;
                    move->source_y = 6;
                    move->moving_piece = board->board_array[move->source_y][move->source_x];
                } else {
                    panicf("move completion error: BLACK PAWN to %c%d (no pawn can move)",
                           'a' + move->target_square_x, move->target_square_y + 1);
                }
            }
        }
    }

    // Helper: check clear path for a rook (no pieces between (sx,sy) and (tx,ty))

else if (move->piece_type == PIECE_ROOK) {
    int tx = move->target_square_x;
    int ty = move->target_square_y;
    int src_x = -1, src_y = -1;

    // Target must be empty or opponent piece (basic legality)
    struct chess_piece tgt = board->board_array[ty][tx];
    if (tgt.piece_type != PIECE_EMPTY && tgt.colour == board->next_move_player) {
        panicf("illegal move: %s rook to %c%d (own piece on target)",
               player_string(board->next_move_player), 'a' + tx, ty + 1);
    }

    // --- Disambiguation by file (e.g. Rfe1, Rcc1) ---
    if (move->source_x != -1 && move->source_y == -1) {
        int sx = move->source_x;
        for (int y = 0; y < 8; y++) {
            struct chess_piece p = board->board_array[y][sx];
            if (p.piece_type == PIECE_ROOK && p.colour == board->next_move_player) {
                bool clear = false;

                if (sx == tx) {
                    // Vertical move on same file
                    int dy = (ty > y) ? 1 : -1;
                    clear = true;
                    for (int yy = y + dy; yy != ty; yy += dy) {
                        if (board->board_array[yy][sx].piece_type != PIECE_EMPTY) { clear = false; break; }
                    }
                } else if (y == ty) {
                    // Horizontal move on same rank (this fixes Rfe1)
                    int dx = (tx > sx) ? 1 : -1;
                    clear = true;
                    for (int xx = sx + dx; xx != tx; xx += dx) {
                        if (board->board_array[ty][xx].piece_type != PIECE_EMPTY) { clear = false; break; }
                    }
                }

                if (clear) { src_x = sx; src_y = y; break; }
            }
        }
    }

    // --- Disambiguation by rank (e.g. R2e2) ---
    else if (move->source_y != -1 && move->source_x == -1) {
        int sy = move->source_y;
        for (int x = 0; x < 8; x++) {
            struct chess_piece p = board->board_array[sy][x];
            if (p.piece_type == PIECE_ROOK && p.colour == board->next_move_player) {
                bool clear = false;

                if (sy == ty) {
                    // Horizontal move on same rank
                    int dx = (tx > x) ? 1 : -1;
                    clear = true;
                    for (int xx = x + dx; xx != tx; xx += dx) {
                        if (board->board_array[sy][xx].piece_type != PIECE_EMPTY) { clear = false; break; }
                    }
                } else if (x == tx) {
                    // Vertical move on same file
                    int dy = (ty > sy) ? 1 : -1;
                    clear = true;
                    for (int yy = sy + dy; yy != ty; yy += dy) {
                        if (board->board_array[yy][tx].piece_type != PIECE_EMPTY) { clear = false; break; }
                    }
                }

                if (clear) { src_x = x; src_y = sy; break; }
            }
        }
    }

    // --- No disambiguation: scan outward with clearance checks ---
    else {
        // Scan left
        for (int x = tx - 1; x >= 0 && src_x == -1; x--) {
            struct chess_piece p = board->board_array[ty][x];
            if (p.piece_type != PIECE_EMPTY) {
                if (p.piece_type == PIECE_ROOK && p.colour == board->next_move_player) {
                    bool clear = true;
                    for (int xx = x + 1; xx < tx; xx++) {
                        if (board->board_array[ty][xx].piece_type != PIECE_EMPTY) { clear = false; break; }
                    }
                    if (clear) { src_x = x; src_y = ty; }
                }
                break;
            }
        }
        // Scan right
        for (int x = tx + 1; x < 8 && src_x == -1; x++) {
            struct chess_piece p = board->board_array[ty][x];
            if (p.piece_type != PIECE_EMPTY) {
                if (p.piece_type == PIECE_ROOK && p.colour == board->next_move_player) {
                    bool clear = true;
                    for (int xx = x - 1; xx > tx; xx--) {
                        if (board->board_array[ty][xx].piece_type != PIECE_EMPTY) { clear = false; break; }
                    }
                    if (clear) { src_x = x; src_y = ty; }
                }
                break;
            }
        }
        // Scan down
        for (int y = ty - 1; y >= 0 && src_x == -1; y--) {
            struct chess_piece p = board->board_array[y][tx];
            if (p.piece_type != PIECE_EMPTY) {
                if (p.piece_type == PIECE_ROOK && p.colour == board->next_move_player) {
                    bool clear = true;
                    for (int yy = y + 1; yy < ty; yy++) {
                        if (board->board_array[yy][tx].piece_type != PIECE_EMPTY) { clear = false; break; }
                    }
                    if (clear) { src_x = tx; src_y = y; }
                }
                break;
            }
        }
        // Scan up
        for (int y = ty + 1; y < 8 && src_x == -1; y++) {
            struct chess_piece p = board->board_array[y][tx];
            if (p.piece_type != PIECE_EMPTY) {
                if (p.piece_type == PIECE_ROOK && p.colour == board->next_move_player) {
                    bool clear = true;
                    for (int yy = y - 1; yy > ty; yy--) {
                        if (board->board_array[yy][tx].piece_type != PIECE_EMPTY) { clear = false; break; }
                    }
                    if (clear) { src_x = tx; src_y = y; }
                }
                break;
            }
        }
    }

    // --- Finalize ---
    if (src_x == -1) {
        panicf("move completion error: %s rook to %c%d (no rook can move)",
               player_string(board->next_move_player), 'a' + tx, ty + 1);
    } else {
        move->source_x = src_x;
        move->source_y = src_y;
        move->moving_piece = board->board_array[src_y][src_x];
    }
}

    else if (move->piece_type == PIECE_BISHOP) {
        int src_x = -1, src_y = -1;
        int found = 0;

        // Scan diagonals
        int dirs[4][2] = {{-1, 1}, {1, 1}, {-1, -1}, {1, -1}};
        for (int d = 0; d < 4; d++) {
            int dx = dirs[d][0], dy = dirs[d][1];
            int x = move->target_square_x + dx;
            int y = move->target_square_y + dy;
            while (x >= 0 && x < 8 && y >= 0 && y < 8) {
                struct chess_piece p = board->board_array[y][x];
                if (p.piece_type != PIECE_EMPTY) {
                    if (p.piece_type == PIECE_BISHOP && p.colour == board->next_move_player) {
                        // Candidate found
                        if (found == 0) {
                            src_x = x;
                            src_y = y;
                        }
                        found++;
                    }
                    break; // stop scanning this direction
                }
                x += dx;
                y += dy;
            }
        }

        if (found == 0) {
            panicf("move completion error: %s bishop to %c%d (no bishop can move)",
                   player_string(board->next_move_player),
                   'a' + move->target_square_x,
                   move->target_square_y + 1);
        } else if (found > 1) {
            // Use disambiguation if provided
            if (move->source_x != -1 || move->source_y != -1) {
                bool matched = false;
                for (int d = 0; d < 4; d++) {
                    int dx = dirs[d][0], dy = dirs[d][1];
                    int x = move->target_square_x + dx;
                    int y = move->target_square_y + dy;
                    while (x >= 0 && x < 8 && y >= 0 && y < 8) {
                        struct chess_piece p = board->board_array[y][x];
                        if (p.piece_type != PIECE_EMPTY) {
                            if (p.piece_type == PIECE_BISHOP && p.colour == board->next_move_player) {
                                if ((move->source_x == -1 || move->source_x == x) &&
                                    (move->source_y == -1 || move->source_y == y)) {
                                    src_x = x;
                                    src_y = y;
                                    matched = true;
                                }
                            }
                            break;
                        }
                        x += dx;
                        y += dy;
                    }
                }
                if (!matched) {
                    panicf("move completion error: %s bishop to %c%d (disambiguation does not match any bishop)",
                           player_string(board->next_move_player),
                           'a' + move->target_square_x,
                           move->target_square_y + 1);
                }
            } else {
                panicf("move completion error: %s bishop to %c%d (ambiguous bishop move, source not specified)",
                       player_string(board->next_move_player),
                       'a' + move->target_square_x,
                       move->target_square_y + 1);
            }
        }

        // Finalize
        move->source_x = src_x;
        move->source_y = src_y;
        move->moving_piece = board->board_array[src_y][src_x];
    } else if (move->piece_type == PIECE_QUEEN) {
        int src_x = -1, src_y = -1;
        int found = 0;

        // Directions: rook + bishop
        int dirs[8][2] = {
            {-1, 0}, {1, 0}, {0, -1}, {0, 1}, // rook directions
            {-1, 1}, {1, 1}, {-1, -1}, {1, -1} // bishop directions
        };

        // Scan all 8 directions
        for (int d = 0; d < 8; d++) {
            int dx = dirs[d][0], dy = dirs[d][1];
            int x = move->target_square_x + dx;
            int y = move->target_square_y + dy;
            while (x >= 0 && x < 8 && y >= 0 && y < 8) {
                struct chess_piece p = board->board_array[y][x];
                if (p.piece_type != PIECE_EMPTY) {
                    if (p.piece_type == PIECE_QUEEN && p.colour == board->next_move_player) {
                        // Candidate queen found
                        if (found == 0) {
                            src_x = x;
                            src_y = y;
                        }
                        found++;
                    }
                    break; // stop scanning this direction
                }
                x += dx;
                y += dy;
            }
        }

        if (found == 0) {
            panicf("move completion error: %s queen to %c%d (no queen can move)",
                   player_string(board->next_move_player),
                   'a' + move->target_square_x,
                   move->target_square_y + 1);
        } else if (found > 1) {
            // Use disambiguation if provided
            if (move->source_x != -1 || move->source_y != -1) {
                bool matched = false;
                for (int d = 0; d < 8; d++) {
                    int dx = dirs[d][0], dy = dirs[d][1];
                    int x = move->target_square_x + dx;
                    int y = move->target_square_y + dy;
                    while (x >= 0 && x < 8 && y >= 0 && y < 8) {
                        struct chess_piece p = board->board_array[y][x];
                        if (p.piece_type != PIECE_EMPTY) {
                            if (p.piece_type == PIECE_QUEEN && p.colour == board->next_move_player) {
                                if ((move->source_x == -1 || move->source_x == x) &&
                                    (move->source_y == -1 || move->source_y == y)) {
                                    src_x = x;
                                    src_y = y;
                                    matched = true;
                                }
                            }
                            break;
                        }
                        x += dx;
                        y += dy;
                    }
                }
                if (!matched) {
                    panicf("move completion error: %s queen to %c%d (disambiguation does not match any queen)",
                           player_string(board->next_move_player),
                           'a' + move->target_square_x,
                           move->target_square_y + 1);
                }
            } else {
                panicf("move completion error: %s queen to %c%d (ambiguous queen move, source not specified)",
                       player_string(board->next_move_player),
                       'a' + move->target_square_x,
                       move->target_square_y + 1);
            }
        }

        // Finalize
        move->source_x = src_x;
        move->source_y = src_y;
        move->moving_piece = board->board_array[src_y][src_x];
    } else if (move->piece_type == PIECE_KNIGHT) {
        int src_x = -1, src_y = -1;
        int found = 0;

        // All 8 knight move offsets
        int offsets[8][2] = {
            {1, 2}, {2, 1}, {-1, 2}, {-2, 1},
            {1, -2}, {2, -1}, {-1, -2}, {-2, -1}
        };

        for (int i = 0; i < 8; i++) {
            int x = move->target_square_x + offsets[i][0];
            int y = move->target_square_y + offsets[i][1];
            if (x >= 0 && x < 8 && y >= 0 && y < 8) {
                struct chess_piece p = board->board_array[y][x];
                if (p.piece_type == PIECE_KNIGHT && p.colour == board->next_move_player) {
                    if (found == 0) {
                        src_x = x;
                        src_y = y;
                    }
                    found++;
                }
            }
        }

        if (found == 0) {
            panicf("move completion error: %s knight to %c%d (no knight can move)",
                   player_string(board->next_move_player),
                   'a' + move->target_square_x,
                   move->target_square_y + 1);
        } else if (found > 1) {
            // Use disambiguation if provided
            if (move->source_x != -1 || move->source_y != -1) {
                bool matched = false;
                for (int i = 0; i < 8; i++) {
                    int x = move->target_square_x + offsets[i][0];
                    int y = move->target_square_y + offsets[i][1];
                    if (x >= 0 && x < 8 && y >= 0 && y < 8) {
                        struct chess_piece p = board->board_array[y][x];
                        if (p.piece_type == PIECE_KNIGHT && p.colour == board->next_move_player) {
                            if ((move->source_x == -1 || move->source_x == x) &&
                                (move->source_y == -1 || move->source_y == y)) {
                                src_x = x;
                                src_y = y;
                                matched = true;
                            }
                        }
                    }
                }
                if (!matched) {
                    panicf("move completion error: %s knight to %c%d (disambiguation does not match any knight)",
                           player_string(board->next_move_player),
                           'a' + move->target_square_x,
                           move->target_square_y + 1);
                }
            } else {
                panicf("move completion error: %s knight to %c%d (ambiguous knight move, source not specified)",
                       player_string(board->next_move_player),
                       'a' + move->target_square_x,
                       move->target_square_y + 1);
            }
        }

        // Finalize
        move->source_x = src_x;
        move->source_y = src_y;
        move->moving_piece = board->board_array[src_y][src_x];
    }
    else if (move->piece_type == PIECE_KING && move->castling != CASTLE_NONE) {
    int src_x = 4; // e-file
    int src_y = (board->next_move_player == PLAYER_WHITE ? 0 : 7);

    int dst_x, dst_y;
    dst_y = src_y;

    if (move->castling == CASTLE_KINGSIDE) {
        dst_x = 6; // g-file
        // Check rook presence
        struct chess_piece rook = board->board_array[src_y][7];
        if (rook.piece_type != PIECE_ROOK || rook.colour != board->next_move_player) {
            panicf("move completion error: %s castling kingside (rook not present)",
                   player_string(board->next_move_player));
        }
        // Check empty squares between king and rook
        if (board->board_array[src_y][5].piece_type != PIECE_EMPTY ||
            board->board_array[src_y][6].piece_type != PIECE_EMPTY) {
            panicf("move completion error: %s castling kingside (path blocked)",
                   player_string(board->next_move_player));
        }
    } else { // Queenside
        dst_x = 2; // c-file
        struct chess_piece rook = board->board_array[src_y][0];
        if (rook.piece_type != PIECE_ROOK || rook.colour != board->next_move_player) {
            panicf("move completion error: %s castling queenside (rook not present)",
                   player_string(board->next_move_player));
        }
        if (board->board_array[src_y][1].piece_type != PIECE_EMPTY ||
            board->board_array[src_y][2].piece_type != PIECE_EMPTY ||
            board->board_array[src_y][3].piece_type != PIECE_EMPTY) {
            panicf("move completion error: %s castling queenside (path blocked)",
                   player_string(board->next_move_player));
        }
    }

    // Confirm king is on starting square
    struct chess_piece king = board->board_array[src_y][src_x];
    if (king.piece_type != PIECE_KING || king.colour != board->next_move_player) {
        panicf("move completion error: %s castling (king not on starting square)",
               player_string(board->next_move_player));
    }

    // TODO: add checks for "king/rook has not moved" and "squares not attacked"
    // These require extra state tracking and an attack-detection helper.

    // Finalize move
    move->source_x = src_x;
    move->source_y = src_y;
    move->target_square_x = dst_x;
    move->target_square_y = dst_y;
    move->moving_piece = king;
}

    else if (move->piece_type == PIECE_KING) {
        int src_x = -1, src_y = -1;
        int found = 0;

        // All 8 king move offsets
        int offsets[8][2] = {
            {1,0}, {-1,0}, {0,1}, {0,-1},
            {1,1}, {1,-1}, {-1,1}, {-1,-1}
        };

        // Instead of target+offset, check target-offset
        for (int i = 0; i < 8; i++) {
            int x = move->target_square_x - offsets[i][0];
            int y = move->target_square_y - offsets[i][1];
            if (x >= 0 && x < 8 && y >= 0 && y < 8) {
                struct chess_piece p = board->board_array[y][x];
                if (p.piece_type == PIECE_KING && p.colour == board->next_move_player) {
                    src_x = x; src_y = y;
                    found++;
                }
            }
        }

        if (found == 0) {
            panicf("move completion error: %s king to %c%d (no king can move)",
                   player_string(board->next_move_player),
                   'a' + move->target_square_x,
                   move->target_square_y + 1);
        } else if (found > 1) {
            // Should never happen — each side has only one king
            panicf("move completion error: %s king to %c%d (ambiguous king move)",
                   player_string(board->next_move_player),
                   'a' + move->target_square_x,
                   move->target_square_y + 1);
        }

        // Finalize
        move->source_x = src_x;
        move->source_y = src_y;
        move->moving_piece = board->board_array[src_y][src_x];
    }

else if (move->piece_type == PIECE_KING && move->castling != CASTLE_NONE) {
    int src_x = 4; // e-file
    int src_y = (board->next_move_player == PLAYER_WHITE ? 0 : 7);

    int dst_x, dst_y;
    dst_y = src_y;

    if (move->castling == CASTLE_KINGSIDE) {
        dst_x = 6; // g-file
        // Check rook presence
        struct chess_piece rook = board->board_array[src_y][7];
        if (rook.piece_type != PIECE_ROOK || rook.colour != board->next_move_player) {
            panicf("move completion error: %s castling kingside (rook not present)",
                   player_string(board->next_move_player));
        }
        // Check empty squares between king and rook
        if (board->board_array[src_y][5].piece_type != PIECE_EMPTY ||
            board->board_array[src_y][6].piece_type != PIECE_EMPTY) {
            panicf("move completion error: %s castling kingside (path blocked)",
                   player_string(board->next_move_player));
        }
    } else { // Queenside
        dst_x = 2; // c-file
        struct chess_piece rook = board->board_array[src_y][0];
        if (rook.piece_type != PIECE_ROOK || rook.colour != board->next_move_player) {
            panicf("move completion error: %s castling queenside (rook not present)",
                   player_string(board->next_move_player));
        }
        if (board->board_array[src_y][1].piece_type != PIECE_EMPTY ||
            board->board_array[src_y][2].piece_type != PIECE_EMPTY ||
            board->board_array[src_y][3].piece_type != PIECE_EMPTY) {
            panicf("move completion error: %s castling queenside (path blocked)",
                   player_string(board->next_move_player));
        }
    }

    // Confirm king is on starting square
    struct chess_piece king = board->board_array[src_y][src_x];
    if (king.piece_type != PIECE_KING || king.colour != board->next_move_player) {
        panicf("move completion error: %s castling (king not on starting square)",
               player_string(board->next_move_player));
    }

    // TODO: add checks for "king/rook has not moved" and "squares not attacked"
    // These require extra state tracking and an attack-detection helper.

    // Finalize move
    move->source_x = src_x;
    move->source_y = src_y;
    move->target_square_x = dst_x;
    move->target_square_y = dst_y;
    move->moving_piece = king;
}



}


// Helper: convert piece to char
char piece_char(struct chess_piece p) {
    if (p.piece_type == PIECE_EMPTY) return '.';

    char c;
    switch (p.piece_type) {
        case PIECE_PAWN: c = 'P';
            break;
        case PIECE_KNIGHT: c = 'N';
            break;
        case PIECE_BISHOP: c = 'B';
            break;
        case PIECE_ROOK: c = 'R';
            break;
        case PIECE_QUEEN: c = 'Q';
            break;
        case PIECE_KING: c = 'K';
            break;
        default: c = '?';
            break;
    }
    return c;
}

// Draw the board
void board_draw(const struct chess_board *board) {
    printf("\n   a b c d e f g h\n");
    printf("  -----------------\n");

    for (int y = 7; y >= 0; y--) {
        // rank 8 down to 1
        printf("%d| ", y + 1);
        for (int x = 0; x < 8; x++) {
            printf("%c ", piece_char(board->board_array[y][x]));
        }
        printf("|%d\n", y + 1);
    }

    printf("  -----------------\n");
    printf("   a b c d e f g h\n\n");
}

void board_apply_move(struct chess_board *board, const struct chess_move *move) {
    //if a piece is captured, the target square needs to be reset to empty
    if (move->capture) {
        board->board_array[move->target_square_y][move->target_square_x] = empty_piece;
    }
    // Apply castling move: rearrange pieces only
    if (move->piece_type == PIECE_KING && move->castling != CASTLE_NONE) {
        int y = (move->moving_piece.colour == PLAYER_WHITE ? 0 : 7);

        if (move->castling == CASTLE_KINGSIDE) {
            // King: e -> g
            board->board_array[y][6] = board->board_array[y][4];
            board->board_array[y][4].piece_type = PIECE_EMPTY;

            // Rook: h -> f
            board->board_array[y][5] = board->board_array[y][7];
            board->board_array[y][7].piece_type = PIECE_EMPTY;
        } else { // Queenside
            // King: e -> c
            board->board_array[y][2] = board->board_array[y][4];
            board->board_array[y][4].piece_type = PIECE_EMPTY;

            // Rook: a -> d
            board->board_array[y][3] = board->board_array[y][0];
            board->board_array[y][0].piece_type = PIECE_EMPTY;
        }
    }
    //move piece to another square while replacing the source square with an empty space
    board->board_array[move->target_square_y][move->target_square_x] = move->moving_piece;
    board->board_array[move->source_y][move->source_x] = empty_piece;

    // The final step is to update the turn of players in the board state.
    switch (board->next_move_player) {
        case PLAYER_WHITE:
            board->next_move_player = PLAYER_BLACK;
            break;
        case PLAYER_BLACK:
            board->next_move_player = PLAYER_WHITE;
            break;
    }
    board_draw(board);
}


//helper funct
void board_summarize(const struct chess_board *board) {
    // TODO: print the state of the game.
    //determine whose turn it is
    enum chess_player current_player = board->next_move_player;
    //is the current player's king under attack?
    bool in_check = false;
    //Does the current player have any legal moves left?
    bool can_move = true;
    if (can_move) {
        printf("game incomplete\n");
    } else {
        //the player has no legal moves，we need to check if it's checkmate or stalemate

        if (in_check) {
            if (current_player == PLAYER_WHITE)
                printf("black wins by checkmate\n");
            else
                printf("white wins by checkmate\n");
        } else {
            printf("draw by stalemate\n");
        }
    }
}


//d4 Nf6 Bf4 g6 e3 Bg7 Bd3 d5 Nd2 c6 c3 Qb6 Qb3 Nbd7 Ngf3 Nh5 Qxb6 axb6 h3 Nxf4 exf4 Nf6 a3 O-O O-O Nh5 g3 Bxh3 Rfe1 e6 c4 Bg4 cxd5 exd5 Ne5 Bxe5 dxe5 c5 f3 Bd7 g4 Nxf4 Bc2 Bb5 Nb1 Rfe8 Nc3 Ba6 Ba4 Re7 Rad1 d4 Ne4 Kg7 Nd6 Nd3 Re2 Nxe5 Rf2 Nd3 Rg2 Nf4 Rh2 Ne2 Kg2 Nf4 Kg3 Nd5 Rdh1 Rh8 Bc2 Ne3 Kf4 Nxc2 Rxc2 Re2 Rcc1 Rxb2 Ne4 Rd8 Ng3 d3 Ne4 d2 Rcd1 Rd4 Ke5 Be2 Rxh7 Kxh7 Ng5 Kg7 Rh1 f6 Ke6 fxg5 a4 Bxf3 Rg1 d1=Q Rxd1 Rxd1 a5 Re2