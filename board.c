#include "board.h"

#include <stddef.h>
#include <stdio.h>

#include "panic.h"
#include <stdlib.h>

const char *player_string(enum chess_player player) {
    switch (player) {
        case PLAYER_WHITE:
            return "white";
        case PLAYER_BLACK:
            return "black";
    }
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

//intializes board with propper piece order as well as empty squares
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


// fills out necessary fields of the given move struct so the apply move function will know exactly which piece is
//moving and if it is actually legal
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

    // Pawn moves
    if (move->piece_type == PIECE_PAWN) {
        if (move->capture) {

            // throws error if there isn't a pawn to capture
            if (target.piece_type == PIECE_EMPTY) {
                panicf("move completion error: %s %s to %c%d (capture on empty square)",
                       player_string(board->next_move_player),
                       piece_string(move->piece_type),
                       'a' + move->target_square_x,
                       move->target_square_y + 1);
            }

            // candidates array stores the file of a pawn that is available to make the move and the count variable
            //keeps track of how many candidates have been found
            int candidates[2] = {-1, -1};
            int count = 0;

            // finds candidates based on which colour is capturing
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

            //throws error if there are no pawns that can preform the capture
            if (count == 0) {
                panicf("move completion error: %s %s to %c%d (no pawn can capture)",
                       player_string(board->next_move_player),
                       piece_string(move->piece_type),
                       'a' + move->target_square_x,
                       move->target_square_y + 1);
            }

            // throws error if 2 pawns can capture and the origin file wasn't specified
            else if (count > 1 && move->source_x == -1) {
                panicf("move completion error: %s %s to %c%d (ambiguous capture, source file not specified)",
                       player_string(board->next_move_player),
                       piece_string(move->piece_type),
                       'a' + move->target_square_x,
                       move->target_square_y + 1);
            }

            // fills out source_x and source_y
            else {
                int src_file = (count == 1) ? candidates[0] : move->source_x;
                int src_rank = (board->next_move_player == PLAYER_WHITE) ? move->target_square_y - 1: move->target_square_y + 1;
                move->source_x = src_file;
                move->source_y = src_rank;
                move->moving_piece = board->board_array[src_rank][src_file];
            }
        }

        // if move isn't a capture
        else {
            if (board->next_move_player == PLAYER_WHITE) {
                // checks below target square to ensure a movable pawn is there
                if (board->board_array[move->target_square_y - 1][move->target_square_x].piece_type == PIECE_PAWN &&
                    board->board_array[move->target_square_y - 1][move->target_square_x].colour == PLAYER_WHITE) {
                    move->source_x = move->target_square_x;
                    move->source_y = move->target_square_y - 1;
                    move->moving_piece = board->board_array[move->source_y][move->source_x];
                    }
                //checks case that pawn moves 2 squares to the 4th row from it's starting position
                else if (move->target_square_y == 3 &&
                           board->board_array[2][move->target_square_x].piece_type == PIECE_EMPTY &&
                           board->board_array[1][move->target_square_x].piece_type == PIECE_PAWN &&
                           board->board_array[1][move->target_square_x].colour == PLAYER_WHITE) {
                    move->source_x = move->target_square_x;
                    move->source_y = 1;
                    move->moving_piece = board->board_array[move->source_y][move->source_x];
                           }

                else {
                    panicf("move completion error: WHITE PAWN to %c%d (no pawn can move)",
                           'a' + move->target_square_x, move->target_square_y + 1);
                }
            }

            // standard pawn move for black pawns
            // same code as white pawns just scanning in the opposite direction
            else {
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


//
else if (move->piece_type == PIECE_ROOK) {
     //target square must be empty or contain opponent piece
    struct chess_piece target_piece = board->board_array[move->target_square_y][move->target_square_x];
    if (target_piece.piece_type != PIECE_EMPTY && target_piece.colour == board->next_move_player) {
        panicf("illegal move: %s rook to %c%d (own piece on target)",
               player_string(board->next_move_player),
               'a' + move->target_square_x, move->target_square_y + 1);
    }

    bool rook_found = false;

    // If source file is given, scan that file for a rook of the correct colour
    if (move->source_x != -1 && move->source_y == -1) {
        for (int row = 0; row < 8 && !rook_found; row++) {
            struct chess_piece candidate = board->board_array[row][move->source_x];
            if (candidate.piece_type == PIECE_ROOK && candidate.colour == board->next_move_player) {
                bool path_clear = false;

                if (move->source_x == move->target_square_x) {
                    // Vertical move on same file
                    int step = (move->target_square_y > row) ? 1 : -1;
                    path_clear = true;
                    // ensures the path is clear from the rook that was found to the target square
                    for (int check_row = row + step; check_row != move->target_square_y; check_row += step) {
                        if (board->board_array[check_row][move->source_x].piece_type != PIECE_EMPTY) {
                            path_clear = false; break;
                        }
                    }
                } else if (row == move->target_square_y) {
                    // Horizontal move on same rank
                    int step = (move->target_square_x > move->source_x) ? 1 : -1;
                    path_clear = true;
                    // ensures the path is clear from the rook that was found to the target square
                    for (int check_col = move->source_x + step; check_col != move->target_square_x; check_col += step) {
                        if (board->board_array[move->target_square_y][check_col].piece_type != PIECE_EMPTY) {
                            path_clear = false; break;
                        }
                    }
                }

                if (path_clear) {
                    move->source_y = row;
                    move->moving_piece = candidate;
                    rook_found = true;
                }
            }
        }
    }

    // if source rank was given
    // similar function to when the file is given
    else if (move->source_y != -1 && move->source_x == -1) {
        for (int col = 0; col < 8 && !rook_found; col++) {
            struct chess_piece candidate = board->board_array[move->source_y][col];
            if (candidate.piece_type == PIECE_ROOK && candidate.colour == board->next_move_player) {
                bool path_clear = false;

                if (move->source_y == move->target_square_y) {
                    // Horizontal move on same rank
                    int step = (move->target_square_x > col) ? 1 : -1;
                    path_clear = true;
                    for (int check_col = col + step; check_col != move->target_square_x; check_col += step) {
                        if (board->board_array[move->source_y][check_col].piece_type != PIECE_EMPTY) {
                            path_clear = false; break;
                        }
                    }
                } else if (col == move->target_square_x) {
                    // Vertical move on same file
                    int step = (move->target_square_y > move->source_y) ? 1 : -1;
                    path_clear = true;
                    for (int check_row = move->source_y + step; check_row != move->target_square_y; check_row += step) {
                        if (board->board_array[check_row][move->target_square_x].piece_type != PIECE_EMPTY) {
                            path_clear = false; break;
                        }
                    }
                }

                if (path_clear) {
                    move->source_x = col;
                    move->moving_piece = candidate;
                    rook_found = true;
                }
            }
        }
    }

    //If no source is given at all
    // Scan outward from the target square in all four directions until a rook is found
    else {
        // Scan left
        for (int col = move->target_square_x - 1; col >= 0 && !rook_found; col--) {
            struct chess_piece candidate = board->board_array[move->target_square_y][col];
            if (candidate.piece_type != PIECE_EMPTY) {
                if (candidate.piece_type == PIECE_ROOK && candidate.colour == board->next_move_player) {
                    bool path_clear = true;
                    for (int check_col = col + 1; check_col < move->target_square_x; check_col++) {
                        if (board->board_array[move->target_square_y][check_col].piece_type != PIECE_EMPTY) {
                            path_clear = false; break;
                        }
                    }
                    if (path_clear) {
                        move->source_x = col;
                        move->source_y = move->target_square_y;
                        move->moving_piece = candidate;
                        rook_found = true;
                    }
                }
                break; // stop scanning once a piece is encountered
            }
        }

        // Scan right
        for (int col = move->target_square_x + 1; col < 8 && !rook_found; col++) {
            struct chess_piece candidate = board->board_array[move->target_square_y][col];
            if (candidate.piece_type != PIECE_EMPTY) {
                if (candidate.piece_type == PIECE_ROOK && candidate.colour == board->next_move_player) {
                    bool path_clear = true;
                    for (int check_col = col - 1; check_col > move->target_square_x; check_col--) {
                        if (board->board_array[move->target_square_y][check_col].piece_type != PIECE_EMPTY) {
                            path_clear = false; break;
                        }
                    }
                    if (path_clear) {
                        move->source_x = col;
                        move->source_y = move->target_square_y;
                        move->moving_piece = candidate;
                        rook_found = true;
                    }
                }
                break;
            }
        }

        // Scan downward
        for (int row = move->target_square_y - 1; row >= 0 && !rook_found; row--) {
            struct chess_piece candidate = board->board_array[row][move->target_square_x];
            if (candidate.piece_type != PIECE_EMPTY) {
                if (candidate.piece_type == PIECE_ROOK && candidate.colour == board->next_move_player) {
                    bool path_clear = true;
                    for (int check_row = row + 1; check_row < move->target_square_y; check_row++) {
                        if (board->board_array[check_row][move->target_square_x].piece_type != PIECE_EMPTY) {
                            path_clear = false; break;
                        }
                    }
                    if (path_clear) {
                        move->source_x = move->target_square_x;
                        move->source_y = row;
                        move->moving_piece = candidate;
                        rook_found = true;
                    }
                }
                break;
            }
        }

        // Scan upward
        for (int row = move->target_square_y + 1; row < 8 && !rook_found; row++) {
            struct chess_piece candidate = board->board_array[row][move->target_square_x];
            if (candidate.piece_type != PIECE_EMPTY) {
                if (candidate.piece_type == PIECE_ROOK && candidate.colour == board->next_move_player) {
                    bool path_clear = true;
                    for (int check_row = row - 1; check_row > move->target_square_y; check_row--) {
                        if (board->board_array[check_row][move->target_square_x].piece_type != PIECE_EMPTY) {
                            path_clear = false; break;
                        }
                    }
                    if (path_clear) {
                        move->source_x = move->target_square_x;
                        move->source_y = row;
                        move->moving_piece = candidate;
                        rook_found = true;
                    }
                }
                break;
            }
        }
    }

    // --- Finalize ---
    if (!rook_found) {
        panicf("move completion error: %s rook to %c%d (no rook can move)",
               player_string(board->next_move_player),
               'a' + move->target_square_x, move->target_square_y + 1);
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

    if (move->piece_type == PIECE_PAWN &&
        abs(move->target_square_y - move->source_y) == 2)
    {
        board->en_passant_available = 1;
        board->en_passant_x = move->source_x;
        board->en_passant_y =
            (move->source_y + move->target_square_y) / 2;
    }
    else {
        board->en_passant_available = 0;
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

// TODO: print the state of the game.
//helper: check if coordinates are valid
bool is_valid_pos(int x, int y) {
    return x >= 0 && x < 8 && y >= 0 && y < 8;
}

//core logic: check if the current player's king is under attack
bool is_in_check(const struct chess_board *board, enum chess_player player) {
    int kx = -1, ky = -1;

    //1.find the king
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (board->board_array[y][x].piece_type == PIECE_KING &&
                board->board_array[y][x].colour == player) {
                kx = x;
                ky = y;
                break;
            }
        }
        if (kx != -1) break;
    }

    if (kx == -1) return false;

    enum chess_player enemy = (player == PLAYER_WHITE) ? PLAYER_BLACK : PLAYER_WHITE;

    //2.check straight lines (rook/queen)
    int dirs_straight[4][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
    for (int i = 0; i < 4; i++) {
        for (int dist = 1; dist < 8; dist++) {
            int nx = kx + dirs_straight[i][0] * dist;
            int ny = ky + dirs_straight[i][1] * dist;
            
            if (!is_valid_pos(nx, ny)) break;

            struct chess_piece p = board->board_array[ny][nx];
            if (p.piece_type == PIECE_EMPTY) continue;
            
            if (p.colour == player) break; //blocked by own piece
            if (p.colour == enemy) {
                if (p.piece_type == PIECE_ROOK || p.piece_type == PIECE_QUEEN) return true;
                break; //blocked by non-attacking enemy
            }
        }
    }

    //3.check diagonals (bishop/queen)
    int dirs_diag[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    for (int i = 0; i < 4; i++) {
        for (int dist = 1; dist < 8; dist++) {
            int nx = kx + dirs_diag[i][0] * dist;
            int ny = ky + dirs_diag[i][1] * dist;
            
            if (!is_valid_pos(nx, ny)) break;

            struct chess_piece p = board->board_array[ny][nx];
            if (p.piece_type == PIECE_EMPTY) continue;
            
            if (p.colour == player) break;
            if (p.colour == enemy) {
                if (p.piece_type == PIECE_BISHOP || p.piece_type == PIECE_QUEEN) return true;
                break;
            }
        }
    }

    //4.check knights
    int knight_moves[8][2] = {{1, 2}, {1, -2}, {-1, 2}, {-1, -2}, 
                              {2, 1}, {2, -1}, {-2, 1}, {-2, -1}};
    for (int i = 0; i < 8; i++) {
        int nx = kx + knight_moves[i][0];
        int ny = ky + knight_moves[i][1];
        if (is_valid_pos(nx, ny)) {
            struct chess_piece p = board->board_array[ny][nx];
            if (p.colour == enemy && p.piece_type == PIECE_KNIGHT) return true;
        }
    }

    //5.check pawns
    int p_dir = (enemy == PLAYER_WHITE) ? -1 : 1; 
    int p_cols[2] = {-1, 1};
    for (int i = 0; i < 2; i++) {
        int nx = kx + p_cols[i];
        int ny = ky + p_dir;
        if (is_valid_pos(nx, ny)) {
            struct chess_piece p = board->board_array[ny][nx];
            if (p.colour == enemy && p.piece_type == PIECE_PAWN) return true;
        }
    }

    return false;
}

//helper:check if player has any legal moves (placeholder)
bool has_legal_moves(const struct chess_board *board, enum chess_player player) {
    return true;
}

void board_summarize(const struct chess_board *board) {
    //determine whose turn it is
    enum chess_player current_player = board->next_move_player;
    
    //check game status
    bool in_check = is_in_check(board, current_player);
    bool can_move = false;//has_legal_moves(board, current_player);

    if (can_move) {
        printf("game incomplete\n");
    } else {
        if (in_check) {
            if (current_player == PLAYER_WHITE)
                printf("White wins by checkmate\n");
            else
                printf("Black wins by checkmate\n");
        } else {
            printf("draw by stalemate\n");
        }
    }
}

//d4 Nf6 Bf4 g6 e3 Bg7 Bd3 d5 Nd2 c6 c3 Qb6 Qb3 Nbd7 Ngf3 Nh5 Qxb6 axb6 h3 Nxf4 exf4 Nf6 a3 O-O O-O Nh5 g3 Bxh3 Rfe1 e6 c4 Bg4 cxd5 exd5 Ne5 Bxe5 dxe5 c5 f3 Bd7 g4 Nxf4 Bc2 Bb5 Nb1 Rfe8 Nc3 Ba6 Ba4 Re7 Rad1 d4 Ne4 Kg7 Nd6 Nd3 Re2 Nxe5 Rf2 Nd3 Rg2 Nf4 Rh2 Ne2 Kg2 Nf4 Kg3 Nd5 Rdh1 Rh8 Bc2 Ne3 Kf4 Nxc2 Rxc2 Re2 Rcc1 Rxb2 Ne4 Rd8 Ng3 d3 Ne4 d2 Rcd1 Rd4 Ke5 Be2 Rxh7 Kxh7 Ng5 Kg7 Rh1 f6 Ke6 fxg5 a4 Bxf3 Rg1 d1=Q Rxd1 Rxd1 a5 Re2
