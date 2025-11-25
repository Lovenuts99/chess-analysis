#ifndef APSC143__BOARD_H
#define APSC143__BOARD_H
#include <stdbool.h>

enum chess_player
{
    PLAYER_WHITE,
    PLAYER_BLACK,
    PLAYER_EMPTY
};

// Gets a lowercase string denoting the player.
const char *player_string(enum chess_player player);


enum piece_type
{
    PIECE_PAWN,
    PIECE_KNIGHT,
    PIECE_BISHOP,
    PIECE_ROOK,
    PIECE_QUEEN,
    PIECE_KING,
    PIECE_EMPTY
};

enum castle {
    CASTLE_NONE,
    CASTLE_KINGSIDE,
    CASTLE_QUEENSIDE
};

struct chess_piece {
    enum piece_type piece_type;
    enum chess_player colour;
};

extern struct chess_piece empty_piece;

// Gets a lowercase string denoting the piece type.
const char *piece_string(enum piece_type piece);

struct chess_board
{
    enum chess_player next_move_player;
    struct chess_piece board_array[8][8];
    
    //en passant moves are included in here becase they are determined by the previous move, controlled by 1 and 0. 
    int en_passant_available;  
    int en_passant_x;           
    int en_passant_y;   

    // TODO: what other fields are needed?


};

struct chess_move
{

    struct chess_piece moving_piece;

    enum piece_type piece_type;


    //Info on where the square is
    bool source_known; //Is the source known? yes or no
    int source_square; //What square does the source exist in the array
    int source_y; //from side of the board
    bool source_column_check; //Did the user enter this info
    int source_x; // from bottom of the board
    bool source_row_check;//Did the user enter this info

    //destination info
    int target_square_x;
    int target_square_y;

    //Move variables/potential scenarios
    bool capture;
    bool promotion;
    enum piece_type promotion_piece;
    bool en_passant;
    enum castle castling;



    // TODO: what other fields are needed?
};

// Initializes the state of the board for a new chess game.
void board_initialize(struct chess_board *board);

// Determine which piece is moving, and complete the move data accordingly.
// Panics if there is no piece which can make the specified move, or if there
// are multiple possible pieces.
void board_complete_move(const struct chess_board *board, struct chess_move *move);

// Apply move to the board. The move must already be complete, i.e., the initial
// square must be known. Panics if the move is not legal in the current board
// position.
void board_apply_move(struct chess_board *board, const struct chess_move *move);

// Classify the state of the board, printing one of the following:
// - game incomplete
// - white wins by checkmate
// - black wins by checkmate
// - draw by stalemate
void board_summarize(const struct chess_board *board);

#endif
