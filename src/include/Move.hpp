#ifndef MOVE_HPP
#define MOVE_HPP

#include "Constants.hpp"

// Define constants for bit positions
constexpr U32 ORIGIN_MASK = 0b111111;
constexpr U32 TARGET_MASK = 0b111111 << 6;
constexpr U32 PIECE_TAKEN_MASK = 1 << 12;
constexpr U32 COLOR_MASK = 1 << 13;
constexpr U32 PIECE_MASK = 0b111 << 14;
constexpr U32 EN_PASSANT_MASK = 1 << 17;
constexpr U32 CASTLING_MASK = 1 << 18;
constexpr U32 PROMOTION_MASK = 1 << 19;
constexpr U32 CHECK_MASK = 1 << 26;


// Define all the functions to quickly extract data for the U32 move
inline U64 GetMoveOrigin(U32 move) {
    U64 origin = 0;
    int lsb = move & ORIGIN_MASK; // Get the first 6-bits only
    set_bit(origin, lsb);
    return origin;
}

inline void SetMoveOrigin(U32 &move, U64 origin) {
    int lsb = get_LSB(origin);
    move &= ~ORIGIN_MASK;        // Clear the first 6 bits
    move |= (lsb & ORIGIN_MASK); // Set the first 6 bits based on the LSB
}

inline U64 GetMoveTarget(U32 move) {
    U64 target = 0;
    int lsb = (move >> 6) & ORIGIN_MASK; // Get bits 7-12
    set_bit(target, lsb);
    return target;
}

inline void SetMoveTarget(U32 &move, U64 origin) {
    int lsb = get_LSB(origin);
    move &= ~TARGET_MASK; // Clear the first 7-12 bits
    move |= ((lsb & ORIGIN_MASK) << 6); // Set bits 7-12 based on the new value
}

inline bool GetMovePieceWasTaken(U32 move) {
    return move & PIECE_TAKEN_MASK;
}

inline void SetMovePieceWasTaken(U32 &move, bool pieceIsTaken) {
    pieceIsTaken ? move |= PIECE_TAKEN_MASK : move &= ~PIECE_TAKEN_MASK;
}

inline Color GetMoveColor(U32 move) {
    return (move & COLOR_MASK) ? Color::White : Color::Black;
}

inline void SetMoveColor(U32 &move, Color color) {
    color == Color::White ? move |= COLOR_MASK : move &= ~COLOR_MASK;
}

inline Piece GetMoveTakenPiece(U32 move) {
    int numPiece = (move >> 14) & 0b111;
    Piece piece = Piece::Null;
    if(numPiece < 7)
        piece = static_cast<Piece>(numPiece);
    return piece;
} 

inline void SetMoveTakenPiece(U32 &move, Piece piece) {
    int numPiece = static_cast<int>(piece);
    move &= ~PIECE_MASK; // Clear the first 16-18 bits
    move |= ((numPiece & 0b111) << 14); // Set bits based on the new value
}

inline bool GetMoveIsEnPassant(U32 move) {
    return move & EN_PASSANT_MASK;
}

inline void SetMoveIsEnPassant(U32 &move, bool isEnPassant) {
    isEnPassant ? move |= EN_PASSANT_MASK : move &= ~EN_PASSANT_MASK;
}

inline bool GetMoveIsCastling(U32 move) {
    return move & CASTLING_MASK;
}

inline void SetMoveIsCastling(U32 &move, bool isCastling) {
    isCastling ? move |= CASTLING_MASK : move &= ~CASTLING_MASK;
}

inline bool GetMoveIsPromotion(U32 move) {
    return move & PROMOTION_MASK;
}

inline void SetMoveIsPromotion(U32 &move, bool isPromotion) {
    isPromotion ? move |= PROMOTION_MASK : move &= ~PROMOTION_MASK;
}

inline Piece GetMovePromotionPiece(U32 move) {
    int numPiece = (move >> 20) & 0b111;
    Piece piece = Piece::Null;
    if(numPiece < 7)
        piece = static_cast<Piece>(numPiece);
    return piece;
}

inline void SetMovePromotionPiece(U32 &move, Piece piece) {
    int numPiece = static_cast<int>(piece);
    move &= ~(0b111 << 20);
    move |= ((numPiece & 0b111) << 20);
}

inline Piece GetMovePiece(U32 move) {
    int numPiece = (move >> 23) & 0b111;
    Piece piece = Piece::Null;
    if(numPiece < 7)
        piece = static_cast<Piece>(numPiece);
    return piece;
}

inline void SetMovePiece(U32 &move, Piece piece) {
    int numPiece = static_cast<int>(piece);
    move &= ~(0b111 << 23);
    move |= ((numPiece & 0b111) << 23);
}

inline void SetMove(U32 &move, U64 origin, U64 target, Piece piece, Piece takenPiece) {
    SetMoveOrigin(move, origin);
    SetMoveTarget(move, target);
    SetMovePiece(move, piece);
    SetMoveTakenPiece(move, takenPiece);
    SetMovePieceWasTaken(move, takenPiece != Piece::Null);
}

inline bool GetMoveIsCheck(U32 &move) {
    return move & CHECK_MASK;
}

inline void SetMoveIsCheck(U32 &move, bool isCheck) {
    isCheck ? move |= CHECK_MASK : move &= ~CHECK_MASK;
}

inline void PrintMove(U32 m) {
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