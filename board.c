#include "board.h"

#include <stddef.h>
#include <stdio.h>

#include "panic.h"

const char *player_string(enum chess_player player)
{
    switch (player)
    {
    case PLAYER_WHITE:
        return "white";
    case PLAYER_BLACK:
        return "black";
    }
}

const char *piece_string(enum piece_type piece)
{
    switch (piece)
    {
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

void board_initialize(struct chess_board *board)
{
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
    board->board_array[0][0].piece_type = PIECE_ROOK;   board->board_array[0][0].colour = PLAYER_WHITE;
    board->board_array[0][1].piece_type = PIECE_KNIGHT; board->board_array[0][1].colour = PLAYER_WHITE;
    board->board_array[0][2].piece_type = PIECE_BISHOP; board->board_array[0][2].colour = PLAYER_WHITE;
    board->board_array[0][3].piece_type = PIECE_QUEEN;  board->board_array[0][3].colour = PLAYER_WHITE;
    board->board_array[0][4].piece_type = PIECE_KING;   board->board_array[0][4].colour = PLAYER_WHITE;
    board->board_array[0][5].piece_type = PIECE_BISHOP; board->board_array[0][5].colour = PLAYER_WHITE;
    board->board_array[0][6].piece_type = PIECE_KNIGHT; board->board_array[0][6].colour = PLAYER_WHITE;
    board->board_array[0][7].piece_type = PIECE_ROOK;   board->board_array[0][7].colour = PLAYER_WHITE;

    // Black back rank (rank 8)
    board->board_array[7][0].piece_type = PIECE_ROOK;   board->board_array[7][0].colour = PLAYER_BLACK;
    board->board_array[7][1].piece_type = PIECE_KNIGHT; board->board_array[7][1].colour = PLAYER_BLACK;
    board->board_array[7][2].piece_type = PIECE_BISHOP; board->board_array[7][2].colour = PLAYER_BLACK;
    board->board_array[7][3].piece_type = PIECE_QUEEN;  board->board_array[7][3].colour = PLAYER_BLACK;
    board->board_array[7][4].piece_type = PIECE_KING;   board->board_array[7][4].colour = PLAYER_BLACK;
    board->board_array[7][5].piece_type = PIECE_BISHOP; board->board_array[7][5].colour = PLAYER_BLACK;
    board->board_array[7][6].piece_type = PIECE_KNIGHT; board->board_array[7][6].colour = PLAYER_BLACK;
    board->board_array[7][7].piece_type = PIECE_ROOK;   board->board_array[7][7].colour = PLAYER_BLACK;
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

    if (move->piece_type == PIECE_PAWN) {
        // ... your existing pawn logic here ...
    }

    else if (move->piece_type == PIECE_ROOK) {
        int candidates_x[8], candidates_y[8];
        int count = 0;

        // --- Search along rank (horizontal) ---
        for (int x = 0; x < 8; x++) {
            if (x == move->target_square_x) continue;
            struct chess_piece p = board->board_array[move->target_square_y][x];
            if (p.piece_type == PIECE_ROOK && p.colour == board->next_move_player) {
                // Check path clear between x and target_square_x
                int step = (x < move->target_square_x) ? 1 : -1;
                bool clear = true;
                for (int i = x + step; i != move->target_square_x; i += step) {
                    if (board->board_array[move->target_square_y][i].piece_type != PIECE_EMPTY) {
                        clear = false;
                        break;
                    }
                }
                if (clear) {
                    candidates_x[count] = x;
                    candidates_y[count] = move->target_square_y;
                    count++;
                }
            }
        }

        // --- Search along file (vertical) ---
        for (int y = 0; y < 8; y++) {
            if (y == move->target_square_y) continue;
            struct chess_piece p = board->board_array[y][move->target_square_x];
            if (p.piece_type == PIECE_ROOK && p.colour == board->next_move_player) {
                // Check path clear between y and target_square_y
                int step = (y < move->target_square_y) ? 1 : -1;
                bool clear = true;
                for (int i = y + step; i != move->target_square_y; i += step) {
                    if (board->board_array[i][move->target_square_x].piece_type != PIECE_EMPTY) {
                        clear = false;
                        break;
                    }
                }
                if (clear) {
                    candidates_x[count] = move->target_square_x;
                    candidates_y[count] = y;
                    count++;
                }
            }
        }

        if (count == 0) {
            panicf("move completion error: %s rook to %c%d (no rook can move)",
                   player_string(board->next_move_player),
                   'a' + move->target_square_x,
                   move->target_square_y + 1);
        } else if (count > 1 && move->source_x == -1 && move->source_y == -1) {
            panicf("move completion error: %s rook to %c%d (ambiguous rook move, source not specified)",
                   player_string(board->next_move_player),
                   'a' + move->target_square_x,
                   move->target_square_y + 1);
        } else {
            int src_file, src_rank;
            if (count == 1) {
                src_file = candidates_x[0];
                src_rank = candidates_y[0];
            } else {
                // Use provided source_x/source_y to disambiguate
                src_file = move->source_x;
                src_rank = move->source_y;
            }
            move->source_x = src_file;
            move->source_y = src_rank;
            move->moving_piece = board->board_array[src_rank][src_file];
        }
    }
}





void board_apply_move(struct chess_board *board, const struct chess_move *move) {
//if a piece is captured, the target square needs to be reset to empty
   if (move -> capture) {
       board->board_array[move->target_square_y][move->target_square_x] = empty_piece;
   }
//move piece to another square while replacing the source square with an empty space
        board->board_array[move->target_square_y][move->target_square_x] = move->moving_piece;
        board->board_array[move-> source_y][move-> source_x] = empty_piece;


    // TODO: apply a completed move to the board.

    // The final step is to update the turn of players in the board state.
    switch (board->next_move_player)
    {
    case PLAYER_WHITE:
        board->next_move_player = PLAYER_BLACK;
        break;
    case PLAYER_BLACK:
        board->next_move_player = PLAYER_WHITE;
        break;
    }
}

#include <stdio.h>
#include "board.h"

// Helper: convert piece to char
char piece_char(struct chess_piece p) {
    if (p.piece_type == PIECE_EMPTY) return '.';

    char c;
    switch (p.piece_type) {
        case PIECE_PAWN:   c = 'P'; break;
        case PIECE_KNIGHT: c = 'N'; break;
        case PIECE_BISHOP: c = 'B'; break;
        case PIECE_ROOK:   c = 'R'; break;
        case PIECE_QUEEN:  c = 'Q'; break;
        case PIECE_KING:   c = 'K'; break;
        default:           c = '?'; break;
    }

    // White = uppercase, Black = lowercase
    if (p.colour == PLAYER_BLACK) {
        c = (char)tolower(c);
    }
    return c;
}

// Draw the board
void board_draw(const struct chess_board *board) {
    printf("\n   a b c d e f g h\n");
    printf("  -----------------\n");

    for (int y = 7; y >= 0; y--) {   // rank 8 down to 1
        printf("%d| ", y + 1);
        for (int x = 0; x < 8; x++) {
            printf("%c ", piece_char(board->board_array[y][x]));
        }
        printf("|%d\n", y + 1);
    }

    printf("  -----------------\n");
    printf("   a b c d e f g h\n\n");
}
//helper funct
void board_summarize(const struct chess_board *board)
{
    board_draw(board);
    // TODO: print the state of the game.
    //determine whose turn it is
    enum chess_player current_player = board->next_move_player;
    //is the current player's king under attack?
    bool in_check = false;
    //Does the current player have any legal moves left?
    bool can_move = true;
    if (can_move) {
        printf ("game incomplete\n");
    }
    else {
        //the player has no legal moves，we need to check if it's checkmate or stalemate

        if (in_check) {
            if (current_player == PLAYER_WHITE)
                printf ("black wins by checkmate\n");
            else
                printf("white wins by checkmate\n");

        }

        else {
            printf("draw by stalemate\n");
        }


    }


}



