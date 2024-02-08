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
        void GenerateLegalMoves(const std::unique_ptr<Board> &board);
        /**
         * @brief Get the legal moves from the last move generation.
         * @return Reference to the fLegalMoves vector.
        */
        std::vector<U32>& GetLegalMoves() { return fLegalMoves; };
        /**
         * @brief Get a copy of all the legal moves from the last move generation.
         * @return Vector of all possible legal moves, as a copy of the fLegalMoves vector.
        */
        std::vector<U32> GetLegalMovesCopy() { return fLegalMoves; };
    private:
        std::vector<U32> fLegalMoves; ///< The set of legal moves available upon the last call to GenerateLegalMoves.

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
        bool CheckFiftyMoveDraw(const std::unique_ptr<Board> &board);
        /**
         * @brief Check if the board has met the conditions for a draw by insufficient material.
         * @param board The board to check.
         * @return True if the conditions for insufficient material have been met.
        */
        bool CheckInsufficientMaterial(const std::unique_ptr<Board> &board);
        /**
         * @brief Generate the pseudo-legal moves for a given position and append to the fLegalMoves vector.
         * @param board The board configuration to generate moves for.
        */
        void GeneratePseudoLegalMoves(const std::unique_ptr<Board> &board);
        /**
         * @brief Generate the pseudo-legal moves for the king.
         * @param board The board configuration to generate moves for.
        */
        void GenerateKingPseudoLegalMoves(const std::unique_ptr<Board> &board);
        /**
         * @brief Generate the pseudo-legal moves for all the knights.
         * @param board The board configuration to generate moves for.
        */
        void GenerateKnightPseudoLegalMoves(const std::unique_ptr<Board> &board);
        /**
         * @brief Generate the pseudo-legal moves for all the rooks.
         * @param board The board configuration to generate moves for.
        */
        void GenerateRookPseudoLegalMoves(const std::unique_ptr<Board> &board);
        /**
         * @brief Generate the pseudo-legal moves for all the bishops.
         * @param board The board configuration to generate moves for.
        */
        void GenerateBishopPseudoLegalMoves(const std::unique_ptr<Board> &board);


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
        Color fColor;
        Color fOtherColor;
        U64 fOccupancy;
        U64 fKing;
        
};

#endif