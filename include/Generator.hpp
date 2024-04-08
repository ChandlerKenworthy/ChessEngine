/**
 * @file Generator.hpp
 * @brief Definition of the Generator class.
 */

#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include <memory>
#include <vector>
#include <chrono>
#include <algorithm>

#include "Constants.hpp"
#include "Board.hpp"
#include "Move.hpp"

/**
 * @class Generator
 * @brief Handles generation of legal moves.
 * 
 * The Generator class is used to handle the computation necessary to play the game with the user and all legal computer moves. The generator can check for move legality. It uses pre-generated attack tables to speed up computation.
 */ 
class Generator {
    public:
        /**
         * @brief Instantiate a new instance of the Generator class.
        */
        explicit Generator();
        /**
         * @brief Generate all legal moves given a board configuration. Moves are stored in the fLegalMoves vector.
         * @param board The board configuration for which legal moves will be generated.
        */
        void GenerateLegalMoves(const std::shared_ptr<Board> &board);
        /**
         * @brief Generates all legal moves involving a capture in the current position.
         * @param board The board configuration to generate moves for.
        */
        void GenerateCaptureMoves(const std::shared_ptr<Board> &board);
        /**
         * @brief Get the legal moves from the last move generation.
         * @return Reference to the fLegalMoves vector.
        */
        std::vector<U16> GetLegalMoves() { return fLegalMoves; };
        /**
         * @brief Get a reference to the vector of legal moves stored [warning: dangerous do not modify in place]
         * @return The legal moves vector.
        */
        std::vector<U16>& GetLegalMoveRef() { return fLegalMoves; };
        /**
         * @brief Get the number of legal moves from the last move generation.
         * @return Number of legal moves. 
        */
        U8 GetNLegalMoves() { return fLegalMoves.size(); };
        /**
         * @brief Get the move at the specified position from the legal moves vector.
         * @param index The index to copy the move from.
         * @return The move at the specified index.
        */
        U16 GetMoveAt(const U8 index) { return index < fLegalMoves.size() ? fLegalMoves.at(index) : 0; };
        /**
         * @brief Get whether a provided move is legal. Will update the move word with extra information.
         * @param move The move to check.
         * @return True if the move is legal false otherwise.
        */
        bool GetMoveIsLegal(U16 &move);
        /**
         * @brief Get whether any squares in the specified mask are under attack from the enemy. Does not account for pinned enemy pieces.
         * @param mask The mask of squares to check.
         * @param attackingColor The colour to calculate the attack rays for.
         * @param board The board configuration to generate moves for.
         * @return True if any of the squares in mask are under attack from the specified colour.
        */
        bool IsUnderAttack(const U64 mask, const Color attackingColor, const std::shared_ptr<Board> &board);
        /**
         * @brief Find the attacks of the pawns for a particular colour. To be used for move ordering.
         * @param board The board configuration.
         * @param colorToMoveAttacks True if the attacking colour is the current colour to move on board.
         * @return Mask of all possible attacks of pawns from the specified colour.
        */
        U64 GetPawnAttacks(const std::shared_ptr<Board> &board, bool colorToMoveAttacks);
        /**
         * @brief Get all moves that are captures in the current set of legal moves.
         * @return Vector of legal capture moves.
        */
        std::vector<U16> GetCaptureMoves() { return fCaptureMoves; };
    private:
        std::vector<U16> fLegalMoves; ///< The set of legal moves available upon the last call to GenerateLegalMoves.
        std::vector<U16> fCaptureMoves; ///< The legal capturing moves available. Updated on call to GenerateLegalMoves.
        /**
         * @brief Generate attack tables for faster lookup during move generation.
        */
        void GenerateAttackTables();
        /**
         * @brief Fill the king attack table for the position specified.
         * @param pos The position of the king.
        */
        void FillKingAttackTable(const U64 pos);
        /**
         * @brief Fill the knight attack table for the position specified.
         * @param pos The position of the knight.
        */
        void FillKnightAttackTable(const U64 pos);
        /**
         * @brief Fill the attack tables for white and black pawns.
         * @param pos The position of the pawn.
        */
        void FillPawnAttackTable(const U64 pos);
        /**
         * @brief Fill attack table for straight sliding rays. Fills the primary (up-down) and secondary (side-to-side) straight attacks (e.g. those of a rook).
         * @param pos The position of the sliding piece (rook/queen).
        */
        void FillStraightAttackTables(const U64 pos);
        /**
         * @brief Fill attack table for diagonal sliding rays. Fills the primary and secondary diagonal attacks (e.g. those of a bishop).
         * @param pos The position of the sliding piece (bishop/queen).
        */
        void FillDiagonalAttackTables(const U64 pos);
        /**
         * @brief Check if the board has met the conditions for draw by the 50-move rule.
         * @param board The board to check.
         * @return True if the fifty move rule conditions have been met.
        */
        bool CheckFiftyMoveDraw(const std::shared_ptr<Board> &board);
        /**
         * @brief Check if the board has met the conditions for a draw by insufficient material.
         * @param board The board to check.
         * @return True if the conditions for insufficient material have been met.
        */
        bool CheckInsufficientMaterial(const std::shared_ptr<Board> &board);
        /**
         * @brief Check if the board has met the conditions for a draw by move repitition.
         * @param board The board to check.
         * @return True if the board has reached the identically same position 3 times already.
        */
        bool CheckMoveRepitition(const std::shared_ptr<Board> &board);
        /**
         * @brief Generate the pseudo-legal moves for a given position and append to the fLegalMoves vector.
         * @param board The board configuration to generate moves for.
        */
        void GeneratePseudoLegalMoves(const std::shared_ptr<Board> &board);
        /**
         * @brief Generate the pseudo-legal capturing moves for a given position and append to the fLegalMoves vector.
         * @param board The board configuration to generate moves for.
        */
        void GeneratePseudoLegalCaptureMoves(const std::shared_ptr<Board> &board);
        /**
         * @brief Generate the en-passant moves and append to fCaptureMoves
         * @param board The board configuration to generate moves for.
        */
        void GenerateEnPassantCaptureMoves(const std::shared_ptr<Board> &board);
        /**
         * @brief Remove illegal capturing moves (i.e. absolute pins, etc)
         * @param board The board configuration to generate moves for.
        */
        void RemoveIllegalCaptureMoves(const std::shared_ptr<Board> &board); // TODO: These appened to fLegalMoves
        /**
         * @brief Generate the pseudo-legal moves for the king.
         * @param board The board configuration to generate moves for.
        */
        void GenerateKingPseudoLegalMoves();
        /**
         * @brief Generate the pseudo-legal moves for all the knights.
         * @param board The board configuration to generate moves for.
        */
        void GenerateKnightPseudoLegalMoves(const std::shared_ptr<Board> &board);
        /**
         * @brief Generate the pseudo-legal moves for all the rooks.
         * @param board The board configuration to generate moves for.
        */
        void GenerateRookPseudoLegalMoves(const std::shared_ptr<Board> &board);
        /**
         * @brief Generate the pseudo-legal moves for all the bishops.
         * @param board The board configuration to generate moves for.
        */
        void GenerateBishopPseudoLegalMoves(const std::shared_ptr<Board> &board);
        /**
         * @brief Generate the pseudo-legal moves for all the queens.
         * @param board The board configuration to generate moves for.
        */
        void GenerateQueenPseudoLegalMoves(const std::shared_ptr<Board> &board);
        /**
         * @brief Generate the pseudo-legal moves for all the pawns.
         * @param board The board configuration to generate moves for.
        */
        void GeneratePawnPseudoLegalMoves(const std::shared_ptr<Board> &board);
        /**
         * @brief Generate en-passant moves.
         * @param board The board configuration to generate moves for.
        */
        void GenerateEnPassantMoves(const std::shared_ptr<Board> &board);
        /**
         * @brief Generate the set of possible castling moves.
         * @param board The board configuration to generate moves for.
        */
        void GenerateCastlingMoves(const std::shared_ptr<Board> &board);
        /**
         * @brief Get whether a specific type of castling is possible. Effectively checks occupancy and attack masks.
         * @param castlingMask All squares that must be free from attack in order to permit castling (includes the king and end point and all squares in between).
         * @param occupanyMask The tiles that must be free of any occupancy in order for castling to be permitted.
         * @param board The board configuration to generate moves for.
         * @return True if castling with these masks is possible.
        */
        bool IsCastlingPossible(U64 castlingMask, U64 occupancyMask, const std::shared_ptr<Board> &board);
        /**
         * @brief Get the bitboard of all possible attacks by the specified colour assuming they are the next colour to move. Does not take into account absolutely positioned pieces.
         * @param board The board configuration to generate moves for.
         * @param attackingColor The colour to calculate attacks for (assumes they are colour to move).
         * @return Mask of all attacks by the attacking colour excluding absolute pins.
        */
        U64 GetAttacks(const std::shared_ptr<Board> &board, const Color attackingColor);
        /**
         * @brief Removes illegal moves from the fLegalMoves vector. Does a complete check such as with absolute pins etc.
         * @param board The board configuration to generate moves for.
        */
        void RemoveIllegalMoves(const std::shared_ptr<Board> &board);
        /**
         * @brief Fill a vetor with the position of all absolutely pinned pieces and the pinning ray they occupy.
         * @param board The board configuration to generate moves for.
         * @param v Vector to fill.
         * @param d Direction to search for pinning rays.
        */
        void AddAbolsutePins(const std::shared_ptr<Board> &board, Direction d);
        /**
         * @brief Remove moves from the fLegalMoves vector that do not resolve the check when the king is in check.
         * @param board The board configuration to generate moves for.
        */
        void PruneCheckMoves(const std::shared_ptr<Board> &board, const bool copyToCapures);

        // Attack tables generated on instantiation
        U64 fKnightAttacks[64]; ///< All possible attacks of a knight at each position on the board.
        U64 fKingAttacks[64]; ///< All possible attacks of a king at each position on the board.
        U64 fPrimaryDiagonalAttacks[64]; ///< Primary diagonal (bottom left to top right) attacks for a sliding diagonal piece at LSB.
        U64 fSecondaryDiagonalAttacks[64]; ///< Secondary diagonal (top left to bottom right) attacks for a sliding diagonal piece at LSB.
        U64 fPrimaryStraightAttacks[64]; ///< Primary straight attacks (rank) for a sliding straight piece at LSB.
        U64 fSecondaryStraightAttacks[64]; ///< Primary straight attacks (file) for a sliding straight piece at LSB.
        U64 fWhitePawnDiagonalAttacks[64]; ///< All possible diagonal attacks of a white pawn given LSB as input.
        U64 fBlackPawnDiagonalAttacks[64]; ///< All possible diagonal attacks of a black pawn given LSB as input.
        U64 fWhitePawnForwardAttacks[64]; ///< Attacks (1 and 2 square forwards) of the white pawns
        U64 fBlackPawnForwardAttacks[64]; ///< Attacks (1 and 2 square forwards) of the black pawns

        // Variables to use when generating moves, helps reduce number of function calls
        Color fColor; ///< The colour of the piece to move for the provided board configuration when generating legal moves is called.
        U64 fEnemy; ///< Occupancy bit board of the color not to move.
        Color fOtherColor; ///< The colour who has just moved.
        U64 fOccupancy; ///< Total occupancy of the board represented as a single bitboard for ray occupancy calculations.
        U64 fKing; ///< Position of the king whose colour it is to move.

        std::vector<std::pair<U64, U64>> fPinnedPieces; ///< The position of the absolutely pinned piece and the ray pinning it including the position of the pinning piece.
        U64 fPinnedPositions; ///< Accumulation of first elements of fPinnedPieces.
        
};

#endif
