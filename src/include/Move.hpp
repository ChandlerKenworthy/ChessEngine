#ifndef MOVE_HPP
#define MOVE_HPP

#include "Constants.hpp"

// Define all the functions to quickly extract data for the U32 move
inline U64 GetMoveOrigin(U32 move) {
    U64 origin = 0;
    int lsb = move & 0b111111; // Get the first 6-bits only
    set_bit(origin, lsb);
    return origin;
}

inline void SetMoveOrigin(U32 &move, U64 origin) {
    int lsb = get_LSB(origin);
    // Clear the first 6 bits
    move &= ~0b111111;

    // Set the first 6 bits based on the LSB
    move |= (lsb & 0b111111);
}

inline U64 GetMoveTarget(U32 move) {
    U64 target = 0;
    int lsb = (move >> 6) & 0b111111; // Get the first 6-bits only
    set_bit(target, lsb);
    return target;
}

inline void SetMoveTarget(U32 &move, U64 origin) {
    int lsb = get_LSB(origin);
    // Clear the first 7-12 bits
    move &= ~(0b111111 << 6);

    // Set bits 7-12 based on the new value
    move |= ((lsb & 0b111111) << 6);
}

inline bool GetMovePieceWasTaken(U32 move) {
    return move & 0b1000000000000;
}

inline void SetMovePieceWasTaken(U32 &move, bool pieceIsTaken) {
    // Set the 13th bit
    if(pieceIsTaken) {
        move |= (1 << 12);
    } else {
        move &= ~(1 << 12);
    }
}

inline Color GetMoveColor(U32 move) {
    return (move & 0b10000000000000) ? Color::White : Color::Black;
}

inline void SetMoveColor(U32 &move, Color color) {
    // Set the 14th bit
    if(color == Color::White) {
        move |= (1 << 13);
    } else {
        move &= ~(1 << 13);
    }
}

// TODO: Up to here
inline Piece GetMoveTakenPiece(U32 move) {
    int numPiece = (move >> 14) & 0b111;
    Piece piece = Piece::Null;
    if(numPiece < 7)
        piece = static_cast<Piece>(numPiece);
    return piece;
} 

inline void SetMoveTakenPiece(U32 &move, Piece piece) {
    int numPiece = static_cast<int>(piece);
    // Clear the first 16-18 bits
    move &= ~(0b111 << 14);

    // Set bits based on the new value
    move |= ((numPiece & 0b111) << 14);
}

inline bool GetMoveIsEnPassant(U32 move) {
    return move & 0b100000000000000000;
}

inline void SetMoveIsEnPassant(U32 &move, bool isEnPassant) {
    // Set the 18th bit
    if(isEnPassant) {
        move |= (1 << 17);
    } else {
        move &= ~(1 << 17);
    }
}

inline bool GetMoveIsCastling(U32 move) {
    return move & 0b1000000000000000000;
}

inline void SetMoveIsCastling(U32 &move, bool isCastling) {
    // Set the 18th bit
    if(isCastling) {
        move |= (1 << 18);
    } else {
        move &= ~(1 << 18);
    }
}

inline bool GetMoveIsPromotion(U32 move) {
    return move & 0b10000000000000000000;
}

inline void SetMoveIsPromotion(U32 &move, bool isPromotion) {
    // Set the 18th bit
    if(isPromotion) {
        move |= (1 << 19);
    } else {
        move &= ~(1 << 19);
    }
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
    return move & 0b100000000000000000000000000;
}

inline void SetMoveIsCheck(U32 &move, bool isCheck) {
    if(isCheck) {
        move |= (1 << 26);
    } else {
        move &= ~(1 << 26);
    }
}

inline void PrintMove(U32 m) {
    std::cout << GetPieceString(GetMovePiece(m));
    std::cout << " from " << GetStringPosition(GetMoveOrigin(m));
    std::cout << " to " << GetStringPosition(GetMoveTarget(m)) << std::endl;
    
}

#endif