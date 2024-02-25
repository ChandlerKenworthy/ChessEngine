#include <iostream>
#include <fstream>
#include <string>

#include "Engine.hpp"

Engine::Engine(const std::unique_ptr<Generator> &generator, const std::unique_ptr<Board> &board, const int maxDepth) : fGenerator(generator), fBoard(board), fMaxDepth(maxDepth) {

}


float Engine::Evaluate() {
    fOtherColor = fBoard->GetColorToMove() == Color::White ? Color::Black : Color::White;
    float evaluation = GetMaterialEvaluation();

    //evaluation += ForceKingToCornerEndgame();
    int perspective = fBoard->GetColorToMove() == Color::White ? 1. : -1.;
    return evaluation * perspective; // Return in centipawns rather than pawns
}

float Engine::ForceKingToCornerEndgame() {
    // Endgame weight is inversely proportional to the number of major pieces the opponent has
    float evaluation = 0.0;

    // Favour positions where the enemy king is in the corner of the board
    const U64 enemyKing = fBoard->GetBoard(fOtherColor, Piece::King);
    const U64 myKing = fBoard->GetBoard(Piece::King);
    const int file = get_file_number(enemyKing);
    const int rank = get_rank_number(enemyKing);
    const int distToCentreRank = std::max(4 - rank, rank - 5);
    const int distToCentreFile = std::max(4 - file, file - 5);
    const int distToCentre = distToCentreRank + distToCentreFile;
    evaluation += distToCentre;

    // Favour when the king moves towards the enemy to cut that king off
    const int rankDist = std::abs(rank - get_rank_number(myKing));
    const int fileDist = std::abs(file - get_file_number(myKing));
    const int distBetweenKings = rankDist + fileDist;
    evaluation += (14 - distBetweenKings);

    return 10.0 * fBoard->GetEndgameWeight() * evaluation;
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

std::pair<float, int> Engine::SearchAllCaptures(float alpha, float beta) {
    // Called when minimax hits maximum depth
    // TODO: Count the number of moves this seaches and send back to minimax
    int nMovesSearched = 1;
    float eval = Evaluate();
    if(eval >= beta) {
        //std::cout << "Search all broke early since eval >= beta\n";
        return std::make_pair(eval, nMovesSearched);
    }
    alpha = std::max(alpha, eval);

    fGenerator->GenerateCaptureMoves(fBoard);
    std::vector<U32> captureMoves = fGenerator->GetCaptureMoves(); // Get only capture moves
    //std::cout << "Generated " << captureMoves.size() << " captures for colour " << (int)fBoard->GetColorToMove() << "\n";
    OrderMoves(captureMoves);

    for(U32 move : captureMoves) {
        fBoard->MakeMove(move);
        std::pair<float, int> result = SearchAllCaptures(-alpha, -beta);
        eval = -result.first;
        nMovesSearched += result.second;
        fBoard->UndoMove();
        if(eval >= beta)
            return std::make_pair(alpha, nMovesSearched);
        alpha = std::max(alpha, eval);
    }
    return std::make_pair(alpha, nMovesSearched);
}

std::pair<float, int> Engine::Minimax(int depth, float alpha, float beta) {
    // Return the evaluation up to depth [depth] and the number of moves explicitly searched
    if(depth == 0) //  ...(Evaluate(), 1) 
        return SearchAllCaptures(alpha, beta); //std::make_pair(Evaluate(), 1); 

    int movesSearched = 0;
    fGenerator->GenerateLegalMoves(fBoard); // Move has been made in GetBestMove so need to find legal moves again
    std::vector<U32> moves = fGenerator->GetLegalMoves(); // Set of legal moves for this position
    //std::cout << "Depth: " << depth << " Moves: " << moves.size() << "\n";

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
        //std::cout << "Depth = " << fMaxDepth - depth << " ";
        //fBoard->PrintDetailedMove(move);
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
    for(U32 move : moves) { // this loop accounts for one order of depth already
        fBoard->MakeMove(move);
        //std::cout << "Depth: " << 0 << " ";  
        //fBoard->PrintDetailedMove(move);
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
