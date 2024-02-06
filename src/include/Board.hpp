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
         * @return The bitboard containing the abstract representation of all pieces of color.
         */
        U64 GetBoard(const Color color);
        /**
         * @brief Get occupation bitboard for all pieces with the specified colour and type found at the given position.
         * @param color The color of the pieces to search.
         * @param occupiedPosition The position of the piece on the board to check.
         * @return The bitboard containing the abstract representation of all pieces of the specified color. Empty board if no piece found at occupiedPosition.
         */
        U64 GetBoard(const Color color, const U64 occupiedPosition);
        /**
         * @brief Return all occupied bits on the chess board as a bitboard.
         * @return The single bitboard occupancy of the entire game.
        */
        U64 GetOccupancy() const { return std::accumulate(std::begin(fBoards), std::end(fBoards), U64(0), std::bit_or<U64>()); };
        /**
         * @brief Get the colour of the player whose turn it is to move.
         * @return The colour whose turn it is to make a move.
        */
        Color GetColorToMove() const { return fColorToMove; };
        /**
         * @brief Make the move on the board.
         * @param move The move to be made at a 32-bit word.
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
         * @param fen The FEN (Forthy Edwards Notation) string to load into the board.
        */
        void LoadFEN(const std::string &fen);
        /**
         * @brief Get whether a position on the board is occupied.
         * @param pos The position on the board to check.
         * @return The color and piece type occupying the square. If nothing will return a white null piece.
        */
        std::pair<Color, Piece> GetIsOccupied(const U64 pos);
        /**
        * @brief Get the Color and type of piece (if any) occupying the position
        * @param pos The position on the board to check
        * @param color The color whose pieces occupancy should be checked.
        * @return The color and piece of the occupying piece (if any) else Null piece is given.
        */
        std::pair<Color, Piece> GetIsOccupied(const U64 pos, const Color color);
        /**
        * @brief Get the number of completed moves full moves (e.g. both black and white have had a turn)
        * @return The number of completed turns (i.e. one move for white and one move for black)
        */
        int GetNMovesMade() { return (int)(fMadeMoves.size() / 2); };
        /**
         * @brief True if the white king has moved
         * @return True if the white king has moved during the game.
        */
        bool GetWhiteKingMoved() { return fWhiteKingMoved > 0; };
        /**
         * @brief True if the black king has moved
         * @return True if the black king has moved during the game.
        */
        bool GetBlackKingMoved() { return fBlackKingMoved > 0; };
        /**
         * @brief True if the white kingside rook has moved
         * @return True if white's kingside rook has moved or been captured.
        */
        bool GetWhiteKingsideRookMoved() { return fWhiteKingsideRookMoved > 0; };
        /**
         * @brief True if the white queenside rook has moved
         * @return True if the white queenside rook has moved or been captured.
        */
        bool GetWhiteQueensideRookMoved() { return fWhiteQueensideRookMoved > 0; };
        /**
         * @brief True if the black kingside rook has moved
         * @return True if the black kingside rook has moved or been captured.
        */
        bool GetBlackKingsideRookMoved() { return fBlackKingsideRookMoved > 0; };
        /**
         * @brief True if the black queenside rook has moved
         * @return True if the black queenside rook has moved or been captured.
        */
        bool GetBlackQueensideRookMoved() { return fBlackQueensideRookMoved > 0; };
        /**
         * @brief (Defunct) Get the integer incremented everytime the board changes (+1 even for undo)
         * @return Get a unique integer for each board position, not persistent between sessions.
        */
        int GetUnique() { return fUnique; };
        /**
         * @brief Get the last move made on the board.
         * @return The 32-bit move word. If no moves are made will return an empty 32-bit word.
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
         * @brief Clears all game state variables and put pieces in starting positions.
         */
        void Reset();
        /**
         * @brief Get whether the board has been loaded from a FEN to prevent castling and en-passant move skipping
         * @return True if board was initalised using a FEN string.
        */
        bool GetWasLoadedFromFEN() { return fWasLoadedFromFEN; };
        /**
        * @brief Get the tile (or empty bitboard) that is available for en-passant capture 
        * @return The square immedeately available for an en-passant capture when board has been loaded from FEN.
        */
        U64 GetEnPassantFEN() { return fEnPassantFENTarget; };
        /**
         * @brief Get the number of half-moves made since the last capture or pawn move.
         * @return The number of half moves made since the last pawn push or capture.
        */
        unsigned short GetHalfMoveClock() { return fHalfMoves; };
        /**
         * @brief Print to the console the supplied move in the standard notation, assumes move is not yet made.
         * @param move The move that will be printed.
        */
        void PrintDetailedMove(U32 move); 
    private:
        U64 fBoards[12]; ///< Array of 12 bitboards defining the postion. White pieces occupy boards 0-5 and black 6-12 in order (pawn, knight, bishop, queen, king)
        int fUnique; ///< Integer that is incremented everytime the board is changed, undone or modified in any way.
        // Move tracking
        std::vector<U32> fMadeMoves; ///< Vector of moves made with the back of the vector being the last made move.
        unsigned short fHalfMoves; ///< The half-move clock for enforcing the 50 move rule

        // Game state variables
        State fGameState; ///< Current state of play in the game e.g. stalemate
        unsigned short fWhiteKingMoved; ///< True if the white king has moved
        unsigned short fBlackKingMoved; ///< True if the black king has moved
        unsigned short fWhiteKingsideRookMoved; ///< >0 if the white kingside rook has moved or been captured.
        unsigned short fWhiteQueensideRookMoved; ///< >0 if the white queenside rook has moved or been captured.
        unsigned short fBlackKingsideRookMoved; ///< >0 if the black kingside rook has moved or been captured.
        unsigned short fBlackQueensideRookMoved; ///< >0 if the black queenside rook has moved or been captured.
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