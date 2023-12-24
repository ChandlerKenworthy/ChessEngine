#ifndef BOARD_HPP
#define BOARD_HPP

#include <vector>
#include <string>
#include <cctype>
#include <utility>

#include "Constants.hpp"

/**
 * @file Board.hpp
 * @brief Definition of the Board class and Move struct.
 */

/**
 * @class Board
 * @brief Abstract representation of the state of a chess board using a bitboard representation.
 * 
 * The Board class provides an abstract representation of the state of the chess game leaning on the bitboard representation. It includes properties of the game such as the set of moves that have been made, whether the king is in check, the global game state e.g. checkmate or stalemate and the ability to read in and load FEN strings.
 */ 
class Board {
    public:
        /**
         * @brief Constructs a board object.
         */
        explicit Board();
        bool GetGameIsOver() { return fGameIsOver; };
        U64 GetBoard(Color color, Piece piece);
        U64* GetBoard(Color color, U64 occupiedPosition);
        U64 GetBoard(Color color); // Get occupation bitboard for all pieces of specified colour
        std::pair<Color, Piece> GetIsOccupied(U64 pos);
        U64 GetJumpingPieceAttacks(Color attackingColor, Piece pieceType);
        U64 GetSlidingPieceAttacks(Color attackingColor, Piece pieceType);
        void GenerateLegalMoves();
        bool GetMoveIsLegal(Move* move);
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
        U64 fKnightAttacks[64]; // 1 bitboard showing all squares the knight attacks given LSB bit as input
        U64 fWhitePawnAttacks[64];
        U64 fBlackPawnAttacks[64];
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

        // Skipping functions
        bool fWasLoadedFromFEN;
        int fnMovesLastUpdate;

        // Internal methods
        Piece GetPiece(Color color, U64 position);
        bool IsCastlingPossible(U64 occupancy, U64 castlingMask, Color attackingColor);
        bool IsUnderAttack(U64 squares, Color attackingColor);
        void GenerateAttackTables();
        void PopulateKnightAttackTable(int iPos, U64 position);
        void PopulateKingAttackTable(int iPos, U64 position);
        void PopulatePawnAttackTable(int iPos, U64 position);
        void PopulateRookAttackTable(int iPos, U64 position);
        void PopulateBishopAttackTable(int iPos, U64 position);
        void PopulateQueenAttackTable(int iPos, U64 position);
        void FillPseudoKnightMoves(U64 ownPieces, U64 otherPieces);
        void FillPseudoKingMoves(U64 otherPieces);
        void FillPseudoRookMoves(U64 ownPieces, U64 otherPieces);
        void FillPseudoBishopMoves(U64 ownPieces, U64 otherPieces);
        void FillPseudoQueenMoves(U64 otherPieces);
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