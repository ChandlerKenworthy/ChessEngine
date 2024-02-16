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

        //float Evaluate(const std::unique_ptr<Board> &board); // Static evaluation of current game state with no look-ahead
        void SetMaxDepth(int depth) { fMaxDepth = depth; };
        int GetMaxDepth() { return fMaxDepth; };
        //U32 GetBestMove(const std::unique_ptr<Board> &board);
    private:
        const std::unique_ptr<Generator> &fGenerator;
        const std::unique_ptr<Board> &fBoard;

        //std::random_device fRandomDevice;
        int fMaxDepth;

        //float SearchAllCaptures(const std::unique_ptr<Board> &board, float alpha, float beta);

        /**
         * @brief Main move search function including alpha-beta pruning. Returns evaluation of a position up-to a specified depth.
         * @param board The board to evaluate.
         * @param depth The depth the evaluation function should calculate up-to.
         * @param alpha The alpha value to prune at.
         * @param beta The beta value to prune at.
        */
        //std::pair<float, int> Minimax(const std::unique_ptr<Board> &board, int depth, float alpha, float beta);

        //float GetMaterialEvaluation(const std::unique_ptr<Board> &board);
        //void OrderMoves(const std::unique_ptr<Board> &board, std::vector<U32> &moves);

};

#endif