#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <iostream>
#include <string>

/**
 * @file Constants.hpp
 * @brief Lots of useful constants and bit manipulation functions.
 */

#define set_bit(b, i) ((b) |= (1ULL << i))
#define get_bit(b, i) ((b) & (1ULL << i))
#define clear_bit(b, i) ((b) &= ~(1ULL << i))
#define get_LSB(b) (__builtin_ctzll(b))
#define get_MSB(b) (__builtin_clzll(b))

/**
 * @brief Typedef for U64 using unsigned long long.
 *
 * Using unsigned long long provides a 64-bit integer. Each bit can be considered a square on the chess board.
 */
typedef unsigned long long U64;

/**
 * @brief Use a 32-bit unsigned integer to represent a Move.
 *
 * 32-bit word representing a move. The first 6-bits (0-5) represent an integer [0,63] representing the least significant bit to set for the origin of the move. Bits (6-11) represent the LSB of the target. Bit 12 is whether the move took a piece (1 if it did). Bit 13 is the colour of the moving piece (1 for white). Bits 14-16 represent the type of the taken piece. Bit 17, 18 and 19 represent whether the move was en-passant, castling or promotion (1 for true, 0 for false). The last 3 bits (20-22) specify the promotional piece type. The last 9 bits are unused.
 */
typedef uint32_t U32;

/**
 * @brief Enumeration describing different piece colors.
 *
 * This enum class represents white and black colors.
 */
enum class Color {
    White, ///< White color.
    Black  ///< Black color.
};

/**
 * @brief Enumeration describing different states of the chess game.
 *
 * Represents all possible game states: in-play, stalemate, draw or checkmate.
 */
enum class State {
    Play,      ///< Game is in play.
    Stalemate, ///< Game is stalemate.
    Draw,      ///< Game is draw.
    Checkmate  ///< Game is checkmate.
};

/**
 * @brief Enumeration describing the compass directions of the board (North defined as increasing rank)
*/
enum class Direction {
    North,
    NorthEast,
    East,
    SouthEast,
    South,
    SouthWest,
    West,
    NorthWest
};

const std::vector<Direction> DIRECTIONS = {Direction::North, Direction::East, Direction::South, Direction::West, Direction::SouthEast, Direction::SouthWest, Direction::NorthEast, Direction::NorthWest};

/**
 * @brief Enumeration for all different types of chess piece.
 *
 * Represents all possible chess pieces: pawn, bishop, knight, rook, queen and king. Also includes a Null piece to represent emptiness.
 */
enum class Piece {
    Null,   ///< Null piece.
    Pawn,   ///< Pawn piece.
    Bishop, ///< Bishop piece.
    Knight, ///< Knight piece.
    Rook,   ///< Rook piece.
    Queen,  ///< Queen piece.
    King,   ///< King piece.
};

const int NSQUARES = 64;
const int BITS_PER_FILE = 8;
const int MIN_MOVES_FOR_CASTLING = 6;
const int MIN_MOVES_FOR_ENPASSANT = 3;

const float VALUE_PAWN = 100; // centi-pawn value
const float VALUE_BISHOP = 300;
const float VALUE_KNIGHT = 300;
const float VALUE_ROOK = 500;
const float VALUE_QUEEN = 900;
const float VALUE_KING = 99999;

inline int pop_LSB(U64 &b) {
    int i = get_LSB(b);
    b &= b - 1;
    return i;
}

inline U64 reverse(U64 b) {
    b = (b & 0x5555555555555555) << 1 | ((b >> 1) & 0x5555555555555555);
    b = (b & 0x3333333333333333) << 2 | ((b >> 2) & 0x3333333333333333);
    b = (b & 0x0f0f0f0f0f0f0f0f) << 4 | ((b >> 4) & 0x0f0f0f0f0f0f0f0f);
    b = (b & 0x00ff00ff00ff00ff) << 8 | ((b >> 8) & 0x00ff00ff00ff00ff);
    return (b << 48) | ((b & 0xffff0000) << 16) | ((b >> 16) & 0xffff0000) | (b >> 48);
}

inline U64 hypQuint(U64 piece, U64 occupancy, U64 mask) {
    return (((mask & occupancy) - piece * 2) ^ reverse(reverse(mask & occupancy) - reverse(piece) * 2)) & mask;
}

// TODO: Delete me this is for testing only
inline int PrintBitset(U64 b) {
    std::bitset<64> x = b;
    std::string s = x.to_string();
    for (std::size_t i=0 ; i<s.size() ; ++i)
    {
        if (i%8 == 0 && i != 0)
            std::cout << '\n';
        std::cout << s[i];
    }
    std::cout << "\n" << std::endl;
    return 0;
}

const U64 RANK_1 = 0x00000000000000FFULL;
const U64 RANK_2 = 0x000000000000FF00ULL;
const U64 RANK_3 = 0x0000000000FF0000ULL;
const U64 RANK_4 = 0x00000000FF000000ULL;
const U64 RANK_5 = 0x000000FF00000000ULL;
const U64 RANK_6 = 0x0000FF0000000000ULL;
const U64 RANK_7 = 0x00FF000000000000ULL;
const U64 RANK_8 = 0xFF00000000000000ULL;

const U64 FILE_A = 0x8080808080808080ULL;
const U64 FILE_B = 0x4040404040404040ULL;
const U64 FILE_C = 0x2020202020202020ULL;
const U64 FILE_D = 0x1010101010101010ULL;
const U64 FILE_E = 0x0808080808080808ULL;
const U64 FILE_F = 0x0404040404040404ULL;
const U64 FILE_G = 0x0202020202020202ULL;
const U64 FILE_H = 0x0101010101010101ULL;
const U64 FILE_GH = FILE_G | FILE_H;
const U64 FILE_AB = FILE_A | FILE_B;

const U64 WHITE_SQUARES = (FILE_A & (RANK_2 | RANK_4 | RANK_6 | RANK_8)) |
                            (FILE_B & (RANK_1 | RANK_3 | RANK_5 | RANK_7)) |
                            (FILE_C & (RANK_2 | RANK_4 | RANK_6 | RANK_8)) |
                            (FILE_D & (RANK_1 | RANK_3 | RANK_5 | RANK_7)) |
                            (FILE_E & (RANK_2 | RANK_4 | RANK_6 | RANK_8)) |
                            (FILE_F & (RANK_1 | RANK_3 | RANK_5 | RANK_7)) |
                            (FILE_G & (RANK_2 | RANK_4 | RANK_6 | RANK_8)) |
                            (FILE_H & (RANK_1 | RANK_3 | RANK_5 | RANK_7));
const U64 BLACK_SQUARES = ~WHITE_SQUARES;
const U64 PRIMARY_DIAGONAL = 0x8040201008040201; // top left to bottom right
const U64 SECONDARY_DIAGONAL = 0x0102040810204080; // top right to bottom left
const U64 EDGES = RANK_1 | RANK_8 | FILE_A | FILE_H;

constexpr U64 west(U64 b) { return (b & ~FILE_A) << 1; };
constexpr U64 east(U64 b) { return (b & ~FILE_H) >> 1; };
constexpr U64 north(U64 b) { return (b & ~RANK_8) << 8; };
constexpr U64 south(U64 b) { return (b & ~RANK_1) >> 8; };

constexpr U64 south_east(U64 b) { return (b & ~FILE_H) >> 9; };
constexpr U64 north_east(U64 b) { return (b & ~FILE_H) << 7; };
constexpr U64 south_west(U64 b) { return (b & ~FILE_A) >> 7; };
constexpr U64 north_west(U64 b) { return (b & ~FILE_A) << 9; };

const U64 KING_SIDE_CASTLING_MASK_WHITE = RANK_1 & (FILE_F | FILE_G);
const U64 QUEEN_SIDE_CASTLING_MASK_WHITE = RANK_1 & (FILE_C | FILE_D);
const U64 KING_SIDE_CASTLING_MASK_BLACK = RANK_8 & (FILE_F | FILE_G);
const U64 QUEEN_SIDE_CASTLING_MASK_BLACK = RANK_8 & (FILE_C | FILE_D);

inline int CountSetBits(U64 number) {
    int count = 0;
    while (number) {
        count += number & 1;
        number >>= 1;
    }
    return count;
}

inline U64 get_rank(U64 position) {
    if(position & RANK_1) {
        return RANK_1;
    } else if(position & RANK_2) {
        return RANK_2;
    } else if(position & RANK_3) {
        return RANK_3;
    } else if(position & RANK_4) {
        return RANK_4;
    } else if(position & RANK_5) {
        return RANK_5;
    } else if(position & RANK_6) {
        return RANK_6;
    } else if(position & RANK_7) {
        return RANK_7;
    } else if(position & RANK_8) {
        return RANK_8;
    } else {
        return 0;
    }
}

inline int get_rank_number(U64 position) {
    if(position & RANK_1) {
        return 1;
    } else if(position & RANK_2) {
        return 2;
    } else if(position & RANK_3) {
        return 3;
    } else if(position & RANK_4) {
        return 4;
    } else if(position & RANK_5) {
        return 5;
    } else if(position & RANK_6) {
        return 6;
    } else if(position & RANK_7) {
        return 7;
    } else if(position & RANK_8) {
        return 8;
    } else {
        return 0;
    }
}

inline U64 get_file(U64 position) {
    if(position & FILE_A) {
        return FILE_A;
    } else if(position & FILE_B) {
        return FILE_B;
    } else if(position & FILE_C) {
        return FILE_C;
    } else if(position & FILE_D) {
        return FILE_D;
    } else if(position & FILE_E) {
        return FILE_E;
    } else if(position & FILE_F) {
        return FILE_F;
    } else if(position & FILE_G) {
        return FILE_G;
    } else if(position & FILE_H) {
        return FILE_H;
    } else {
        return 0;
    }
}

inline int get_file_number(U64 position) {
    if(position & FILE_A) {
        return 1;
    } else if(position & FILE_B) {
        return 2;
    } else if(position & FILE_C) {
        return 3;
    } else if(position & FILE_D) {
        return 4;
    } else if(position & FILE_E) {
        return 5;
    } else if(position & FILE_F) {
        return 6;
    } else if(position & FILE_G) {
        return 7;
    } else if(position & FILE_H) {
        return 8;
    } else {
        return 0;
    }
}

inline U64 get_rank_from_number(int n) {
    if(n == 1) {
        return RANK_1;
    } else if(n == 2) {
        return RANK_2;
    } else if(n == 3) {
        return RANK_3;
    } else if(n == 4) {
        return RANK_4;
    } else if(n == 5) {
        return RANK_5;
    } else if(n == 6) {
        return RANK_6;
    } else if(n == 7) {
        return RANK_7;
    } else if(n == 8) {
        return RANK_8;
    } else {
        return U64{0};
    }
}

inline U64 get_file_from_number(int n) {
    if(n == 1) {
        return FILE_A;
    } else if(n == 2) {
        return FILE_B;
    } else if(n == 3) {
        return FILE_C;
    } else if(n == 4) {
        return FILE_D;
    } else if(n == 5) {
        return FILE_E;
    } else if(n == 6) {
        return FILE_F;
    } else if(n == 7) {
        return FILE_G;
    } else if(n ==8) {
        return FILE_H;
    } else {
        return U64{0};
    }
}

inline std::string GetStringPosition(U64 pos) {
    std::string sPos = "";
    if(pos & FILE_A) {
        sPos += "a";
    } else if(pos & FILE_B) {
        sPos += "b";
    } else if(pos & FILE_C) {
        sPos += "c";
    } else if(pos & FILE_D) {
        sPos += "d";
    } else if(pos & FILE_E) {
        sPos += "e";
    } else if(pos & FILE_F) {
        sPos += "f";
    } else if(pos & FILE_G) {
        sPos += "g";
    } else if(pos & FILE_H) {
        sPos += "h";
    }

    if(pos & RANK_1) {
        sPos += "1";
    } else if(pos & RANK_2) {
        sPos += "2";
    } else if(pos & RANK_3) {
        sPos += "3";
    } else if(pos & RANK_4) {
        sPos += "4";
    } else if(pos & RANK_5) {
        sPos += "5";
    } else if(pos & RANK_6) {
        sPos += "6";
    } else if(pos & RANK_7) {
        sPos += "7";
    } else if(pos & RANK_8) {
        sPos += "8";
    }

    return sPos;
}

inline std::string GetPieceString(Piece piece) {
    switch(piece) {
    case Piece::Pawn:
        return "Pawn";
        break;
    case Piece::Bishop:
        return "Bishop";
        break;
    case Piece::Knight:
        return "Knight";
        break;
    case Piece::Rook:
        return "Rook";
        break;
    case Piece::Queen:
        return "Queen";
        break;
    case Piece::King:
        return "King";
        break;
    default:
        return "Error piece does not exist";
        break;
    }
}

inline Piece GetPieceFromChar(char c) {
    c = toupper(c);
    if(c == 'N') {
        return Piece::Knight;
    } else if(c == 'K') {
        return Piece::King;
    } else if(c == 'P') { 
        return Piece::Pawn;
    } else if(c == 'Q') {
        return Piece::Queen;
    } else if(c == 'R') {
        return Piece::Rook;
    } else if(c == 'B') {
        return Piece::Bishop;
    } else {
        return Piece::Null;
    }
}

const std::vector<Piece> PIECES = {Piece::Pawn, Piece::Bishop, Piece::Knight, Piece::Rook, Piece::Queen, Piece::King}; ///< Vector of all the piece types for easy iterations

#endif