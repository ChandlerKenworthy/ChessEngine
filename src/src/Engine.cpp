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
    float material = 0.;
    material += (__builtin_popcountll(fBoard->GetBoard(Color::White, Piece::Pawn)) - __builtin_popcountll(fBoard->GetBoard(Color::Black, Piece::Pawn))) * VALUE_PAWN;
    material += (__builtin_popcountll(fBoard->GetBoard(Color::White, Piece::Bishop)) - __builtin_popcountll(fBoard->GetBoard(Color::Black, Piece::Bishop))) * VALUE_BISHOP;
    material += (__builtin_popcountll(fBoard->GetBoard(Color::White, Piece::Knight)) - __builtin_popcountll(fBoard->GetBoard(Color::Black, Piece::Knight))) * VALUE_KNIGHT;
    material += (__builtin_popcountll(fBoard->GetBoard(Color::White, Piece::Rook)) - __builtin_popcountll(fBoard->GetBoard(Color::Black, Piece::Rook))) * VALUE_ROOK;
    material += (__builtin_popcountll(fBoard->GetBoard(Color::White, Piece::Queen)) - __builtin_popcountll(fBoard->GetBoard(Color::Black, Piece::Queen))) * VALUE_QUEEN;
    return material;
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
/*
std::vector<U32> Engine::GenerateCaptureMoves(const std::unique_ptr<Board> &board) {
    // TODO: Rewrite this so it is much, much faster
    std::vector<U32> allMoves = GenerateLegalMoves(board, true);

    // Remove moves where the taken piece is Piece::Null
    allMoves.erase(std::remove_if(allMoves.begin(), allMoves.end(),
        [&](U32 move) { return GetMoveTakenPiece(move) == Piece::Null; }),
        allMoves.end());

    return allMoves;
}

float Engine::SearchAllCaptures(const std::unique_ptr<Board> &board, float alpha, float beta) {
    float eval = Evaluate(board);
    if(eval >= beta)
        return beta;
    alpha = std::max(alpha, eval);

    std::vector<U32> captureMoves = GenerateCaptureMoves(board); // Generate only captures
    OrderMoves(captureMoves);

    for(U32 move : captureMoves) {
        board->MakeMove(move);
        eval = -SearchAllCaptures(board, -alpha, -beta);
        board->UndoMove();

        if(eval >= beta)
            return beta;
        alpha = std::max(alpha, eval);
    }

    return alpha;
}
*/
std::pair<float, int> Engine::Minimax(int depth, float alpha, float beta) {
    // Return the evaluation up to depth [depth] and the number of moves explicitly searched
    if(depth == 0)
        return std::make_pair(Evaluate(), 1); // Was SearchAllCaptures(board, alpha, beta)

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

U32 Engine::GetBestMove() {
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
    std::cout << "Time: " << duration.count() / 1000. << " seconds\n";
    std::cout << "Evaluated: " << nMovesSearched << " positions\n";
    return bestMove;
}

U32 Engine::GetRandomMove() {   
    std::random_device seeder;
    std::mt19937 engine(seeder());
    std::uniform_int_distribution<U32> dist2(0, fGenerator->GetNLegalMoves() - 1);
    return fGenerator->GetMoveAt(dist2(engine));
}
