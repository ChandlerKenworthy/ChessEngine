/**
 * @file Engine.hpp
 * @brief Definition of the Engine class.
 */

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <memory>
#include <random>
#include <vector>
#include <cstdint>
#include <chrono>

#include "Constants.hpp"
#include "Board.hpp"
#include "Move.hpp"
#include "Generator.hpp"

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
         * @param depth The maximum search depth of the engine.
        */
        explicit Engine(const std::unique_ptr<Generator> &generator, const std::unique_ptr<Board> &board);


        /***
         * @brief Get a random legal move, testing purposes only. Must generate legal moves first!
        */
        U32 GetRandomMove();

        float Evaluate(); // Static evaluation of a board
        float ForceKingToCornerEndgame(); // Favour positions where king is forced to edge of board for an easier mate in the endgame
        void SetMaxDepth(int depth) { fMaxDepth = depth; };
        int GetMaxDepth() { return fMaxDepth; };
        U32 GetBestMove(bool verbose);
    private:
        const std::unique_ptr<Generator> &fGenerator;
        const std::unique_ptr<Board> &fBoard;
        Color fOtherColor;

        float fKnightPosModifier[64] = { ///< Value modifier for the knight based on its position on the board
            0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, // H1, G1, F1, E1, D1, C1, B1, A1
            0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.5, // H2, ... A2
            0.5, 1.0, 1.2, 1.2, 1.2, 1.2, 1.0, 0.5, // H3, ... A3
            0.5, 1.0, 1.2, 1.5, 1.5, 1.2, 1.0, 0.5, // H4, ... A4
            0.5, 1.0, 1.2, 1.5, 1.5, 1.2, 1.0, 0.5, // H5, ... A5
            0.5, 1.0, 1.2, 1.2, 1.2, 1.2, 1.0, 0.5, // H6, ... A6
            0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.5, // H7, ... A7
            0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5  // H8, ... A8
        };

        float fQueenPosModifier[64] = { ///< Value modifier for the queen based on its position on the board
            0.5, 1.0, 1.0, 1.5, 1.5, 1.0, 1.0, 0.5,  // H1, G1, F1, E1, D1, C1, B1, A1
            1.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 1.0,  // H2, ... A2
            1.0, 2.0, 2.2, 2.2, 2.2, 2.2, 2.0, 1.0,  // H3, ... A3
            1.5, 2.0, 2.2, 2.5, 2.5, 2.2, 2.0, 1.5,  // H4, ... A4
            1.5, 2.0, 2.2, 2.5, 2.5, 2.2, 2.0, 1.5,  // H5, ... A5
            1.0, 2.0, 2.2, 2.2, 2.2, 2.2, 2.0, 1.0,  // H6, ... A6
            1.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 1.0,  // H7, ... A7
            0.5, 1.0, 1.0, 1.5, 1.5, 1.0, 1.0, 0.5   // H8, ... A8
        };

        float fRookPosModifier[64] = { ///< Value modifier for the rook based on its position on the board
            0.5, 1.0, 1.0, 1.5, 1.5, 1.0, 1.0, 0.5,  // H1, G1, F1, E1, D1, C1, B1, A1
            1.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 1.0,  // H2, ... A2
            1.0, 2.0, 2.2, 2.2, 2.2, 2.2, 2.0, 1.0,  // H3, ... A3
            1.5, 2.0, 2.2, 2.5, 2.5, 2.2, 2.0, 1.5,  // H4, ... A4
            1.5, 2.0, 2.2, 2.5, 2.5, 2.2, 2.0, 1.5,  // H5, ... A5
            1.0, 2.0, 2.2, 2.2, 2.2, 2.2, 2.0, 1.0,  // H6, ... A6
            1.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 1.0,  // H7, ... A7
            0.5, 1.0, 1.0, 1.5, 1.5, 1.0, 1.0, 0.5   // H8, ... A8
        };

        float fBishopPosModifier[64] = { ///< Value modifier for the bishop based on its position on the board
            0.5, 1.0, 1.0, 1.5, 1.5, 1.0, 1.0, 0.5,  // H1, G1, F1, E1, D1, C1, B1, A1
            1.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 1.0,  // H2, ... A2
            1.0, 2.0, 2.2, 2.2, 2.2, 2.2, 2.0, 1.0,  // H3, ... A3
            1.5, 2.0, 2.2, 2.5, 2.5, 2.2, 2.0, 1.5,  // H4, ... A4
            1.5, 2.0, 2.2, 2.5, 2.5, 2.2, 2.0, 1.5,  // H5, ... A5
            1.0, 2.0, 2.2, 2.2, 2.2, 2.2, 2.0, 1.0,  // H6, ... A6
            1.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 1.0,  // H7, ... A7
            0.5, 1.0, 1.0, 1.5, 1.5, 1.0, 1.0, 0.5   // H8, ... A8
        };

        //float fKingPosModifier[64]; ///< Value modifier for the king based on its position on the board

        //std::random_device fRandomDevice;
        int fMaxDepth;

        /**
         * @brief Search until no more captures are available.
         * @param alpha Current value of alpha from minimax.
         * @param beta Current value of beta from minimax.
         * @return Evaluation of the position.
        */
        std::pair<float, int> SearchAllCaptures(float alpha, float beta);
        /**
         * @brief Main move search function including alpha-beta pruning. Returns evaluation of a position up-to a specified depth.
         * @param board The board to evaluate.
         * @param depth The depth the evaluation function should calculate up-to.
         * @param alpha The alpha value to prune at.
         * @param beta The beta value to prune at.
        */
        std::pair<float, int> Minimax(int depth, float alpha, float beta);
        /**
         * @brief Counts up the knight material on both sides taking into account the positional value.
         * @return The value of the material with positive values favouring white.
        */
        float EvaluateKnightPositions();
        /**
         * @brief Counts up the queen material on both sides taking into account the positional value.
         * @return The value of the material with positive values favouring white.
        */
        float EvaluateQueenPositions();
        /**
         * @brief Counts up the rook material on both sides taking into account the positional value.
         * @return The value of the material with positive values favouring white.
        */
        float EvaluateRookPositions();
        /**
         * @brief Counts up the bishop material on both sides taking into account the positional value.
         * @return The value of the material with positive values favouring white.
        */
        float EvaluateBishopPositions();

        float GetMaterialEvaluation();
        void OrderMoves(std::vector<U32> &moves);

};

#endif