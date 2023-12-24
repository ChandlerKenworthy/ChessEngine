/**
 * @file Engine.hpp
 * @brief Definition of the Engine class.
 */

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <memory>
#include <random>

#include "Constants.hpp"
#include "Board.hpp"

/**
 * @class Engine
 * @brief Class to handle all computations related to the game of chess and bot the player plays against.
 * 
 * The Engine class is used to handle the computation necessary to play the game with the user and generate good quality computer moves. The engine can check for move legality, generate attacks tables and use static evaluation functions to a specified depth to gauge the best move in a given position.
 */ 
class Engine {
    public:
        /**
         * @brief Instantiate a new Engine class.
         * @param init True to generate attack tables (required for Engine to work correctly). Parameter only required for testing purposes. 
        */
        explicit Engine(bool init);



        float Evaluate(Board board); // Static evaluation of current game state with no look-ahead
        void SetMaxDepth(int depth) { fMaxDepth = depth; };
        int GetMaxDepth() { return fMaxDepth; };
        Move GetBestMove(Board board);
    private:
        std::random_device fRandomDevice;
        int fMaxDepth;

        float Minimax(Board board, int depth, float alpha, float beta, Color maximisingPlayer);
        float GetMaterialEvaluation(Board board);

        U64 fKnightAttacks[64]; ///< All possible attacks of a knight at each position on the board.
        U64 fKingAttacks[64]; ///< All possible attacks of a king at each position on the board.
        U64 fWhitePawnAttacks[64]; ///< All possible attacks of a white pawn given LSB as input.
        U64 fBlackPawnAttacks[64]; ///< All possible attacks of a black pawn given LSB as input.
        U64 fBishopAttacks[64]; ///< All possible attacks of an un-blocked bishop given LSB as input.
        U64 fRookAttacks[64]; ///< All possible attacks of an un-blocked rook given LSB as input.
        U64 fQueenAttacks[64]; ///< All possible attacks of an un-blocked queen given LSB as input.

        /**
         * @brief Used on instantiation of the Engine class to generate attack tables ahead of time for faster lookup later.
        */
        void Prepare();
        /**
         * @brief Populate the knight attack table for the specified position.
         * @param pos Position to generate attack table for, must have only 1 set bit.
        */
        void BuildKnightAttackTable(U64 pos);
        /**
            * @brief Populate the king attack table for the specified position.
            * @param pos Position to generate attack table for, must have only 1 set bit.
        */
        void BuildKingAttackTable(U64 pos);
        /**
         * @brief Populate the white and black pawn attack table for the specified position.
         * @param pos Position to generate the attack table for, must have only 1 set bit.
         */
        void BuildPawnAttackTable(U64 pos);
        /**
         * @brief Populate the rook attack table for the specified position. Does not take into account blocking.
         * @param pos Position to generate attack table for, must have only 1 set bit.
         */
        void BuildRookAttackTable(U64 pos);
        /**
         * @brief Populate the bishop attack table for the specified position. Does not take into account blocking.
         * @param pos Position to generate attack table for, must have only 1 set bit.
         */
        void BuildBishopAttackTable(U64 pos);
        /**
         * @brief Populate the queen attack table for the specified position. Does not take into account blocking.
         * @param pos Position to generate attack table for, must have only 1 set bit.
         */
        void BuildQueenAttackTable(U64 pos);
};

#endif