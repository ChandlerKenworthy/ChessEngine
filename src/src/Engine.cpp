#include <iostream>
#include <fstream>
#include <string>

#include "Engine.hpp"

Engine::Engine(const std::unique_ptr<Generator> &generator, const std::unique_ptr<Board> &board) : fGenerator(generator), fBoard(board) {

}

/*
float Engine::Evaluate(const std::unique_ptr<Board> &board) {
    float evaluation = GetMaterialEvaluation(board);
    int perspective = board->GetColorToMove() == Color::White ? 1. : -1.;
    return evaluation * perspective; // Return in centipawns rather than pawns
}

float Engine::GetMaterialEvaluation(const std::unique_ptr<Board> &board) {
    float material = 0.;
    material += (__builtin_popcountll(board->GetBoard(Color::White, Piece::Pawn)) - __builtin_popcountll(board->GetBoard(Color::Black, Piece::Pawn))) * VALUE_PAWN;
    material += (__builtin_popcountll(board->GetBoard(Color::White, Piece::Bishop)) - __builtin_popcountll(board->GetBoard(Color::Black, Piece::Bishop))) * VALUE_BISHOP;
    material += (__builtin_popcountll(board->GetBoard(Color::White, Piece::Knight)) - __builtin_popcountll(board->GetBoard(Color::Black, Piece::Knight))) * VALUE_KNIGHT;
    material += (__builtin_popcountll(board->GetBoard(Color::White, Piece::Rook)) - __builtin_popcountll(board->GetBoard(Color::Black, Piece::Rook))) * VALUE_ROOK;
    material += (__builtin_popcountll(board->GetBoard(Color::White, Piece::Queen)) - __builtin_popcountll(board->GetBoard(Color::Black, Piece::Queen))) * VALUE_QUEEN;
    return material;
}

void Engine::OrderMoves(const std::unique_ptr<Board> &board, std::vector<U32> &moves) {
    const U64 pawnAttacks = GetPawnAttacks(board, false);

    auto compareMoves = [&](U32 move1, U32 move2) {
        float move1ScoreEstimate = 0.;
        const int pieceType1 = (int)GetMovePiece(move1);
        const int takenPieceType1 = (int)GetMoveTakenPiece(move1);
        float move2ScoreEstimate = 0.;
        const int pieceType2 = (int)GetMovePiece(move2);
        const int takenPieceType2 = (int)GetMoveTakenPiece(move2);

        // Prioritise capturing opponent's most valudable pieces with our least valuable piece
        if(takenPieceType1 != (int)Piece::Null)
            move1ScoreEstimate += 10. * (PIECE_VALUES[takenPieceType1] - PIECE_VALUES[pieceType1]);
        if(takenPieceType2 != (int)Piece::Null)
            move2ScoreEstimate += 10. * (PIECE_VALUES[takenPieceType2] - PIECE_VALUES[pieceType2]);

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
    };

    std::sort(moves.begin(), moves.end(), compareMoves);
}

U64 Engine::GetPawnAttacks(const std::unique_ptr<Board> &board, bool colorToMoveAttacks) {
    // Find squares attacked by pawns for quick move ordering, does not take into account pins
    Color attackingColor = colorToMoveAttacks ? board->GetColorToMove() : (board->GetColorToMove() == Color::White ? Color::Black : Color::White);
    U64 pawns = board->GetBoard(attackingColor, Piece::Pawn);
    if(attackingColor == Color::White) {
        return north_east(pawns) | north_west(pawns);
    } else {
        return south_east(pawns) | south_west(pawns);
    }
}

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
    OrderMoves(board, captureMoves);

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

std::pair<float, int> Engine::Minimax(const std::unique_ptr<Board> &board, int depth, float alpha, float beta) {
    // Return the evaluation up to depth [depth] and the number of moves explicitly searched
    if(depth == 0)
        return std::make_pair(Evaluate(board), 1); // Was SearchAllCaptures(board, alpha, beta)

    int movesSearched = 0;
    std::vector<U32> moves = GenerateLegalMoves(board, true);

    Color movingColor = board->GetColorToMove();
    Color otherColor = movingColor == Color::White ? Color::Black : Color::White;
    if(moves.size() == 0) {
        if(IsUnderAttack(board->GetBoard(movingColor, Piece::King), otherColor, board))
            return std::make_pair(movingColor == Color::White ? -99999. : 99999., movesSearched); // Checkmate loss
        return std::make_pair(0., movesSearched); // Stalemate
    } 

    // For speed up, order the generated moves each iteration
    OrderMoves(board, moves);

    for(U32 move : moves) {
        board->MakeMove(move);
        movesSearched++;
        std::pair<float, int> result = Minimax(board, depth - 1, -beta, -alpha);
        float evaluation = -result.first;
        movesSearched += result.second;
        board->UndoMove();
        if(evaluation >= beta)
            return std::make_pair(beta, movesSearched);
        alpha = std::max(alpha, evaluation);
    }
    return std::make_pair(alpha, movesSearched);
}

U32 Engine::GetBestMove(const std::unique_ptr<Board> &board) {
    //auto start = std::chrono::high_resolution_clock::now();
    U32 bestMove{0};
    Color colorToMove = board->GetColorToMove();
    // If white is playing the worst eval is -999 (i.e. black completely winning)
    float bestEval = colorToMove == Color::White ? -9999. : 9999.;
    //int nMovesSearched = 0;

    // Order the moves for faster searching
    OrderMoves(board, fLegalMoves);

    // For each of the moves we want to find the "best" evaluation
    for(U32 move : fLegalMoves) {
        board->MakeMove(move);
        std::pair<float, int> result = Minimax(board, fMaxDepth - 1, -99999., 99999.);
        board->UndoMove();
        float eval = result.first;
        //nMovesSearched += result.second;
        if(colorToMove == Color::White && eval >= bestEval) {
            bestEval = eval;
            bestMove = move;
        } else if(colorToMove == Color::Black && eval <= bestEval) {
            bestEval = eval;
            bestMove = move;
        }
    }

    //auto stop = std::chrono::high_resolution_clock::now();
    //auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    //std::cout << "Time: " << duration.count() / 1000. << " seconds\n";
    //std::cout << "Evaluated: " << nMovesSearched << " positions\n";
    return bestMove;
}
*/

U32 Engine::GetRandomMove() {   
    std::random_device seeder;
    std::mt19937 engine(seeder());
    std::uniform_int_distribution<U32> dist2(0, fGenerator->GetNLegalMoves() - 1);
    U32 m = fGenerator->GetMoveAt(dist2(engine));
    fBoard->PrintDetailedMove(m);
    return m;
}
