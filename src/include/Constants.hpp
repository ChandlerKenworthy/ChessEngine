#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#define set_bit(b, i) ((b) |= (1ULL << i))
#define get_bit(b, i) ((b) & (1ULL << i))
#define clear_bit(b, i) ((b) &= ~(1ULL << i))
#define get_LSB(b) (__builtin_ctzll(b))
#define get_MSB(b) (__builtin_clzll(b))

typedef unsigned long long U64;
const int SQUARES = 64;

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
    } else {
        return RANK_8;
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
    } else {
        return FILE_H;
    }
}

enum class Color {
    White,
    Black
};

enum class Piece {
    Pawn,   // 0
    Bishop, // 1
    Knight, // 2
    Rook,   // 3
    Queen,  // 4
    King,   // 5
    Null,   // 6
};

const std::vector<Piece> PIECES = {Piece::Pawn, Piece::Bishop, Piece::Knight, Piece::Rook, Piece::Queen, Piece::King};

const float VALUE_PAWN = 100; // centi-pawn value
const float VALUE_BISHOP = 300;
const float VALUE_KNIGHT = 300;
const float VALUE_ROOK = 500;
const float VALUE_QUEEN = 900;
const float VALUE_KING = 99999;

#endif