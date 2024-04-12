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
#include <cmath>
#include <list>
#include <unordered_map>

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
        explicit Engine(const std::shared_ptr<Generator> &generator, const std::shared_ptr<Board> &board, const int maxDepth);
        /***
         * @brief Get a random legal move, testing purposes only. Must generate legal moves first!
        */
        U16 GetRandomMove();
        /**
         * @brief Add a bonus to the evaluation score for each passed pawn. This is especially true when the phase
         * of the game is endgame-like.
         * @return The bonus to add to the evaluation in centipawns.
        */
        float EvaluatePassedPawns();
        /**
         * @brief Add a penalty for isolated pawns. Especially those towards the centre as they are more vulnerable.
         * @return Penalty to apply (negative value) to the evaluation.
        */
        float EvaluateIsolatedPawns();
        /**
         * @brief Add a penalty for "bad" bishops i.e. those who are unable to progress forwards fully due to being blocked
         * by your own pawns.
         * @return Penalty to apply (negative value).
        */
        float EvaluateBadBishops();


        float Search(U8 depth, float alpha, float beta);

        float Evaluate(); // Static evaluation of a board
        float ForceKingToCornerEndgame(); // Favour positions where king is forced to edge of board for an easier mate in the endgame
        void SetMaxDepth(int depth) { fMaxDepth = depth; };
        int GetMaxDepth() { return fMaxDepth; };
        U16 GetBestMove(const bool verbose);
    private:
        int fNMovesSearched;
        int fNHashesFound;
        const std::shared_ptr<Generator> &fGenerator;
        const std::shared_ptr<Board> &fBoard;
        Color fOtherColor;

        // Transposition tables for storing evaluations, faster to lookup than recompute
        // <Hash, <Evaluation, Iterator to LRU list>>
        std::unordered_map<U64, std::pair<float, std::list<U64>::iterator>> fEvaluationCache; 
        std::list<U64> fLruList; // List to keep track of LRU (least recently used) order
        const std::size_t fMaxCacheSize; // Maximum size of the cache (N evaluations)

        int fMaxDepth;
        float fGamePhase;

        const int fPassPawnBonus[6] = {50, 40, 30, 20, 10, 5}; ///< Distance from left to right so 0th = 1 square from promo values are in centipawns
        const int fIsolatedPawnPenaltyByFile[8] = {-10, -15, -25, -30, -30, -25, -15, -10};
        const int fBadBishopPawnRankAwayPenalty[7] = {-200, -150, -100, -70, -50, -30, -20}; ///< Penalty to apply given number of ranks away pawn is so 0 (1 rank away is very bad)

        const float fPawnGuardKingEval[4] = {-200, 50, 100, 120};

        const float fKnightPosModifier[64] = { ///< Value modifier for the knight based on its position on the board
            -50,-40,-30,-30,-30,-30,-40,-50, // H1, G1, F1, E1, D1, C1, B1, A1 (7)
            -40,-20,  0,  5,  5,  0,-20,-40, // H2, ... A2
            -30,  5, 10, 15, 15, 10,  5,-30, // H3, ... A3
            -30,  0, 15, 20, 20, 15,  0,-30, // H4, ... A4
            -30,  5, 15, 20, 20, 15,  5,-30, // H5, ... A5
            -30,  0, 10, 15, 15, 10,  0,-30, // H6, ... A6
            -40,-20,  0,  0,  0,  0,-20,-40, // H7, ... A7
            -50,-40,-30,-30,-30,-30,-40,-50  // H8, ... A8
        };

        const float fQueenPosModifier[64] = { ///< Value modifier for the queen based on its position on the board
            -20,-10,-10, -5, -5,-10,-10,-20,
            -10,  0,  0,  0,  0,  0,  0,-10,
            -10,  0,  5,  5,  5,  5,  0,-10,
            -5,   0,  5,  5,  5,  5,  0, -5,
             0,   0,  5,  5,  5,  5,  0, -5,
            -10,  5,  5,  5,  5,  5,  0,-10,
            -10,  0,  5,  0,  0,  0,  0,-10,
            -20,-10,-10, -5, -5,-10,-10,-20
        };

        const float fRookPosModifier[2][64] = {{ ///< Value modifier for the rook based on its position on the board, 0th for white, 1st for black
            0,  0,  0,  5,  5,  0,  0,  0,
            -5,  0,  0,  0,  0,  0,  0, -5,
            -5,  0,  0,  0,  0,  0,  0, -5,
            -5,  0,  0,  0,  0,  0,  0, -5,
            -5,  0,  0,  0,  0,  0,  0, -5,
            -5,  0,  0,  0,  0,  0,  0, -5,
            5, 10, 10, 10, 10, 10, 10,  5,
            0,  0,  0,  0,  0,  0,  0,  0    
        },
        {
            0,  0,  0,  0,  0,  0,  0,  0,
            5, 10, 10, 10, 10, 10, 10,  5,
            -5,  0,  0,  0,  0,  0,  0, -5,
            -5,  0,  0,  0,  0,  0,  0, -5,
            -5,  0,  0,  0,  0,  0,  0, -5,
            -5,  0,  0,  0,  0,  0,  0, -5,
            -5,  0,  0,  0,  0,  0,  0, -5,
            0,  0,  0,  5,  5,  0,  0,  0
        }};

        const float fBishopPosModifier[2][64] = {{ ///< Value modifier for the bishop based on its position on the board
            -20,-10,-10,-10,-10,-10,-10,-20,
            -10,  5,  0,  0,  0,  0,  5,-10,
            -10, 10, 10, 10, 10, 10, 10,-10,
            -10,  0, 10, 10, 10, 10,  0,-10,
            -10,  5,  5, 10, 10,  5,  5,-10,
            -10,  0,  5, 10, 10,  5,  0,-10,
            -10,  0,  0,  0,  0,  0,  0,-10,
            -20,-10,-10,-10,-10,-10,-10,-20,
        }, {
            -20,-10,-10,-10,-10,-10,-10,-20,
            -10,  0,  0,  0,  0,  0,  0,-10,
            -10,  0,  5, 10, 10,  5,  0,-10,
            -10,  5,  5, 10, 10,  5,  5,-10,
            -10,  0, 10, 10, 10, 10,  0,-10,
            -10, 10, 10, 10, 10, 10, 10,-10,
            -10,  5,  0,  0,  0,  0,  5,-10,
            -20,-10,-10,-10,-10,-10,-10,-20,
        }};

        const float fKingPosModifier[4][64] = {{
            20, 30, 10,  0,  0, 10, 30, 20,
            20, 20,  0,  0,  0,  0, 20, 20,
            -10,-20,-20,-20,-20,-20,-20,-10,
            -20,-30,-30,-40,-40,-30,-30,-20,
            -30,-40,-40,-50,-50,-40,-40,-30,
            -30,-40,-40,-50,-50,-40,-40,-30,
            -30,-40,-40,-50,-50,-40,-40,-30,
            -30,-40,-40,-50,-50,-40,-40,-30,
        },{
            -30,-40,-40,-50,-50,-40,-40,-30,
            -30,-40,-40,-50,-50,-40,-40,-30,
            -30,-40,-40,-50,-50,-40,-40,-30,
            -30,-40,-40,-50,-50,-40,-40,-30,
            -20,-30,-30,-40,-40,-30,-30,-20,
            -10,-20,-20,-20,-20,-20,-20,-10,
            20, 20,  0,  0,  0,  0, 20, 20,
            20, 30, 10,  0,  0, 10, 30, 20
        }, { // White endgame king, want it to be more active in the ultimate endgame
            -10, -10, -10, -10, -10, -10, -10, -10,
            -10, -5 , -5,-5,-5,-5,-5, -10,
            -10, -5 , 5,5,5, 5, -5, -10,
            -10, -5 , 5,20,20, 5, -5, -10,
            -10, -5 , 5,20,20, 5, -5, -10,
            -10, -5 , 5,5,5, 5, -5, -10,
            -10, -5 ,-5,-5,-5,-5,-5, -10,
            -10,-10,-10,-10,-10,-10,-10,-10,
        }, {
            -10, -10, -10, -10, -10, -10, -10, -10,
            -10, -5 , -5,-5,-5,-5,-5, -10,
            -10, -5 , 5,5,5, 5, -5, -10,
            -10, -5 , 5,20,20, 5, -5, -10,
            -10, -5 , 5,20,20, 5, -5, -10,
            -10, -5 , 5,5,5, 5, -5, -10,
            -10, -5 ,-5,-5,-5,-5,-5, -10,
            -10,-10,-10,-10,-10,-10,-10,-10,
        }}; ///< Value modifier for the king based on its position on the board 0 = white early game, 1 = black early game, 2 = white endgame, 3 = black endgame

        //std::random_device fRandomDevice;

        /**
         * @brief Search until no more captures are available.
         * @param alpha Current value of alpha from minimax.
         * @param beta Current value of beta from minimax.
         * @return Evaluation of the position.
        */
        float SearchAllCaptures(float alpha, float beta);
        /**
         * @brief Counts up the knight material on both sides taking into account the positional value.
         * @return The value of the material with positive values favouring white.
        */
        float EvaluateKingPositions();
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
        /**
         * @brief Considers the position of the current king and how safe that is considering the current phase of the game. 
         * @return The value to enhance or reduce the evaluation of the position by based on the king safety (positive values are better).
        */
        float EvaluateKingSafety();

        // TODO: Reward rook pair, bishop pair over knight pair, rooks on open files.

        float GetMaterialEvaluation();
        void OrderMoves(std::vector<U16> &moves);

};

#endif