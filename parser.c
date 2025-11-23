#include "parser.h"
#include <stdio.h>
#include "panic.h"

bool parse_move(struct chess_move *move)
{
    // Reset move fields
    move->capture   = false;
    move->source_x  = -1;
    move->source_y  = -1;
    move->castling  = CASTLE_NONE;
    move->promotion = PIECE_EMPTY;   // new field for promotion

    char c;

    // Skip leading spaces
    do {
        c = getc(stdin);
    } while (c == ' ');

    // End of input
    if (c == '\n' || c == '\r' || c == EOF) {
        return false;
    }

    // --- Castling notation ---
    if (c == 'O') {
        char dash = getc(stdin);
        char o2   = getc(stdin);
        if (dash == '-' && o2 == 'O') {
            move->piece_type = PIECE_KING;
            move->castling   = CASTLE_KINGSIDE;

            char maybe_dash = getc(stdin);
            if (maybe_dash == '-') {
                char o3 = getc(stdin);
                if (o3 == 'O') {
                    move->castling = CASTLE_QUEENSIDE;
                } else {
                    panicf("parse error: invalid castling notation\n");
                    return false;
                }
            } else if (maybe_dash != EOF) {
                ungetc(maybe_dash, stdin);
            }
            return true;
        }
        panicf("parse error: invalid castling notation\n");
        return false;
    }

    // --- Pawn moves ---
    if (c >= 'a' && c <= 'h') {
        move->piece_type = PIECE_PAWN;
        char next_c = getc(stdin);

        // Pawn capture like "exd5"
        if (next_c == 'x') {
            move->source_x = c - 'a';
            move->capture  = true;
            c      = getc(stdin); // target file
            next_c = getc(stdin); // target rank
        }

        move->target_square_x = c - 'a';
        if (next_c >= '1' && next_c <= '8') {
            move->target_square_y = next_c - '1';

            // --- Promotion check (inserted here) ---
            char promo = getc(stdin);
            if (promo == '=') {
                char piece = getc(stdin);
                switch (piece) {
                    case 'Q': move->promotion = PIECE_QUEEN;  break;
                    case 'R': move->promotion = PIECE_ROOK;   break;
                    case 'B': move->promotion = PIECE_BISHOP; break;
                    case 'N': move->promotion = PIECE_KNIGHT; break;
                    default:
                        panicf("parse error: invalid promotion piece '%c'\n", piece);
                        return false;
                }
            } else if (promo != EOF) {
                ungetc(promo, stdin);
            }

            return true;
        } else {
            panicf("parse error at character '%c'\n", next_c);
            return false;
        }
    }

    // --- Piece moves ---
    if (c >= 'A' && c <= 'Z') {
        switch (c) {
            case 'K': move->piece_type = PIECE_KING;   break;
            case 'Q': move->piece_type = PIECE_QUEEN;  break;
            case 'R': move->piece_type = PIECE_ROOK;   break;
            case 'B': move->piece_type = PIECE_BISHOP; break;
            case 'N': move->piece_type = PIECE_KNIGHT; break;
            default:
                panicf("parse error: unknown piece '%c'\n", c);
                return false;
        }

        c = getc(stdin);
        char next_c = getc(stdin);

        // --- Handle disambiguation like Qab7 (inserted here) ---
        if ((c >= 'a' && c <= 'h') && (next_c >= 'a' && next_c <= 'h')) {
            move->source_x = c - 'a';
            move->target_square_x = next_c - 'a';
            char rank = getc(stdin);
            if (rank >= '1' && rank <= '8') {
                move->target_square_y = rank - '1';
                return true;
            } else {
                panicf("parse error at character '%c'\n", rank);
                return false;
            }
        }

        // Origin + capture like "Nbd2" or "R1xd4"
        if (next_c == 'x' && ((c >= 'a' && c <= 'h') || (c >= '1' && c <= '8'))) {
            move->capture = true;
            if (c >= 'a' && c <= 'h') {
                move->source_x = c - 'a';
            } else {
                move->source_y = c - '1';
            }
            c      = getc(stdin);
            next_c = getc(stdin);
        }

        // Simple capture like "Qxd5"
        if (c == 'x') {
            move->capture = true;
            c      = next_c;
            next_c = getc(stdin);
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

    panicf("parse error: unexpected character '%c'\n", c);
    return false;
}