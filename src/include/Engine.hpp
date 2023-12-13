#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <memory>

#include "Constants.hpp"
#include "Board.hpp"

class Engine {
    public:
        explicit Engine(bool init);
        float Evaluate(const std::unique_ptr<Board> &board); // Static evaluation of current game state with no look-ahead
        void SetMaxDepth(int depth) { fMaxDepth = depth; };
        int GetMaxDepth() { return fMaxDepth; };
        float GetWhitePawnPosValue(U64 position) { return fWhitePawnPos[get_LSB(position)]; };
        Move GetBestMove(Board board);
    private:
        int fMaxDepth;
        // Piece positional values
        float fWhitePawnPos[64];
        float fWhiteBishopPos[64];
        float fWhiteKnightPos[64];
        float fWhiteRookPos[64];
        float fWhiteQueenPos[64];
        float fWhiteKingPos[64];
        float fBlackPawnPos[64];
        float fBlackBishopPos[64];
        float fBlackKnightPos[64];
        float fBlackRookPos[64];
        float fBlackQueenPos[64];
        float fBlackKingPos[64];

        float Minimax(Board board, int depth, float alpha, float beta, Color maximisingPlayer);
        float GetMaterialEvaluation(const std::unique_ptr<Board> &board);
        float GetPositionalEvaluation(U64 position, Piece piece, Color pieceColor);
        float GetPositionValue(int index, Piece piece, Color color);
        void Initalize();
};

#endif