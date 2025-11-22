    #include "parser.h"
#include <stdio.h>
#include "panic.h"




bool parse_move(struct chess_move *move)
{
    // Reset move fields
    move->capture = false;
    move->source_x = -1;
    move->source_y = -1;

    char c;

    // Skip leading spaces
    do {
        c = getc(stdin);
    } while (c == ' ');

    // End of input
    if (c == '\n' || c == '\r') {
        return false;
    }

    // --- Pawn moves (lowercase file letter) ---
    if (c >= 'a' && c <= 'h') {
        move->piece_type = PIECE_PAWN;

        char next_c = getc(stdin);

        // Pawn capture like "exd5"
        if (next_c == 'x') {
            move->source_x = c - 'a';   // origin file specified
            move->capture = true;
            c = getc(stdin);            // target file
            next_c = getc(stdin);       // target rank
        }

        move->target_square_x = c - 'a';

        if (next_c >= '1' && next_c <= '8') {
            move->target_square_y = next_c - '1';
            return true;
        } else {
            panicf("parse error at character '%c'\n", next_c);
            return false;
        }
    }

    // --- Piece moves (uppercase letter) ---
    if (c >= 'A' && c <= 'Z') {
        switch (c) {
            case 'K': move->piece_type = PIECE_KING;   break;
            case 'Q': move->piece_type = PIECE_QUEEN;  break;
            case 'R': move->piece_type = PIECE_ROOK;   break;
            case 'B': move->piece_type = PIECE_BISHOP; break;
            default:
                panicf("parse error: unknown piece '%c'\n", c);
                return false;
        }

        c = getc(stdin);
        char next_c = getc(stdin);

        // Origin + capture like "Nbd2" or "R1xd4"
        if (next_c == 'x' && ((c >= 'a' && c <= 'h') || (c >= '1' && c <= '8'))) {
            move->capture = true;
            if (c >= 'a' && c <= 'h') {
                move->source_x = c - 'a';
            } else {
                move->source_y = c - '1';
            }
            c = getc(stdin);      // target file
            next_c = getc(stdin); // target rank
        }

        // Simple capture like "Qxd5"
        if (c == 'x') {
            move->capture = true;
            c = next_c;           // target file
            next_c = getc(stdin); // target rank
        }

        if (c >= 'a' && c <= 'h') {
            move->target_square_x = c - 'a';
            if (next_c >= '1' && next_c <= '8') {
                move->target_square_y = next_c - '1';
                return true;
            } else {
                panicf("parse error at character '%c'\n", next_c);
                return false;
            }
        } else {
            panicf("parse error at character '%c'\n", c);
            return false;
        }
    }

    // If we reach here, input was invalid
    panicf("parse error: unexpected character '%c'\n", c);
    return false;
}




