/**
 * @file Board.hpp
 * @brief Definition of the Board class.
 */

#ifndef BOARD_HPP
#define BOARD_HPP

#include <vector>
#include <string>
#include <cctype>
#include <utility>

#include "Constants.hpp"

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
        /**
         * @brief Get the current state of the game can either be in play, stalemate or checkmate.
         * @return State, the enumeration for the current game state.
         */
        State GetState() { return fGameState; };
        /**
         * @brief Get bitboard associated with a particular colour and type of piece.
         * @param color The color of the piece(s).
         * @param piece The type of the piece.
         * @return U64, the bitboard containing the abstract representation of the piece positions.
         */
        U64 GetBoard(Color color, Piece piece);
        /**
         * @brief Get occupation bitboard for all pieces of specified colour.
         * @param color The color whose logical or of bitboards is required.
         * @return U64, the bitboard containing the abstract representation of all pieces of color.
         */
        U64 GetBoard(Color color);
        /**
         * @brief Get occupation bitboard for all pieces with the specified colour and type found at the given position.
         * @param color The color of the pieces to search.
         * @param occupiedPosition The position of the piece on the board to check.
         * @return U64, the bitboard containing the abstract representation of all pieces of color. Empty board if no piece found at occupiedPosition.
         */
        U64 GetBoard(Color color, U64 occupiedPosition);
        /**
         * @brief Get the colour of the player whose turn it is to move.
         */
        Color GetColorToMove() { return fColorToMove; };
        /**
         * @brief Make the move on the board.
         * @param move The move to be made
         * 
         * Updates all the bit boards and required game state variables to make the requested move.
         */
        void MakeMove(Move *move);
        /**
         * @brief Undoes the actions of the last move.
         */
        void UndoMove();
        /**
         * @brief Loads a chess position from the FEN notation.
        */
        void LoadFEN(const std::string &fen);

        //////// >>>>> Move, document and refactor into Engine <<<<<<<
        std::pair<Color, Piece> GetIsOccupied(U64 pos);
        U64 GetJumpingPieceAttacks(Color attackingColor, Piece pieceType);
        U64 GetSlidingPieceAttacks(Color attackingColor, Piece pieceType);
        void GenerateLegalMoves();
        bool GetMoveIsLegal(Move* move);
        std::vector<Move> GetLegalMoves() { return fLegalMoves; };
        //////// >>>>> Move, document and refactor into Engine <<<<<<<
                
        /**
         * @brief Clears all game state variables and put pieces in starting position.
         * @param color The color of the piece.
         * @param piece The type of the piece.
         * @param board The bit-board to overwrite the existing bitboard
         */
        void SetBoard(Color color, Piece piece, U64 board);
        /**
         * @brief Clears all game state variables and put pieces in starting position.
         */
        void Reset();
    private:
        U64 fBoards[12]; ///< Array of 12 bitboards defining the postion. White pieces occupy boards 0-5 and black 6-12 in order (pawn, knight, bishop, queen, king)
        U64 fKnightAttacks[64]; ///< Bitboard giving all squares the knight attacks given LSB as input
        U64 fWhitePawnAttacks[64]; ///< Bitboard giving all squares the white pawn attacks given LSB as input
        U64 fBlackPawnAttacks[64]; ///< Bitboard giving all squares the black pawn attacks given LSB as input
        U64 fBishopAttacks[64]; ///< Bitboard giving (un-blocked) attack rays of bishop at LSB
        U64 fKingAttacks[64]; ///< Bitboard of king attacks at LSB position
        U64 fRookAttacks[64]; ///< Bitboard of rook attacks (un-blocked) at LSB
        U64 fQueenAttacks[64]; ///< Array of bitboards for queen attacks (un-blocked) at each LSB

        // Move tracking
        std::vector<Move> fLegalMoves;
        std::vector<Move> fMadeMoves; // Tracks each move made in the game

        // Game state variables
        State fGameState; ///< Current state of play in the game e.g. stalemate
        bool fWhiteHasCastled; ///< Whether white has castled
        bool fBlackHasCastled; ///< Whether black has castled
        bool fWhiteKingMoved; ///< True if the white king has moved
        bool fBlackKingMoved; ///< True if the black king has moved
        Color fColorToMove; ///< Current colour to make a move

        // Skipping functions
        bool fWasLoadedFromFEN;
        int fnMovesLastUpdate;

        // Internal methods

        /**
         * @brief Set all internal bitboards describing the chess board to zero (empty boards)
        */
        void EmptyBoards();

        // Undocumented/to be moved
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
        
};

#endif