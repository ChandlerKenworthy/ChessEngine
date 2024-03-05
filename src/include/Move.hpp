/**
 * @file Move.hpp
 * @brief Utiltiy functions for setting and getting data from 32-bit move words.
 */

#ifndef MOVE_HPP
#define MOVE_HPP

#include "Constants.hpp"

// A move is fully described by a 16-bit word. The first 6 bits specify the start tile, the next 6 the end tile then the file 4 bits
// act as flags for special things such as pawn promotion (last bit set if is castling, other 3 specify promotion piece type)
// E.g. move = FFFFEEEEEESSSSSS
// The 6-bit numbers are the least significant set bit in the position.

// Define constants for bit positions
constexpr U16 ORIGIN_MASK = 0b111111; ///< Bits [0,5] used to provide the LSB of the move origin.
constexpr U16 TARGET_MASK = 0b111111 << 6; ///< Bits [6,11] used to provide the LSB of the move target.
constexpr U16 PROMOTION_MASK = 0b111 << 12; ///< Bits [12,14] represent the type of piece the pawn promotes to, if no promotion null piece is used
constexpr U16 ISCASTLING_MASK = 0b1 << 15;

/**
 * @brief Get the bitboard with a single set bit at the origin of the move.
 * @param move The 16-bit move word.
 * @return The bitboard with the bit set at the move origin.
*/
inline U64 GetMoveOrigin(U16 move) {
    return 1ULL << (move & ORIGIN_MASK);
}

/**
 * @brief Set the bitboard with a single set bit at the origin provided.
 * @param move The 16-bit move word to modify in-place.
 * @param origin The origin position of the move. Must be a single set bit.
*/
inline void SetMoveOrigin(U16 &move, U64 origin) {
    U8 lsb = __builtin_ctzll(origin);
    move &= ~ORIGIN_MASK;        // Clear the first 6 bits
    move |= (lsb & ORIGIN_MASK); // Set the first 6 bits based on the LSB
}

/**
 * @brief Get the bitboard with a single set bit at the target of the move.
 * @param move The 16-bit move word.
 * @return The bitboard with the bit set at the move target.
*/
inline U64 GetMoveTarget(U16 move) {
    return 1ULL << ((move >> 6) & ORIGIN_MASK);
    //return 1ULL << ((move & TARGET_MASK));
}

/**
 * @brief Set the bitboard with a single set bit at the target provided.
 * @param move The 16-bit move word to modify in-place.
 * @param origin The target position of the move. Must be a single set bit.
*/
inline void SetMoveTarget(U16 &move, U64 origin) {
    int lsb = __builtin_ctzll(origin);
    move &= ~TARGET_MASK; // Clear the bits [6,11]
    move |= ((lsb & 0b111111) << 6); // Set bits [6,11] based on the new value
}

inline bool GetMoveIsCastling(U16 move) {
    return move & ISCASTLING_MASK;
}

inline void SetMoveIsCastling(U16 &move, bool isCastling) {
    isCastling ? move |= ISCASTLING_MASK : move &= ~ISCASTLING_MASK;
}

inline bool GetMoveIsPromotion(U16 move) {
    return move & PROMOTION_MASK;
}

inline Piece GetMovePromotionPiece(U16 move) {
    U8 numPiece = (move & PROMOTION_MASK) >> 12;
    Piece piece = Piece::Null;
    if(numPiece < 7)
        piece = static_cast<Piece>(numPiece);
    return piece;
}

inline void SetMovePromotionPiece(U16 &move, Piece piece) {
    U8 numPiece = static_cast<int>(piece);
    move &= ~PROMOTION_MASK; // Clear the bits
    move |= ((numPiece & 0b111) << 12); // Set the new bits
}


inline void SetMove(U16 &move, U64 origin, U64 target) {
    SetMoveOrigin(move, origin);
    SetMoveTarget(move, target);
}

inline void PrintMove(U16 m) {
    std::cout << GetStringPosition(GetMoveOrigin(m)) << GetStringPosition(GetMoveTarget(m));
    if(GetMoveIsPromotion(m)) {
        Piece p = GetMovePromotionPiece(m);
        if(p == Piece::Queen) {
            std::cout << "q";
        } else if(p == Piece::Bishop) {
            std::cout << "b";
        } else if(p == Piece::Rook) {
            std::cout << "r";
        } else if(p == Piece::Knight) {
            std::cout << "n";
        } else {
            std::cout << "Something went wrong when printing a promotional move with promo piece " << (int)p;
        }
    }
    
}

#endif