#pragma once

#include <cstdint>
#include <stdlib.h>
#include <iostream>
#include <string>
#include "masks.h"

typedef unsigned long long U64;
#define C64(constantU64) constantU64##ULL

const U64 TOP_ROW = C64(0xFF00000000000000);
const U64 BOTTOW_ROW = C64(0xFF);
const U64 RIGHT_COLUMN = C64(0x8080808080808080);
const U64 LEFT_COLUMN = C64(0x101010101010101);
const U64 TOP_TWO = C64(0xFFFF000000000000);
const U64 BOTTOW_TWO = C64(0xFFFF);
const U64 RIGHT_TWO = C64(0xC0C0C0C0C0C0C0C0);
const U64 LEFT_TWO= C64(0x303030303030303);

enum Color {
    WHITE = 0,
    BLACK = 1
};

Color operator~(Color color) {
	return Color(color ^ BLACK);
}

enum PieceType {
    NORMAL_PIECE = 1,
    KING_PIECE = 2
};

enum Piece {
    NO_PIECE = 0,    // Binary: 000
    WHITE_PIECE = 1, // Binary: 001
    WHITE_KING = 2,  // Binary: 010
    BLACK_PIECE = 5, // Binary: 101
    BLACK_KING = 6   // Binary: 110
};

inline Piece make_piece(Color c, PieceType p) {
    return Piece((c << 2) | p);
}

inline PieceType piece_type(Piece p) {
    return PieceType(p & 3);
}

enum Flags {
    QUIET = 0,
    CAPTURE = 0x1000,
    PROMOTION = 0x2000,
    CAPTURE_PROMOTION = 0x3000
};

struct Move {
  private:
    // Internal representation of the move
    // Bits: EMPTY EMPTY PROMOTION CAPTURE FROM FROM FROM FROM FROM FROM TO TO TO TO TO TO
    uint16_t move;
  public:
    Move(int from, int to) {
        move = (from << 6) | to;
    }

    Move(int from, int to, Flags flags) {
        move = flags | (from << 6) | to;
    }

    Move() { //Defaults to 0
        move = 0;
    }

    int from() {
        return (move >> 6) & 0x3F;
    }

    int to() {
        return move & 0x3F;
    }

    int flags() {
        return move & 0x3000;
    }
};

/**
 * Returns false if bit at bit_num is 0, othewise returns true
*/
inline bool check_bit(U64 bitboard, int bit_num) {
    return (bitboard & single_bitboard[bit_num]) != 0;
}

const int index64[64] = {
    0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
};

const U64 debruijn64 = C64(0x03f79d71b4cb0a89);

/**
 * bitScanForward
 * @author Kim Walisch (2012)
 * @param bb bitboard to scan
 * @precondition bb != 0
 * @return index (0..63) of least significant one bit
 */
inline int bitScanForward(U64 bb) {
   return index64[((bb ^ (bb-1)) * debruijn64) >> 58];
}

//Convert notation through indexing array
const std::string index_to_string[64] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};

const std::string row_str = "  + - + - + - + - + - + - + - + - +";
void print(U64 bitboard) {
    std::cout << std::endl;
    for (int row = 7; row >= 0; row--) {
        std::cout << row_str << std::endl;
        std::cout << (row + 1);
        std::cout << " ";
    
        for (int column = 0; column < 8; column++) {
            std::cout << "| ";

            if (check_bit(bitboard, ((row*8) + column))) {
                std::cout << "1";
            } else {
                std::cout << ".";
            }

            std::cout << " ";
        }
        std::cout << "|" << std::endl;
    }
    std::cout << row_str << std::endl;
    std::cout << "    a   b   c   d   e   f   g   h" << std::endl << std::endl;
}