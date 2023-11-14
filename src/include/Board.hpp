#ifndef BOARD_HPP
#define BOARD_HPP

#include <vector>

#include "Constants.hpp"

struct Move {
    U64 origin;
    U64 target;
    Piece piece;
};

class Board {
    /*
    Class used as a wrapper to contain all 12 bitboards needed to define a chess position as well as 
    relevant game variables including whether castling and en-passant are available and whether the
    game is over (either through a draw or checkmate)
    */
    public:
        explicit Board();
        bool GetGameIsOver() { return fGameIsOver; };
        U64 GetBoard(Color color, Piece piece) const;
        U64* GetBoard(Color color, U64 occupiedPosition);
        U64 GetBoard(Color color) const; // Get occupation bitboard for all pieces of specified colour
        void GenerateAttackTables();
        U64 GetKnightAttacks(U64 position) { return fKnightAttacks[get_LSB(position)]; };
        U64 GetKingAttacks(U64 position) { return fKingAttacks[get_LSB(position)]; };
        U64 GetRookAttacks(U64 position) { return fRookAttacks[get_LSB(position)]; };
        U64 GetBishopAttacks(U64 position) { return fBishopAttacks[get_LSB(position)]; };
        U64 GetQueenAttacks(U64 position) { return fQueenAttacks[get_LSB(position)]; };
        void GeneratePseudoLegalMoves();
        std::vector<Move> GetPseudoLegalMoves() { return fPseudoLegalMoves; }; // TODO: pointer
        void MakeMove(Move move); // Maybe this and the two fucntions below should actually generate the pseudo-legal moves
        void SetBitBoard(Color color, Piece piece, U64 board); // TODO: Delete me after testing
    private:
        // Bitboards
        U64 fBoards[12]; // White pieces occupy boards 0-5 and black 6-12 in order (pawn, knight, bishop, queen, king)
        U64 fKnightAttacks[64]; // 1 bitboard showing all squares the knight attacks given bitboard as input
        U64 fBishopAttacks[64];
        U64 fKingAttacks[64];
        U64 fRookAttacks[64];
        U64 fQueenAttacks[64];

        // Move tracking
        std::vector<Move> fPseudoLegalMoves;

        // Game state variables
        bool fGameIsOver;
        bool fWhiteInCheckmate;
        bool fBlackInCheckmate;
        bool fWhiteCanCastle; // TODO: Different variables need for kingside/queenside
        bool fBlackCanCastle;
        Color fColorToMove;

        // Internal methods
        void GenerateKnightAttacks(int iPos, U64 position);
        void GenerateKingAttacks(int iPos, U64 position);
        void GenerateRookAttacks(int iPos, U64 position);
        void GenerateBishopAttacks(int iPos, U64 position);
        void GenerateQueenAttacks(int iPos, U64 position);
        void FillPseudoKnightMoves(U64 ownPieces);
        void FillPseudoKingMoves(U64 ownPieces);
        void FillPseudoRookMoves(U64 ownPieces, U64 otherPieces);
        
};

#endif