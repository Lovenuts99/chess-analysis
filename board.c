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


struct chess_piece empty_piece = {
    .piece_type = PIECE_EMPTY,
    .colour = PLAYER_EMPTY,
};

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

struct chess_move make_blank_move(void) {
    struct chess_move m;

    // Explicit defaults
    m.castling = CASTLE_NONE; // no castling by default
    m.promotion_piece = PIECE_EMPTY; // no promotion piece chosen
    m.source_known = false;
    m.source_column_check = false;
    m.source_row_check = false;
    m.capture = false;
    m.promotion = false;
    m.en_passant = false;

    return m;
}


//initializes board with propper piece order as well as empty squares
void board_initialize(struct chess_board *board) {
    board->next_move_player = PLAYER_WHITE;
    board->castle_kingside_black = true;
    board->castle_kingside_white = true;
    board->castle_queenside_black = true;
    board->castle_queenside_white = true;

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
void board_complete_move(struct chess_board *board, struct chess_move *move) {
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
            //if the specified target square is one behind a pawn that just preformed a 2 square push
            if (board->en_passant_available &&
                move->target_square_x == board->en_passant_x &&
                move->target_square_y == board->en_passant_y) {
                // if a rank was provided, scan the square in the rank diagonal to the target location in the proper
                //direction to ensure there is a pawn that the move can be preformed on
                if (move->source_x != -1) {
                    int dy = (board->next_move_player == PLAYER_WHITE) ? -1 : 1;
                    struct chess_piece temp = board->board_array[move->target_square_y + dy][move->source_x];
                    if (temp.piece_type == PIECE_PAWN && temp.colour == board->next_move_player) {
                        move->source_y = move->target_square_y + dy;
                        move->en_passant = true;
                        return;
                    }
                    //error if the square doesn't contain a pawn
                    panicf("move completion error: %s %s to %c%d (no pawn can capture)",
                           player_string(board->next_move_player),
                           piece_string(move->piece_type),
                           'a' + move->target_square_x,
                           move->target_square_y + 1);
                }

                //if no source was provided
                else {
                    int candidates_x[2] = {-1, -1}; // x_positions of pawns that the move can be preformed on
                    int dy = (board->next_move_player == PLAYER_WHITE) ? -1 : 1;
                    // the 'backwards' direction relative to the colour making the move

                    //check diagonally left
                    if (move->target_square_x - 1 >= 0) {
                        struct chess_piece temp = board->board_array[move->target_square_y + dy][move->source_x - 1];
                        if (temp.piece_type == PIECE_PAWN && temp.colour == board->next_move_player);
                        candidates_x[0] = move->target_square_x - 1;
                    }

                    // check diagonally right
                    if (move->target_square_x + 1 <= 7) {
                        struct chess_piece temp = board->board_array[move->target_square_y + dy][move->source_x + 1];
                        if (temp.piece_type == PIECE_PAWN && temp.colour == board->next_move_player);
                        candidates_x[1] = move->target_square_x + 1;
                    }

                    // ambiguous: two pawns can preform the move -- error
                    if (candidates_x[0] != -1 && candidates_x[1] != -1) {
                        panicf("move completion error: %s bishop to %c%d (ambiguous bishop move, source not specified)",
                               player_string(board->next_move_player),
                               'a' + move->target_square_x,
                               move->target_square_y + 1);
                    } else {
                        int temp = candidates_x[0] + candidates_x[1] + 1;
                        // if no pawns can make the move, throw error
                        if (temp == -1) {
                            panicf("move completion error: %s %s to %c%d (no pawn can capture)",
                                   player_string(board->next_move_player),
                                   piece_string(move->piece_type),
                                   'a' + move->target_square_x,
                                   move->target_square_y + 1);
                        }
                        //confirm the source of the pawn of the singular pawn found
                        move->source_x = temp;
                        move->source_y = move->target_square_y + dy;
                        move->en_passant = true;
                    }
                }

                panicf("move completion error: %s %s to %c%d (no pawn can capture)",
                       player_string(board->next_move_player),
                       piece_string(move->piece_type),
                       'a' + move->target_square_x,
                       move->target_square_y + 1);
            }

            // disable en passant opportunity
            board->en_passant_available = false;


            // throws error if there isn't a pawn to capture
            // comes after en passent check  as the target of an en passant is always empty
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
                //down and to the left
                if (move->target_square_x > 0 &&
                    board->board_array[move->target_square_y - 1][move->target_square_x - 1].piece_type == PIECE_PAWN &&
                    board->board_array[move->target_square_y - 1][move->target_square_x - 1].colour == PLAYER_WHITE) {
                    candidates[count++] = move->target_square_x - 1;
                }
                //down and to the right
                if (move->target_square_x < 7 &&
                    board->board_array[move->target_square_y - 1][move->target_square_x + 1].piece_type == PIECE_PAWN &&
                    board->board_array[move->target_square_y - 1][move->target_square_x + 1].colour == PLAYER_WHITE) {
                    candidates[count++] = move->target_square_x + 1;
                }
            } else {
                //up and to the left
                if (move->target_square_x > 0 &&
                    board->board_array[move->target_square_y + 1][move->target_square_x - 1].piece_type == PIECE_PAWN &&
                    board->board_array[move->target_square_y + 1][move->target_square_x - 1].colour == PLAYER_BLACK) {
                    candidates[count++] = move->target_square_x - 1;
                }
                //up and to the right
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
                int src_rank = (board->next_move_player == PLAYER_WHITE)
                                   ? move->target_square_y - 1
                                   : move->target_square_y + 1;
                move->source_x = src_file;
                move->source_y = src_rank;
                move->moving_piece = board->board_array[src_rank][src_file];
            }
        }

        // if move isn't a capture and just a push
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
                    //enpassant is allowed now for the other player
                    board->en_passant_available = true;
                    //coordinates of the allowed en pasasnt square are stored
                    board->en_passant_x = move->target_square_x;
                    board->en_passant_y = move->target_square_y - 1;
                    move->moving_piece = board->board_array[move->source_y][move->source_x];
                } else {
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
                    board->en_passant_available = true;
                    board->en_passant_x = move->target_square_x;
                    board->en_passant_y = move->target_square_y + 1;
                    move->moving_piece = board->board_array[move->source_y][move->source_x];
                } else {
                    panicf("move completion error: BLACK PAWN to %c%d (no pawn can move)",
                           'a' + move->target_square_x, move->target_square_y + 1);
                }
            }
        }
    }

    // handling for rooks
    else if (move->piece_type == PIECE_ROOK) {
        //target square must be empty or contain opponent piece or else error
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
                                path_clear = false;
                                break;
                            }
                        }
                    } else if (row == move->target_square_y) {
                        // Horizontal move on same rank
                        int step = (move->target_square_x > move->source_x) ? 1 : -1;
                        path_clear = true;
                        // ensures the path is clear from the rook that was found to the target square
                        for (int check_col = move->source_x + step; check_col != move->target_square_x;
                             check_col += step) {
                            if (board->board_array[move->target_square_y][check_col].piece_type != PIECE_EMPTY) {
                                path_clear = false;
                                break;
                            }
                        }
                    }
                    //store move information if rook is available and unobscured
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
                        // reverse scan to the path is clear
                        for (int check_col = col + step; check_col != move->target_square_x; check_col += step) {
                            if (board->board_array[move->source_y][check_col].piece_type != PIECE_EMPTY) {
                                path_clear = false;
                                break;
                            }
                        }
                    } else if (col == move->target_square_x) {
                        // Vertical move on same file
                        int step = (move->target_square_y > move->source_y) ? 1 : -1;
                        path_clear = true;
                        //reverse scan to ensure the path is clear
                        for (int check_row = move->source_y + step; check_row != move->target_square_y;
                             check_row += step) {
                            if (board->board_array[check_row][move->target_square_x].piece_type != PIECE_EMPTY) {
                                path_clear = false;
                                break;
                            }
                        }
                    }

                    // store move information of confirmed piece
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
                        // ensure path to piece is clear
                        bool path_clear = true;
                        for (int check_col = col + 1; check_col < move->target_square_x; check_col++) {
                            if (board->board_array[move->target_square_y][check_col].piece_type != PIECE_EMPTY) {
                                path_clear = false;
                                break;
                            }
                        }
                        //confirm piece location in move structure
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
                                path_clear = false;
                                break;
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
                                path_clear = false;
                                break;
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
                                path_clear = false;
                                break;
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

        // throw error if no rook was found in all 4 directions
        if (!rook_found) {
            panicf("move completion error: %s rook to %c%d (no rook can move)",
                   player_string(board->next_move_player),
                   'a' + move->target_square_x, move->target_square_y + 1);
        }
        if (move->source_x == 0 && board->next_move_player == PLAYER_BLACK) {
            board->castle_queenside_black = false;
        }
        if (move->source_x == 0 && board->next_move_player == PLAYER_BLACK) {
            board->castle_queenside_white = false;
        }
        if (move->source_x == 7 && board->next_move_player == PLAYER_BLACK) {
            board->castle_kingside_black = false;
        }
        if (move->source_x == 7 && board->next_move_player == PLAYER_WHITE) {
            board->castle_kingside_white = false;
        }
    }
    //bishop handling
    else if (move->piece_type == PIECE_BISHOP) {
        int src_x = -1, src_y = -1;
        int found = 0;

        // Scan diagonals
        int directions[4][2] = {{-1, 1}, {1, 1}, {-1, -1}, {1, -1}}; // vector directions, all diagonal
        // iterate through 4 directions
        for (int d = 0; d < 4; d++) {
            int dx = directions[d][0], dy = directions[d][1];
            int x = move->target_square_x + dx;
            int y = move->target_square_y + dy;
            while (x >= 0 && x < 8 && y >= 0 && y < 8) {
                struct chess_piece p = board->board_array[y][x];
                if (p.piece_type != PIECE_EMPTY) {
                    // if piece found isn't a bishop of the correct colour imeditly stop scanning
                    //otherwise store src_x and src_y
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
        }
        //if 2 bishops can make the move
        else if (found > 1) {
            // Use disambiguation if provided
            if (move->source_x != -1 || move->source_y != -1) {
                bool matched = false;
                // scan all 4 directions to locate the bishop with the matching source partial coordinate
                for (int d = 0; d < 4; d++) {
                    int dx = directions[d][0], dy = directions[d][1];
                    int x = move->target_square_x + dx;
                    int y = move->target_square_y + dy;
                    while (x >= 0 && x < 8 && y >= 0 && y < 8) {
                        struct chess_piece p = board->board_array[y][x];
                        if (p.piece_type != PIECE_EMPTY) {
                            // break immediately if cell is occupied by piece other than specified colours bishop
                            // otherwise store pieces information
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

                // if source information provided in the notation is invalid and no bishop can make the move
                if (!matched) {
                    panicf("move completion error: %s bishop to %c%d (disambiguation does not match any bishop)",
                           player_string(board->next_move_player),
                           'a' + move->target_square_x,
                           move->target_square_y + 1);
                }
            }
            // multiple bishops can make the move and is ambiguous
            else {
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

        int offsets[8][2] = {
            {1, 2}, {2, 1}, {-1, 2}, {-2, 1},
            {1, -2}, {2, -1}, {-1, -2}, {-2, -1}
        };

        // Find all knights that can reach target
        // no need to worry if the path is clear
        for (int i = 0; i < 8; i++) {
            int x = move->target_square_x + offsets[i][0];
            int y = move->target_square_y + offsets[i][1];
            if (x >= 0 && x < 8 && y >= 0 && y < 8) {
                struct chess_piece p = board->board_array[y][x];
                if (p.piece_type == PIECE_KNIGHT && p.colour == board->next_move_player) {
                    src_x = x;
                    src_y = y;
                    found++;
                }
            }
        }

        if (found == 0) {
            panicf("move completion error: %s knight to %c%d (no knight can move)",
                   player_string(board->next_move_player),
                   'a' + move->target_square_x,
                   move->target_square_y + 1);
            return;
        }

        // if multiple knights can preform move
        if (found > 1) {
            // if dis ambiguity is provided, use it
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
                                break;
                            }
                        }
                    }
                }
                // throw error if disambiguity isn't accurate
                if (!matched) {
                    panicf("move completion error: %s knight to %c%d (disambiguation does not match any knight)",
                           player_string(board->next_move_player),
                           'a' + move->target_square_x,
                           move->target_square_y + 1);
                    return;
                }
            }
            // throw error if no disambiguity is provided
            else {
                panicf("move completion error: %s knight to %c%d (ambiguous knight move, source not specified)",
                       player_string(board->next_move_player),
                       'a' + move->target_square_x,
                       move->target_square_y + 1);
                return;
            }
        }

        // Finalize only if src_x/src_y are valid
        move->source_x = src_x;
        move->source_y = src_y;
        move->moving_piece = board->board_array[src_y][src_x];
    } else if (move->piece_type == PIECE_KING && move->castling != CASTLE_NONE) {
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
        } else {
            // Queenside
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
    } else if (move->piece_type == PIECE_KING) {
        int src_x = -1, src_y = -1;
        int found = 0;

        // All 8 king move offsets
        int offsets[8][2] = {
            {1, 0}, {-1, 0}, {0, 1}, {0, -1},
            {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
        };

        // Instead of target+offset, check target-offset
        for (int i = 0; i < 8; i++) {
            int x = move->target_square_x - offsets[i][0];
            int y = move->target_square_y - offsets[i][1];
            if (x >= 0 && x < 8 && y >= 0 && y < 8) {
                struct chess_piece p = board->board_array[y][x];
                if (p.piece_type == PIECE_KING && p.colour == board->next_move_player) {
                    src_x = x;
                    src_y = y;
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
        if (board->next_move_player == PLAYER_WHITE) {
            board->castle_kingside_white = false;
            board->castle_queenside_white = false;
        } else {
            board->castle_kingside_black = false;
            board->castle_queenside_black = false;
        }
    } else if (move->piece_type == PIECE_KING && move->castling != CASTLE_NONE) {
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
        } else {
            // Queenside
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


        // Finalize move
        move->source_x = src_x;
        move->source_y = src_y;
        move->target_square_x = dst_x;
        move->target_square_y = dst_y;
        move->moving_piece = king;
        // prevent castling for the player preforming the move since their king has left its original position
        if (board->next_move_player == PLAYER_WHITE) {
            board->castle_kingside_white = false;
            board->castle_queenside_white = false;
        } else {
            board->castle_kingside_black = false;
            board->castle_queenside_black = false;
        }
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
        if (move->en_passant) {
            int dy = (board->next_move_player == PLAYER_WHITE) ? -1 : 1;
            board->board_array[move->target_square_y + dy][move->target_square_x] = empty_piece;
            board->board_array[move->source_y][move->source_x] = empty_piece;
        }
    }

    // Apply castling move: rearrange pieces only
    if (move->piece_type == PIECE_KING && move->castling != CASTLE_NONE) {
        int y = (move->moving_piece.colour == PLAYER_WHITE ? 0 : 7);
        if (move->castling == CASTLE_KINGSIDE) {
            if (board->next_move_player == PLAYER_BLACK && !board->castle_kingside_black || (
                    board->next_move_player == PLAYER_WHITE && !board->castle_kingside_white)) {
                panicf("illegal move: castling piece has been moved already");
            }
            // King: e -> g
            board->board_array[y][6] = board->board_array[y][4];
            board->board_array[y][4].piece_type = PIECE_EMPTY;

            // Rook: h -> f
            board->board_array[y][5] = board->board_array[y][7];
            board->board_array[y][7].piece_type = PIECE_EMPTY;
        } else {
            if (board->next_move_player == PLAYER_BLACK && !board->castle_queenside_black || (
                    board->next_move_player == PLAYER_WHITE && !board->castle_queenside_white)) {
                panicf("illegal move: castling piece has been moved already");
            }
            // Queenside
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


//helper: check if coordinates are valid
bool is_valid_pos(int x, int y) {
    return x >= 0 && x < 8 && y >= 0 && y < 8;
}

//core logic: check if the current player's king is under attack
bool is_in_check(const struct chess_board *board, enum chess_player player) {
    int kx = -1, ky = -1;

    // 1. Find the king
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

    if (kx == -1) return false; // no king found

    enum chess_player enemy = (player == PLAYER_WHITE) ? PLAYER_BLACK : PLAYER_WHITE;

    // 2. Check straight lines (rook/queen)
    int dirs_straight[4][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
    for (int i = 0; i < 4; i++) {
        for (int dist = 1; dist < 8; dist++) {
            int nx = kx + dirs_straight[i][0] * dist;
            int ny = ky + dirs_straight[i][1] * dist;
            if (!is_valid_pos(nx, ny)) break;
            struct chess_piece p = board->board_array[ny][nx];
            if (p.piece_type == PIECE_EMPTY) continue;
            if (p.colour == player) break;
            if (p.colour == enemy) {
                if (p.piece_type == PIECE_ROOK || p.piece_type == PIECE_QUEEN) return true;
                break;
            }
        }
    }

    // 3. Check diagonals (bishop/queen)
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

    // 4. Check knights
    int knight_moves[8][2] = {{1, 2}, {1, -2}, {-1, 2}, {-1, -2}, {2, 1}, {2, -1}, {-2, 1}, {-2, -1}};
    for (int i = 0; i < 8; i++) {
        int nx = kx + knight_moves[i][0];
        int ny = ky + knight_moves[i][1];
        if (is_valid_pos(nx, ny)) {
            struct chess_piece p = board->board_array[ny][nx];
            if (p.colour == enemy && p.piece_type == PIECE_KNIGHT) return true;
        }
    }

    // 5. Check pawns
    int p_dir = (enemy == PLAYER_WHITE) ? 1 : -1; // corrected
    int p_cols[2] = {-1, 1};
    for (int i = 0; i < 2; i++) {
        int nx = kx + p_cols[i];
        int ny = ky + p_dir;
        if (is_valid_pos(nx, ny)) {
            struct chess_piece p = board->board_array[ny][nx];
            if (p.colour == enemy && p.piece_type == PIECE_PAWN) return true;
        }
    }

    // 6. Check enemy king adjacency
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;
            int nx = kx + dx, ny = ky + dy;
            if (is_valid_pos(nx, ny)) {
                struct chess_piece p = board->board_array[ny][nx];
                if (p.colour == enemy && p.piece_type == PIECE_KING) return true;
            }
        }
    }

    return false;
}

int generate_moves_for_piece(const struct chess_board *board,
                             int x, int y,
                             struct chess_piece piece,
                             struct chess_move moves[64]) {
    int count = 0;
    enum chess_player player = piece.colour;
    enum chess_player enemy = (player == PLAYER_WHITE) ? PLAYER_BLACK : PLAYER_WHITE;

    // --- Pawn ---
    if (piece.piece_type == PIECE_PAWN) {
        int dir = (player == PLAYER_WHITE) ? 1 : -1;
        int start_rank = (player == PLAYER_WHITE) ? 1 : 6;
        int promote_rank = (player == PLAYER_WHITE) ? 7 : 0;

        // Single push
        int ny = y + dir;
        if (is_valid_pos(x, ny) && board->board_array[ny][x].piece_type == PIECE_EMPTY) {
            moves[count++] = (struct chess_move){
                .source_x = x,
                .source_y = y,
                .target_square_x = x,
                .target_square_y = ny,
                .capture = false,
                .promotion = false,
                .piece_type = PIECE_PAWN
            };
            if (ny == promote_rank) moves[count - 1].promotion = true;

            // Double push
            if (y == start_rank) {
                int ny2 = y + 2 * dir;
                if (is_valid_pos(x, ny2) && board->board_array[ny2][x].piece_type == PIECE_EMPTY) {
                    moves[count++] = (struct chess_move){
                        .source_x = x,
                        .source_y = y,
                        .target_square_x = x,
                        .target_square_y = ny2,
                        .capture = false,
                        .promotion = false,
                        .piece_type = PIECE_PAWN
                    };
                }
            }
        }

        // Captures + en passant
        for (int dx = -1; dx <= 1; dx += 2) {
            int nx = x + dx, ny = y + dir;
            if (is_valid_pos(nx, ny)) {
                struct chess_piece target = board->board_array[ny][nx];
                if (target.piece_type != PIECE_EMPTY && target.colour == enemy) {
                    moves[count++] = (struct chess_move){
                        .source_x = x,
                        .source_y = y,
                        .target_square_x = nx,
                        .target_square_y = ny,
                        .capture = true,
                        .promotion = false,
                        .piece_type = PIECE_PAWN
                    };
                    if (ny == promote_rank) moves[count - 1].promotion = true;
                }
                if (board->en_passant_available &&
                    nx == board->en_passant_x && ny == board->en_passant_y) {
                    moves[count++] = (struct chess_move){
                        .source_x = x,
                        .source_y = y,
                        .target_square_x = nx,
                        .target_square_y = ny,
                        .capture = true,
                        .en_passant = true,
                        .piece_type = PIECE_PAWN
                    };
                }
            }
        }
    }

    // --- Knight ---
    else if (piece.piece_type == PIECE_KNIGHT) {
        int deltas[8][2] = {
            {1, 2}, {2, 1}, {-1, 2}, {-2, 1},
            {1, -2}, {2, -1}, {-1, -2}, {-2, -1}
        };
        for (int i = 0; i < 8; i++) {
            int nx = x + deltas[i][0];
            int ny = y + deltas[i][1];
            if (is_valid_pos(nx, ny)) {
                struct chess_piece target = board->board_array[ny][nx];
                if (target.colour != player) {
                    moves[count++] = (struct chess_move){
                        .source_x = x,
                        .source_y = y,
                        .target_square_x = nx,
                        .target_square_y = ny,
                        .capture = (target.piece_type != PIECE_EMPTY),
                        .promotion = false,
                        .piece_type = PIECE_KNIGHT
                    };
                }
            }
        }
    }

    // --- Bishop ---
    else if (piece.piece_type == PIECE_BISHOP) {
        int dirs[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
        for (int i = 0; i < 4; i++) {
            for (int dist = 1; dist < 8; dist++) {
                int nx = x + dirs[i][0] * dist, ny = y + dirs[i][1] * dist;
                if (!is_valid_pos(nx, ny)) break;
                struct chess_piece target = board->board_array[ny][nx];
                if (target.piece_type == PIECE_EMPTY) {
                    moves[count++] = (struct chess_move){
                        .source_x = x,
                        .source_y = y,
                        .target_square_x = nx,
                        .target_square_y = ny,
                        .capture = false,
                        .promotion = false,
                        .piece_type = PIECE_BISHOP
                    };
                } else {
                    if (target.colour == enemy)
                        moves[count++] = (struct chess_move){
                            .source_x = x,
                            .source_y = y,
                            .target_square_x = nx,
                            .target_square_y = ny,
                            .capture = true,
                            .promotion = false,
                            .piece_type = PIECE_BISHOP
                        };
                    break;
                }
            }
        }
    }

    // --- Rook ---
    else if (piece.piece_type == PIECE_ROOK) {
        int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (int i = 0; i < 4; i++) {
            for (int dist = 1; dist < 8; dist++) {
                int nx = x + dirs[i][0] * dist, ny = y + dirs[i][1] * dist;
                if (!is_valid_pos(nx, ny)) break;
                struct chess_piece target = board->board_array[ny][nx];
                if (target.piece_type == PIECE_EMPTY) {
                    moves[count++] = (struct chess_move){x, y, nx, ny,false,false, PIECE_ROOK};
                } else {
                    if (target.colour == enemy)
                        moves[count++] = (struct chess_move){
                            .source_x = x,
                            .source_y = y,
                            .target_square_x = nx,
                            .target_square_y = ny,
                            .capture = true,
                            .promotion = false,
                            .piece_type = PIECE_ROOK
                        };
                    break;
                }
            }
        }
    }

    // --- Queen ---
    else if (piece.piece_type == PIECE_QUEEN) {
        int dirs[8][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (int i = 0; i < 8; i++) {
            for (int dist = 1; dist < 8; dist++) {
                int nx = x + dirs[i][0] * dist, ny = y + dirs[i][1] * dist;
                if (!is_valid_pos(nx, ny)) break;
                struct chess_piece target = board->board_array[ny][nx];
                if (target.piece_type == PIECE_EMPTY) {
                    //
                } else {
                    if (target.colour == enemy)
                        moves[count++] = (struct chess_move){
                            .source_x = x,
                            .source_y = y,
                            .target_square_x = nx,
                            .target_square_y = ny,
                            .capture = true,
                            .promotion = false,
                            .piece_type = PIECE_QUEEN
                        };
                    break;
                }
            }
        }
    } else if (piece.piece_type == PIECE_KING) {
        // Normal king moves (one square in any direction)
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) continue;
                int nx = x + dx, ny = y + dy;
                if (is_valid_pos(nx, ny)) {
                    struct chess_piece target = board->board_array[ny][nx];
                    if (target.colour != player) {
                        moves[count++] = (struct chess_move){
                            .source_x = x,
                            .source_y = y,
                            .target_square_x = nx,
                            .target_square_y = ny,
                            .capture = (target.piece_type != PIECE_EMPTY),
                            .promotion = false,
                            .piece_type = PIECE_KING,
                            .moving_piece = board->board_array[y][x]
                        };
                    }
                }
            }
        }

        // --- Castling ---
        if (player == PLAYER_WHITE && x == 4 && y == 0) {
            // king on e1
            // Kingside castle (e1 → g1, rook h1 → f1)
            if (board->board_array[0][7].piece_type == PIECE_ROOK &&
                board->board_array[0][7].colour == PLAYER_WHITE &&
                board->board_array[0][5].piece_type == PIECE_EMPTY &&
                board->board_array[0][6].piece_type == PIECE_EMPTY &&
                !is_in_check(board, PLAYER_WHITE)) {
                // simulate king stepping to f1
                struct chess_board temp = *board;
                temp.board_array[0][5] = temp.board_array[0][4];
                temp.board_array[0][4].piece_type = PIECE_EMPTY;
                if (!is_in_check(&temp, PLAYER_WHITE)) {
                    // simulate king stepping to g1
                    temp.board_array[0][6] = temp.board_array[0][5];
                    temp.board_array[0][5].piece_type = PIECE_EMPTY;
                    if (!is_in_check(&temp, PLAYER_WHITE)) {
                        moves[count++] = (struct chess_move){
                            .source_x = 4,
                            .source_y = 0,
                            .target_square_x = 6,
                            .target_square_y = 0,
                            .capture = false,
                            .promotion = false,
                            .piece_type = PIECE_KING,
                            .castling = CASTLE_KINGSIDE
                        };
                        moves[count - 1].castling = true;
                    }
                }
            }
            // Queenside castle (e1 → c1, rook a1 → d1)
            if (board->board_array[0][0].piece_type == PIECE_ROOK &&
                board->board_array[0][0].colour == PLAYER_WHITE &&
                board->board_array[0][1].piece_type == PIECE_EMPTY &&
                board->board_array[0][2].piece_type == PIECE_EMPTY &&
                board->board_array[0][3].piece_type == PIECE_EMPTY &&
                !is_in_check(board, PLAYER_WHITE)) {
                struct chess_board temp = *board;
                temp.board_array[0][3] = temp.board_array[0][4]; // king to d1
                temp.board_array[0][4].piece_type = PIECE_EMPTY;
                if (!is_in_check(&temp, PLAYER_WHITE)) {
                    temp.board_array[0][2] = temp.board_array[0][3]; // king to c1
                    temp.board_array[0][3].piece_type = PIECE_EMPTY;
                    if (!is_in_check(&temp, PLAYER_WHITE)) {
                        moves[count++] = (struct chess_move){
                            .source_x = 4,
                            .source_y = 0,
                            .target_square_x = 2,
                            .target_square_y = 0,
                            .capture = false,
                            .promotion = false,
                            .piece_type = PIECE_KING,
                            .castling = CASTLE_QUEENSIDE
                        };
                        moves[count - 1].castling = true;
                    }
                }
            }
        } else if (player == PLAYER_BLACK && x == 4 && y == 7) {
            // king on e8
            // Kingside castle (e8 → g8, rook h8 → f8)
            if (board->board_array[7][7].piece_type == PIECE_ROOK &&
                board->board_array[7][7].colour == PLAYER_BLACK &&
                board->board_array[7][5].piece_type == PIECE_EMPTY &&
                board->board_array[7][6].piece_type == PIECE_EMPTY &&
                !is_in_check(board, PLAYER_BLACK)) {
                struct chess_board temp = *board;
                temp.board_array[7][5] = temp.board_array[7][4]; // king to f8
                temp.board_array[7][4].piece_type = PIECE_EMPTY;
                if (!is_in_check(&temp, PLAYER_BLACK)) {
                    temp.board_array[7][6] = temp.board_array[7][5]; // king to g8
                    temp.board_array[7][5].piece_type = PIECE_EMPTY;
                    if (!is_in_check(&temp, PLAYER_BLACK)) {
                        moves[count++] = (struct chess_move){
                            .source_x = 4,
                            .source_y = 7,
                            .target_square_x = 6,
                            .target_square_y = 7,
                            .capture = false,
                            .promotion = false,
                            .piece_type = PIECE_KING,
                            .castling = CASTLE_KINGSIDE
                        };
                        moves[count - 1].castling = true;
                    }
                }
            }
            // Queenside castle (e8 → c8, rook a8 → d8)
            if (board->board_array[7][0].piece_type == PIECE_ROOK &&
                board->board_array[7][0].colour == PLAYER_BLACK &&
                board->board_array[7][1].piece_type == PIECE_EMPTY &&
                board->board_array[7][2].piece_type == PIECE_EMPTY &&
                board->board_array[7][3].piece_type == PIECE_EMPTY &&
                !is_in_check(board, PLAYER_BLACK)) {
                struct chess_board temp = *board;
                temp.board_array[7][3] = temp.board_array[7][4]; // king to d8
                temp.board_array[7][4].piece_type = PIECE_EMPTY;
                if (!is_in_check(&temp, PLAYER_BLACK)) {
                    temp.board_array[7][2] = temp.board_array[7][3]; // king to c8
                    temp.board_array[7][3].piece_type = PIECE_EMPTY;
                    if (!is_in_check(&temp, PLAYER_BLACK)) {
                        moves[count++] = (struct chess_move){
                            .source_x = 4,
                            .source_y = 7,
                            .target_square_x = 2,
                            .target_square_y = 7,
                            .capture = false,
                            .promotion = false,
                            .piece_type = PIECE_KING,
                            .castling = CASTLE_QUEENSIDE
                        };
                        moves[count - 1].castling = true;
                    }
                }
            }
        }
    }

    return count;
}

bool has_legal_moves(const struct chess_board *board, enum chess_player player) {
    // Loop through all squares
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            struct chess_piece piece = board->board_array[y][x];

            // Skip empty squares and opponent pieces
            if (piece.piece_type == PIECE_EMPTY || piece.colour != player) {
                continue;
            }

            // Generate pseudo-legal moves for this piece
            struct chess_move moves[64];
            int move_count = generate_moves_for_piece(board, x, y, piece, moves);

            // Test each move
            for (int i = 0; i < move_count; i++) {
                struct chess_board temp = *board; // copy board
                board_apply_move(&temp, &moves[i]); // apply move

                // If king is safe after this move, it's legal
                if (!is_in_check(&temp, player)) {
                    return true;
                }
            }
        }
    }
    // No legal moves found
    return false;
}


void board_summarize(const struct chess_board *board) {
    enum chess_player current_player = board->next_move_player;

    bool in_check = is_in_check(board, current_player);

    if (in_check) {
        // Current player is checkmated → opponent wins
        if (current_player == PLAYER_WHITE)
            printf("Black wins by checkmate\n");
        else
            printf("White wins by checkmate\n");
    } else {
        if (!has_legal_moves(board, current_player)) {
            printf("draw by stalemate\n");
        } else {
            printf("game incomplete");
        }
    }
}

//d4 Nf6 Bf4 g6 e3 Bg7 Bd3 d5 Nd2 c6 c3 Qb6 Qb3 Nbd7 Ngf3 Nh5 Qxb6 axb6 h3 Nxf4 exf4 Nf6 a3 O-O O-O Nh5 g3 Bxh3 Rfe1 e6 c4 Bg4 cxd5 exd5 Ne5 Bxe5 dxe5 c5 f3 Bd7 g4 Nxf4 Bc2 Bb5 Nb1 Rfe8 Nc3 Ba6 Ba4 Re7 Rad1 d4 Ne4 Kg7 Nd6 Nd3 Re2 Nxe5 Rf2 Nd3 Rg2 Nf4 Rh2 Ne2 Kg2 Nf4 Kg3 Nd5 Rdh1 Rh8 Bc2 Ne3 Kf4 Nxc2 Rxc2 Re2 Rcc1 Rxb2 Ne4 Rd8 Ng3 d3 Ne4 d2 Rcd1 Rd4 Ke5 Be2 Rxh7 Kxh7 Ng5 Kg7 Rh1 f6 Ke6 fxg5 a4 Bxf3 Rg1 d1=Q Rxd1 Rxd1 a5 Re2
