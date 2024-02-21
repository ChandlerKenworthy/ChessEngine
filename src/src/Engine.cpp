#include <iostream>
#include <fstream>
#include <string>

#include "Engine.hpp"

Engine::Engine(const std::unique_ptr<Generator> &generator, const std::unique_ptr<Board> &board) : fGenerator(generator), fBoard(board), fMaxDepth(4) {

}


float Engine::Evaluate() {
    float evaluation = GetMaterialEvaluation();
    int perspective = fBoard->GetColorToMove() == Color::White ? 1. : -1.;
    return evaluation * perspective; // Return in centipawns rather than pawns
}

float Engine::GetMaterialEvaluation() {
    // Counts material and respects the position of the material e.g. knights in the centre are stronger
    float material = 0.;
    material += EvaluateKnightPositions();
    material += EvaluateQueenPositions();
    material += EvaluateRookPositions();
    material += EvaluateBishopPositions();

    material += (__builtin_popcountll(fBoard->GetBoard(Color::White, Piece::Pawn)) - __builtin_popcountll(fBoard->GetBoard(Color::Black, Piece::Pawn))) * VALUE_PAWN;
    return material;
}

float Engine::EvaluateKnightPositions() {
    float val = 0.;
    U64 white_knights = fBoard->GetBoard(Color::White, Piece::Knight);
    U64 black_knights = fBoard->GetBoard(Color::Black, Piece::Knight);
    while(white_knights) {
        val += (VALUE_KNIGHT * fKnightPosModifier[__builtin_ctzll(white_knights)]);
        white_knights &= white_knights - 1;
    }
    while(black_knights) {
        val -= (VALUE_KNIGHT * fKnightPosModifier[__builtin_ctzll(black_knights)]);
        black_knights &= black_knights - 1;
    }
    return val;
}

float Engine::EvaluateQueenPositions() {
    float val = 0.;
    U64 white_queens = fBoard->GetBoard(Color::White, Piece::Queen);
    U64 black_queens = fBoard->GetBoard(Color::Black, Piece::Queen);
    while(white_queens) {
        val += (VALUE_QUEEN * fQueenPosModifier[__builtin_ctzll(white_queens)]);
        white_queens &= white_queens - 1;
    }
    while(black_queens) {
        val -= (VALUE_QUEEN * fQueenPosModifier[__builtin_ctzll(black_queens)]);
        black_queens &= black_queens - 1;
    }
    return val;
}

float Engine::EvaluateRookPositions() {
    float val = 0.;
    U64 white_rooks = fBoard->GetBoard(Color::White, Piece::Rook);
    U64 black_rooks = fBoard->GetBoard(Color::Black, Piece::Rook);
    while(white_rooks) {
        val += (VALUE_ROOK * fRookPosModifier[__builtin_ctzll(white_rooks)]);
        white_rooks &= white_rooks - 1;
    }
    while(black_rooks) {
        val -= (VALUE_ROOK * fRookPosModifier[__builtin_ctzll(black_rooks)]);
        black_rooks &= black_rooks - 1;
    }
    return val;
}

float Engine::EvaluateBishopPositions() {
    float val = 0.;
    U64 white_bishops = fBoard->GetBoard(Color::White, Piece::Bishop);
    U64 black_bishops = fBoard->GetBoard(Color::Black, Piece::Bishop);
    while(white_bishops) {
        val += (VALUE_BISHOP * fBishopPosModifier[__builtin_ctzll(white_bishops)]);
        white_bishops &= white_bishops - 1;
    }
    while(black_bishops) {
        val -= (VALUE_BISHOP * fBishopPosModifier[__builtin_ctzll(black_bishops)]);
        black_bishops &= black_bishops - 1;
    }
    return val;
}

void Engine::OrderMoves(std::vector<U32> &moves) {
    const U64 pawnAttacks = fGenerator->GetPawnAttacks(fBoard, false);

    std::sort(moves.begin(), moves.end(), [&](U32 move1, U32 move2) {
        float move1ScoreEstimate = 0.;
        const int pieceType1 = (int)GetMovePiece(move1);
        const int takenPieceType1 = (int)GetMoveTakenPiece(move1);
        float move2ScoreEstimate = 0.;
        const int pieceType2 = (int)GetMovePiece(move2);
        const int takenPieceType2 = (int)GetMoveTakenPiece(move2);

        // Prioritise capturing opponent's most valudable pieces with our least valuable piece
        if(takenPieceType1 != (int)Piece::Null)
            move1ScoreEstimate += 10. * PIECE_VALUES[takenPieceType1] - PIECE_VALUES[pieceType1];
        if(takenPieceType2 != (int)Piece::Null)
            move2ScoreEstimate += 10. * PIECE_VALUES[takenPieceType2] - PIECE_VALUES[pieceType2];

        // Promoting a pawn is probably a good plan
        if(GetMoveIsPromotion(move1))
            move1ScoreEstimate += PIECE_VALUES[(int)GetMovePromotionPiece(move1)];
        if(GetMoveIsPromotion(move2))
            move2ScoreEstimate += PIECE_VALUES[(int)GetMovePromotionPiece(move2)];
        
        // Penalize moving our pieces to a square attacked by an opponent pawn
        if(pawnAttacks & GetMoveTarget(move1))
            move1ScoreEstimate -= PIECE_VALUES[pieceType1];
        if(pawnAttacks & GetMoveTarget(move2))
            move2ScoreEstimate -= PIECE_VALUES[pieceType2];

        return move1ScoreEstimate > move2ScoreEstimate; // Higher scores come first
    });
}

float Engine::SearchAllCaptures(float alpha, float beta) {
    float eval = Evaluate();
    if(eval >= beta)
        return beta;
    alpha = std::max(alpha, eval);

    fGenerator->GenerateLegalMoves(fBoard); // TODO: Generate only capture moves rather than all then prune
    std::vector<U32> captureMoves = fGenerator->GetCaptureMoves(); // Get only capture moves
    OrderMoves(captureMoves);

    for(U32 move : captureMoves) {
        fBoard->MakeMove(move);
        eval = -SearchAllCaptures(-alpha, -beta);
        fBoard->UndoMove();
        if(eval >= beta)
            return beta;
        alpha = std::max(alpha, eval);
    }
    fBoard->UndoMove(); // Always need to undo one more time here
    return alpha;
}

std::pair<float, int> Engine::Minimax(int depth, float alpha, float beta) {
    // Return the evaluation up to depth [depth] and the number of moves explicitly searched
    if(depth == 0)
        return std::make_pair(Evaluate(), 1); // SearchAllCaptures(alpha, beta)

    int movesSearched = 0;
    fGenerator->GenerateLegalMoves(fBoard); // Move has been made in GetBestMove so need to find legal moves again
    std::vector<U32> moves = fGenerator->GetLegalMoves(); // Set of legal moves for this position

    Color movingColor = fBoard->GetColorToMove(); // These can change on recursive calls
    Color otherColor = movingColor == Color::White ? Color::Black : Color::White;
    if(moves.size() == 0) {
        if(fGenerator->IsUnderAttack(fBoard->GetBoard(movingColor, Piece::King), otherColor, fBoard))
            return std::make_pair(movingColor == Color::White ? -99999. : 99999., movesSearched); // Checkmate loss
        return std::make_pair(0., movesSearched); // Stalemate
    } 
    // For speed up, order the generated moves each iteration
    OrderMoves(moves);
    for(U32 move : moves) { // Search through all the possible moves
        fBoard->MakeMove(move);
        std::pair<float, int> result = Minimax(depth - 1, -beta, -alpha);
        float evaluation = -result.first;
        movesSearched += result.second;
        fBoard->UndoMove();
        if(evaluation >= beta)
            return std::make_pair(beta, movesSearched);
        alpha = std::max(alpha, evaluation);
    }
    return std::make_pair(alpha, movesSearched);
}

U32 Engine::GetBestMove(bool verbose) {
    auto start = std::chrono::high_resolution_clock::now();
    U32 bestMove{0};
    Color colorToMove = fBoard->GetColorToMove();
    // If white is playing the worst eval is -999 (i.e. black completely winning)
    float bestEval = colorToMove == Color::White ? -9999. : 9999.;
    int nMovesSearched = 0;

    // For each of the moves we want to find the "best" evaluation
    std::vector<U32> moves = fGenerator->GetLegalMoves();
    // Order the moves for faster searching
    OrderMoves(moves);
    for(U32 move : moves) {
        fBoard->MakeMove(move);
        std::pair<float, int> result = Minimax(fMaxDepth - 1, -99999., 99999.);
        fBoard->UndoMove();
        float eval = result.first;
        nMovesSearched += result.second;
        if(colorToMove == Color::White && eval >= bestEval) {
            bestEval = eval;
            bestMove = move;
        } else if(colorToMove == Color::Black && eval <= bestEval) {
            bestEval = eval;
            bestMove = move;
        }
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    if(verbose) {
        std::cout << "Time: " << duration.count() / 1000. << " seconds\n";
        std::cout << "Evaluated: " << nMovesSearched << " positions\n";
    }
    return bestMove;
}

U32 Engine::GetRandomMove() {   
    std::random_device seeder;
    std::mt19937 engine(seeder());
    std::uniform_int_distribution<U32> dist2(0, fGenerator->GetNLegalMoves() - 1);
    return fGenerator->GetMoveAt(dist2(engine));
}
