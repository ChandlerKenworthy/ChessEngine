/**
 * @file Board.hpp
 * @brief Definition of the Board class.
 */

#ifndef BOARD_HPP
#define BOARD_HPP

#include <vector>
#include <string>
#include <cctype>
#include <numeric>
#include <algorithm>
#include <utility>
#include <sstream>

#include "Constants.hpp"
#include "Move.hpp"

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
         * @brief Makes a copy of the board object retaining the current game state.
         * @param other Reference to the board to copy.
        */
        Board(const Board& other);
        /**
         * @brief Get the current state of the game can either be in play, stalemate or checkmate.
         * @return State, the enumeration for the current game state.
         */
        State GetState() const { return fGameState; };
        /**
         * @brief Set the play state of the game. 
        */
        void SetState(State state) { fGameState = state; };
        /**
         * @brief Get bitboard associated with a particular colour and type of piece.
         * @param color The color of the piece(s).
         * @param piece The type of the piece.
         * @return U64, the bitboard containing the abstract representation of the piece positions.
         */
        U64 GetBoard(const Color color, const Piece piece);
        /**
         * @brief Get a pointer to the bitboard associated with a particular colour and type of piece.
         * @param color The color of the piece(s).
         * @param piece The type of the piece.
         * @return U64*, pointer to the bitboard containing the abstract representation of the piece positions.
        */
        U64* GetBoardPointer(const Color color, const Piece piece);
        /**
         * @brief Get occupation bitboard for all pieces of specified colour.
         * @param color The color whose logical or of bitboards is required.
         * @return U64, the bitboard containing the abstract representation of all pieces of color.
         */
        U64 GetBoard(const Color color);
        /**
         * @brief Get occupation bitboard for all pieces with the specified colour and type found at the given position.
         * @param color The color of the pieces to search.
         * @param occupiedPosition The position of the piece on the board to check.
         * @return U64, the bitboard containing the abstract representation of all pieces of color. Empty board if no piece found at occupiedPosition.
         */
        U64 GetBoard(const Color color, const U64 occupiedPosition);
        /**
         * @brief Return all occupied bits on the chess board as a bitboard.
        */
        U64 GetOccupancy() const { return std::accumulate(std::begin(fBoards), std::end(fBoards), U64(0), std::bit_or<U64>()); };
        /**
         * @brief Get the colour of the player whose turn it is to move.
        */
        Color GetColorToMove() const { return fColorToMove; };
        /**
         * @brief Make the move on the board.
         * @param move The move to be made
         * 
         * Updates all the bit boards and required game state variables to make the requested move.
         */
        void MakeMove(const U32 move);
        /**
         * @brief Undoes the actions of the last move.
         */
        void UndoMove();
        /**
         * @brief Loads a chess position from the FEN notation.
        */
        void LoadFEN(const std::string &fen);
        /**
         * @brief Get whether a position on the board is occupied.
         * @param pos The position on the board to check.
         * @return The color and piece of the occupying piece (if any) else Null piece is given.
        */
        std::pair<Color, Piece> GetIsOccupied(const U64 pos);
        /**
        * @brief Get the Color and type of piece (if any) occupying the position
        * @param pos The position on the board to check
        * @return The color and piece of the occupying piece (if any) else Null piece is given.
        */
        std::pair<Color, Piece> GetIsOccupied(const U64 pos, const Color color);
        /**
        * @brief Get the number of completed moves full moves (e.g. both black and white have had a turn)
        */
        int GetNMovesMade() { return (int)(fMadeMoves.size() / 2); };
        /**
         * @brief Get the number of half-moves e.g. white making and move followed by black would be 2 half-moves.
        */
        int GetNHalfMoves() { return fMadeMoves.size(); };
        /**
         * @brief True if the white king has moved
        */
        bool GetWhiteKingMoved() { return fWhiteKingMoved > 0; };
        /**
         * @brief True if the black king has moved
        */
        bool GetBlackKingMoved() { return fBlackKingMoved > 0; };
        /**
         * @brief True if the white kingside rook has moved
        */
        bool GetWhiteKingsideRookMoved() { return fWhiteKingsideRookMoved > 0; };
        /**
         * @brief True if the white queenside rook has moved
        */
        bool GetWhiteQueensideRookMoved() { return fWhiteQueensideRookMoved > 0; };
        /**
         * @brief True if the black kingside rook has moved
        */
        bool GetBlackKingsideRookMoved() { return fBlackKingsideRookMoved > 0; };
        /**
         * @brief True if the black queenside rook has moved
        */
        bool GetBlackQueensideRookMoved() { return fBlackQueensideRookMoved > 0; };
        /**
         * @brief Get the integer incremented everytime the board changes (+1 even for undo)
        */
        int GetUnique() { return fUnique; };
        /**
         * @brief Get a pointer to the last move made on the board.
        */
        U32 GetLastMove() { return fMadeMoves.size() > 0 ? fMadeMoves.back() : U32(0); }
        /**
         * @brief Overwrite the bitboard for the specified piece and colour with a new bitboard.
         * @param color The color of the piece.
         * @param piece The type of the piece.
         * @param board The bit-board to overwrite the existing bitboard
         */
        void SetBoard(const Color color, const Piece piece, const U64 board);
        /**
         * @brief Clears all game state variables and put pieces in starting position.
         */
        void Reset();
        /**
         * @brief Get whether the board has been loaded from a FEN to prevent castling and en-passant move skipping
        */
        bool GetWasLoadedFromFEN() { return fWasLoadedFromFEN; };
        /**
        * @brief Get the tile (or empty bitboard) that is available for en-passant capture 
        */
        U64 GetEnPassantFEN() { return fEnPassantFENTarget; };
        /**
         * @brief Get the number of half-moves made since the last capture or pawn move.
        */
        unsigned short GetHalfMoveClock() { return fHalfMoves; };
        /**
         * @brief Print to the console the supplied move in the standard notation, assumes move is not yet made.
         * @param move The move that will be made.
        */
        void PrintDetailedMove(U32 move); 
    private:
        U64 fBoards[12]; ///< Array of 12 bitboards defining the postion. White pieces occupy boards 0-5 and black 6-12 in order (pawn, knight, bishop, queen, king)

        int fUnique; ///< Integer that is incremented everytime the board is changed, undone or modified in any way.

        // Move tracking
        std::vector<U32> fMadeMoves; // Tracks each move made in the game
        unsigned short fHalfMoves; ///< The half-move clock for enforcing the 50 move rule

        // Game state variables
        State fGameState; ///< Current state of play in the game e.g. stalemate
        unsigned short fWhiteKingMoved; ///< True if the white king has moved
        unsigned short fBlackKingMoved; ///< True if the black king has moved
        unsigned short fWhiteKingsideRookMoved; ///< >0 if the white kingside rook has moved
        unsigned short fWhiteQueensideRookMoved; ///< >0 if the white queenside rook has moved
        unsigned short fBlackKingsideRookMoved; ///< >0 if the black kingside rook has moved
        unsigned short fBlackQueensideRookMoved; ///< >0 if the black queenside rook has moved
        U64 fEnPassantFENTarget; ///< The tile in which an en-passant move is now available (as read from a FEN string)
        Color fColorToMove; ///< Current colour to make a move

        // Skipping functions
        bool fWasLoadedFromFEN; ///< Get if the board was loaded from FEN

        /**
         * @brief Set all internal bitboards describing the chess board to zero (empty boards)
        */
        void EmptyBoards();
};

#endif