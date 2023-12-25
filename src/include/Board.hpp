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
         * @brief Get a pointer to the bitboard associated with a particular colour and type of piece.
         * @param color The color of the piece(s).
         * @param piece The type of the piece.
         * @return U64*, pointer to the bitboard containing the abstract representation of the piece positions.
        */
        U64* GetBoardPointer(Color color, Piece piece);
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
        /**
         * @brief Get whether a position on the board is occupied.
         * @param pos The position on the board to check.
         * @return The color and piece of the occupying piece (if any) else Null piece is given.
        */
        std::pair<Color, Piece> GetIsOccupied(U64 pos);
        /**
            * @brief Get the number of completed moves (i.e. the ply)
        */
        int GetNMovesMade() { return fMadeMoves.size(); };
        /**
         * @brief True if the white king has moved
        */
        bool GetWhiteKingMoved() { return fWhiteKingMoved; };
        /**
         * @brief True if the black king has moved
        */
        bool GetBlackKingMoved() { return fBlackKingMoved; };
        /**
         * @brief True if the white kingside rook has moved
        */
        bool GetWhiteKingsideRookMoved() { return fWhiteKingsideRookMoved; };
        /**
         * @brief True if the white queenside rook has moved
        */
        bool GetWhiteQueensideRookMoved() { return fWhiteQueensideRookMoved; };
        /**
         * @brief True if the black kingside rook has moved
        */
        bool GetBlackKingsideRookMoved() { return fBlackKingsideRookMoved; };
        /**
         * @brief True if the black queenside rook has moved
        */
        bool GetBlackQueensideRookMoved() { return fBlackQueensideRookMoved; };
        /**
         * @brief Get the integer incremented everytime the board changes (+1 even for undo)
        */
        int GetUnique() { return fUnique; };
        /**
         * @brief Get a pointer to the last move made on the board.
        */
        Move* GetLastMove() { return fMadeMoves.size() > 0 ? &fMadeMoves.back() : nullptr; }
        /**
         * @brief Overwrite the bitboard for the specified piece and colour with a new bitboard.
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

        int fUnique; ///< Integer that is incremented everytime the board is changed, undone or modified in any way.

        // Move tracking
        std::vector<Move> fLegalMoves;
        std::vector<Move> fMadeMoves; // Tracks each move made in the game

        // Game state variables
        State fGameState; ///< Current state of play in the game e.g. stalemate
        bool fWhiteKingMoved; ///< True if the white king has moved
        bool fBlackKingMoved; ///< True if the black king has moved
        bool fWhiteKingsideRookMoved; ///< True if the white kingside rook has moved
        bool fWhiteQueensideRookMoved; ///< True if the white queenside rook has moved
        bool fBlackKingsideRookMoved; ///< True if the black kingside rook has moved
        bool fBlackQueensideRookMoved; ///< True if the black queenside rook has moved
        Color fColorToMove; ///< Current colour to make a move

        // Skipping functions
        bool fWasLoadedFromFEN;

        /**
         * @brief Set all internal bitboards describing the chess board to zero (empty boards)
        */
        void EmptyBoards();

        // Undocumented/to be moved
        Piece GetPiece(Color color, U64 position);
        void RemoveIllegalMoves();
        bool GetBoardIsLegal();
        
};

#endif