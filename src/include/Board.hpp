#ifndef BOARD_HPP
#define BOARD_HPP

#include <vector>
#include <string>
#include <cctype>

#include "Constants.hpp"

struct Move {
    U64 origin;
    U64 target;
    Piece piece;
    Piece takenPiece{ Piece::Null };
    bool WasEnPassant{ false };
    bool WasCastling{ false };
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
        U64 GetBoard(Color color, Piece piece);
        U64* GetBoard(Color color, U64 occupiedPosition);
        U64 GetBoard(Color color); // Get occupation bitboard for all pieces of specified colour
        // U64 GetKnightAttacks(U64 position) { return fKnightAttacks[get_LSB(position)]; };
        // U64 GetKingAttacks(U64 position) { return fKingAttacks[get_LSB(position)]; };
        // U64 GetRookAttacks(U64 position) { return fRookAttacks[get_LSB(position)]; };
        // U64 GetBishopAttacks(U64 position) { return fBishopAttacks[get_LSB(position)]; };
        // U64 GetQueenAttacks(U64 position) { return fQueenAttacks[get_LSB(position)]; };
        U64 GetJumpingPieceAttacks(Color attackingColor, Piece piece);
        void GenerateLegalMoves();
        std::vector<Move> GetLegalMoves() { return fLegalMoves; };
        void MakeMove(Move move);
        void UndoMove(int nMoves);
        void UndoMove() { UndoMove(1); };
        bool LoadFEN(const std::string &fen);
        Color GetColorToMove() { return fColorToMove; };
        void SetBitBoard(Color color, Piece piece, U64 board); // TODO: Delete me after testing
        void Reset(); // Clears all game state variables and puts pieces in starting position
    private:
        // Bitboards
        U64 fBoards[12]; // White pieces occupy boards 0-5 and black 6-12 in order (pawn, knight, bishop, queen, king)
        U64 fKnightAttacks[64]; // 1 bitboard showing all squares the knight attacks given bitboard as input
        U64 fBishopAttacks[64];
        U64 fKingAttacks[64];
        U64 fRookAttacks[64];
        U64 fQueenAttacks[64];

        // Move tracking
        std::vector<Move> fLegalMoves;
        std::vector<Move> fMadeMoves; // Tracks each move made in the game

        // Game state variables
        bool fGameIsOver;
        bool fWhiteInCheckmate;
        bool fBlackInCheckmate;
        bool fWhiteHasCastled;
        bool fBlackHasCastled;
        bool fWhiteKingMoved;
        bool fBlackKingMoved;
        Color fColorToMove;

        // Internal methods
        Piece GetPiece(Color color, U64 position);
        bool IsCastlingPossible(U64 occupancy, U64 castlingMask, Color attackingColor);
        bool IsUnderAttack(U64 squares, Color attackingColor);
        void GenerateAttackTables();
        void GenerateKnightAttacks(int iPos, U64 position);
        void GenerateKingAttacks(int iPos, U64 position);
        void GenerateRookAttacks(int iPos, U64 position);
        void GenerateBishopAttacks(int iPos, U64 position);
        void GenerateQueenAttacks(int iPos, U64 position);
        void FillPseudoKnightMoves(U64 ownPieces, U64 otherPieces);
        void FillPseudoKingMoves(U64 ownPieces, U64 otherPieces);
        void FillPseudoRookMoves(U64 ownPieces, U64 otherPieces);
        void FillPseudoBishopMoves(U64 ownPieces, U64 otherPieces);
        void FillPseudoQueenMoves(U64 ownPieces, U64 otherPieces);
        void FillPseudoPawnMoves(U64 ownPieces, U64 otherPieces);
        void GeneratePseudoLegalMoves();
        void AddEnPassant();
        void AddCastling();
        void RemoveIllegalMoves();
        bool GetBoardIsLegal();
        void EmptyBoards();
        Piece GetPieceFromChar(char c);
        
};

#endif