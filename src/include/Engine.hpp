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

#include "Constants.hpp"
#include "Board.hpp"
#include "Move.hpp"

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
        explicit Engine(const bool init);
        /**
         * @brief Generate the set of legal moves for the specified board.
         * @param board The board for which the legal moves should be calculated.
        */
        void GenerateLegalMoves(const std::unique_ptr<Board> &board);
        /**
         * @brief True if the provided move is a legal one otherwise false.
        */
        bool GetMoveIsLegal(U32 *move);
        /**
        * @brief Remove illegal moves from the fLegalMoves vector. This does a thorough check for pins, self-checks etc.
        * @param board The current board configuration.
        */
        void StripIllegalMoves(const std::unique_ptr<Board> &board);
        /**
         * @brief Get the number of legal moves for a board given the current position.
        */
       int GetNLegalMoves(const std::unique_ptr<Board> &board);
       /**
        * @brief Get all the legal moves.
       */
        std::vector<U32> GetLegalMoves() { return fLegalMoves; };
        /**
         * @brief Get a vector containing the position and type of pieces currently delivering a check to the player to move.
         * 
         * Modifies the passed vector v in place.
        */
        void PruneCheckMoves(const std::unique_ptr<Board> &board);
        /**
         * @brief Get all squares attacked by the specified color as a single bitboard.
        */
        U64 GetAttacks(const std::unique_ptr<Board> &board, const Color attackingColor);
        /**
         * @brief Check the board to see if the 50 move rule draw has been met. Update internal board state to match.
        */
        void CheckFiftyMoveDraw(const std::unique_ptr<Board> &board);
        /**
         * @brief Check the board for a draw by insufficient material. Updates internal board state to match.
        */
        void CheckInsufficientMaterial(const std::unique_ptr<Board> &board);

        float Evaluate(Board board); // Static evaluation of current game state with no look-ahead
        void SetMaxDepth(int depth) { fMaxDepth = depth; };
        int GetMaxDepth() { return fMaxDepth; };
        U32 GetBestMove(Board board);
    private:
        std::random_device fRandomDevice;
        int fMaxDepth;
        int fLastUnique;

        U64 fKnightAttacks[64]; ///< All possible attacks of a knight at each position on the board.
        U64 fKingAttacks[64]; ///< All possible attacks of a king at each position on the board.


        U64 fWhitePawnDiagonalAttacks[64]; ///< All possible diagonal attacks of a white pawn given LSB as input.
        U64 fBlackPawnDiagonalAttacks[64]; ///< All possible diagonal attacks of a black pawn given LSB as input.
        U64 fWhitePawnForwardAttacks[64]; ///< Attacks (1 and 2 square forwards) of the white pawns
        U64 fBlackPawnForwardAttacks[64]; ///< Attacks (1 and 2 square forwards) of the black pawns
        U64 fPrimaryDiagonalAttacks[64]; ///< Primary diagonal attacks for a sliding diagonal piece at LSB.
        U64 fSecondaryDiagonalAttacks[64]; ///< Secondary diagonal attacks for a sliding diagonal piece at LSB.
        U64 fPrimaryStraightAttacks[64]; ///< Primary straight attacks (rank) for a sliding straight piece at LSB.
        U64 fSecondaryStraightAttacks[64]; ///< Primary straight attacks (file) for a sliding straight piece at LSB.
        std::vector<U32> fLegalMoves; ///< All possible legal moves for a position for which this vector was filled.

        float Minimax(Board board, int depth, float alpha, float beta, Color maximisingPlayer);
        float GetMaterialEvaluation(Board board);


        /**
         * @brief Used on instantiation of the Engine class to generate attack tables ahead of time for faster lookup later.
        */
        void Prepare();
        /**
         * @brief Populate the knight attack table for the specified position.
         * @param pos Position to generate attack table for, must have only 1 set bit.
        */
        void BuildKnightAttackTable(const U64 pos);
        /**
            * @brief Populate the king attack table for the specified position.
            * @param pos Position to generate attack table for, must have only 1 set bit.
        */
        void BuildKingAttackTable(const U64 pos);
        /**
         * @brief Populate the white and black pawn attack table for the specified position.
         * @param pos Position to generate the attack table for, must have only 1 set bit.
         */
        void BuildPawnAttackTables(const U64 pos);
        /**
         * @brief Populate the straight ray attack tables for the specified position. Does not take into account blocking.
         * @param pos Position to generate attack table for, must have only 1 set bit.
         */
        void BuildStraightAttackTables(const U64 pos);
        /**
         * @brief Populate the diagonal ray attack tables for the specified position. Does not take into account blocking.
         * @param pos Position to generate attack table for, must have only 1 set bit.
         */
        void BuildDiagonalAttackTables(const U64 pos);
        /**
         * @brief Adds all the pseudo-legal moves to the fLegalMoves vector.
         * @param board The board for which pseudo-legal moves will be generated.
        */
        void GeneratePseudoLegalMoves(const std::unique_ptr<Board> &board);
        /**
         * @brief Generate the pseudo-legal pawn moves (doesn't check for possibles checks) for the color to move.
         * @param board The board for which pseudo-legal moves will be generated.
        */
        void GeneratePawnPseudoLegalMoves(const std::unique_ptr<Board> &board);
        /**
         * @brief Generate the pseudo-legal king moves (doesn't check for possibles checks) for the color to move.
         * @param board The board for which pseudo-legal moves will be generated.
        */
        void GenerateKingPseudoLegalMoves(const std::unique_ptr<Board> &board);
        /**
         * @brief Generate the pseudo-legal knight moves (doesn't check for possibles checks) for the color to move.
         * @param board The board for which pseudo-legal moves will be generated.
        */
        void GenerateKnightPseudoLegalMoves(const std::unique_ptr<Board> &board);
        /**
         * @brief Generate the pseudo-legal bishop moves (doesn't check for possibles checks) for the color to move.
         * @param board The board for which pseudo-legal moves will be generated.
        */
        void GenerateBishopPseudoLegalMoves(const std::unique_ptr<Board> &board);
        /**
         * @brief Generate the pseudo-legal rook moves (doesn't check for possibles checks) for the color to move.
         * @param board The board for which pseudo-legal moves will be generated.
        */
        void GenerateRookPseudoLegalMoves(const std::unique_ptr<Board> &board);
        /**
         * @brief Generate the pseudo-legal queen moves (doesn't check for possibles checks) for the color to move.
         * @param board The board for which pseudo-legal moves will be generated.
        */
        void GenerateQueenPseudoLegalMoves(const std::unique_ptr<Board> &board);
        /**
         * @brief Finds any pawn promotion moves and splits into separate moves for each type of piece it is possible to promote to.
        */
        void UpdatePromotionMoves();
        /**
         * @brief Add absolutely pinned pieces of the colour to move to a provided vector.
         * @param v Vector of pairs of the pinned piece and piece pinning that piece in that order.
         * @param d Direction of the ray to check.
        */
        void AddAbolsutePins(const std::unique_ptr<Board> &board, std::vector<std::pair<U64, U64>> *v, Direction d);

        // TODO: Doc these functions!
        void GenerateEnPassantMoves(const std::unique_ptr<Board> &board);
        void GenerateCastlingMoves(const std::unique_ptr<Board> &board);
        bool IsCastlingPossible(U64 castlingMask, U64 occupancyMask, const std::unique_ptr<Board> &board);
        bool IsUnderAttack(U64 mask, Color attackingColor, const std::unique_ptr<Board> &board);
};

#endif