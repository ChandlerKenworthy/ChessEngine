#include <iostream>
#include <fstream>
#include <string>

#include "Engine.hpp"

Engine::Engine(const std::unique_ptr<Generator> &generator, const std::unique_ptr<Board> &board, const int maxDepth) : fGenerator(generator), fBoard(board), fMaxDepth(maxDepth) {
    fEvaluationCache.clear();
}


float Engine::Evaluate() {
    //fGamePhase = fBoard->GetGamePhase();
    // See if we have evaluated this board before (via a transposition)
    // Hash not perfectly unique but "unique" enough, extremely unlikely to cause problems
    //const U64 thisHash = fBoard->GetHash();
    const int perspective = fBoard->GetColorToMove() == Color::White ? 1. : -1.;
    // Search the transposition table to see if we have evaluated this position before
    //auto it = fEvaluationCache.find(thisHash);
    //if(it != fEvaluationCache.end()) {
    //    fNHashesFound++;
    //    return it->second * perspective;
    //}

    fOtherColor = fBoard->GetColorToMove() == Color::White ? Color::Black : Color::White;
    float evaluation = GetMaterialEvaluation(); // returns +ve if white advantage and -ve for black advantage
    //evaluation += ForceKingToCornerEndgame();
    //fEvaluationCache[thisHash] = evaluation; // TODO: Prune the evaluation cache size to stop it exponentially increasing
    return evaluation * perspective; // Return in centipawns rather than pawns (always +ve value)
}

float Engine::ForceKingToCornerEndgame() {
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

    return 10.0 * fGamePhase * evaluation;
}

float Engine::GetMaterialEvaluation() {
    // Counts material and respects the position of the material e.g. knights in the centre are stronger
    float material = 0.;
    //material += EvaluateKnightPositions();
    //material += EvaluateQueenPositions();
    //material += EvaluateRookPositions();
    //material += EvaluateBishopPositions();
    //material += EvaluateKingPositions();

    material += (__builtin_popcountll(fBoard->GetBoard(Color::White, Piece::Knight)) - __builtin_popcountll(fBoard->GetBoard(Color::Black, Piece::Knight))) * VALUE_PAWN;
    material += (__builtin_popcountll(fBoard->GetBoard(Color::White, Piece::Bishop)) - __builtin_popcountll(fBoard->GetBoard(Color::Black, Piece::Bishop))) * VALUE_PAWN;
    material += (__builtin_popcountll(fBoard->GetBoard(Color::White, Piece::Rook)) - __builtin_popcountll(fBoard->GetBoard(Color::Black, Piece::Rook))) * VALUE_PAWN;
    material += (__builtin_popcountll(fBoard->GetBoard(Color::White, Piece::Queen)) - __builtin_popcountll(fBoard->GetBoard(Color::Black, Piece::Queen))) * VALUE_QUEEN;
    material += (__builtin_popcountll(fBoard->GetBoard(Color::White, Piece::Pawn)) - __builtin_popcountll(fBoard->GetBoard(Color::Black, Piece::Pawn))) * VALUE_PAWN;
    return material;
}

float Engine::EvaluateKingPositions() {
    float val = 0.;
    U64 white_king = fBoard->GetBoard(Color::White, Piece::King);
    U64 black_king = fBoard->GetBoard(Color::Black, Piece::King);
    float beginWhite = fKingPosModifier[0][__builtin_ctzll(white_king)];
    float endWhite = fKingPosModifier[2][__builtin_ctzll(white_king)];

    float beginBlack = fKingPosModifier[1][__builtin_ctzll(black_king)];
    float endBlack = fKingPosModifier[3][__builtin_ctzll(black_king)];

    val += (VALUE_KING + (beginWhite + (fGamePhase * (endWhite - beginWhite))));
    val -= (VALUE_KING + (beginBlack + (fGamePhase * (endBlack - beginBlack))));
    return val;
}

float Engine::EvaluateKnightPositions() {
    float val = 0.;
    U64 white_knights = fBoard->GetBoard(Color::White, Piece::Knight);
    U64 black_knights = fBoard->GetBoard(Color::Black, Piece::Knight);
    while(white_knights) {
        val += (VALUE_KNIGHT + fKnightPosModifier[__builtin_ctzll(white_knights)]);
        white_knights &= white_knights - 1;
    }
    while(black_knights) {
        val -= (VALUE_KNIGHT + fKnightPosModifier[__builtin_ctzll(black_knights)]);
        black_knights &= black_knights - 1;
    }
    return val;
}

float Engine::EvaluateQueenPositions() {
    float val = 0.;
    U64 white_queens = fBoard->GetBoard(Color::White, Piece::Queen);
    U64 black_queens = fBoard->GetBoard(Color::Black, Piece::Queen);
    while(white_queens) {
        val += (VALUE_QUEEN + fQueenPosModifier[__builtin_ctzll(white_queens)]);
        white_queens &= white_queens - 1;
    }
    while(black_queens) {
        val -= (VALUE_QUEEN + fQueenPosModifier[__builtin_ctzll(black_queens)]);
        black_queens &= black_queens - 1;
    }
    return val;
}

float Engine::EvaluateRookPositions() {
    float val = 0.;
    U64 white_rooks = fBoard->GetBoard(Color::White, Piece::Rook);
    U64 black_rooks = fBoard->GetBoard(Color::Black, Piece::Rook);
    while(white_rooks) {
        val += (VALUE_ROOK + fRookPosModifier[0][__builtin_ctzll(white_rooks)]);
        white_rooks &= white_rooks - 1;
    }
    while(black_rooks) {
        val -= (VALUE_ROOK + fRookPosModifier[1][__builtin_ctzll(black_rooks)]);
        black_rooks &= black_rooks - 1;
    }
    return val;
}

float Engine::EvaluateBishopPositions() {
    float val = 0.;
    U64 white_bishops = fBoard->GetBoard(Color::White, Piece::Bishop);
    U64 black_bishops = fBoard->GetBoard(Color::Black, Piece::Bishop);
    while(white_bishops) {
        val += (VALUE_BISHOP + fBishopPosModifier[0][__builtin_ctzll(white_bishops)]);
        white_bishops &= white_bishops - 1;
    }
    while(black_bishops) {
        val -= (VALUE_BISHOP + fBishopPosModifier[1][__builtin_ctzll(black_bishops)]);
        black_bishops &= black_bishops - 1;
    }
    return val;
}

void Engine::OrderMoves(std::vector<U16> &moves) {
    const U64 pawnAttacks = fGenerator->GetPawnAttacks(fBoard, false);

    std::sort(moves.begin(), moves.end(), [&](U16 move1, U16 move2) {
        float move1ScoreEstimate = 0.;
        float move2ScoreEstimate = 0.;
        const int pieceType1 = (int)fBoard->GetMovePiece(move1);
        const int takenPieceType1 = (int)fBoard->GetMoveTakenPiece(move1);
        const int pieceType2 = (int)fBoard->GetMovePiece(move2);
        const int takenPieceType2 = (int)fBoard->GetMoveTakenPiece(move2);

        // Prioritise capturing opponent's most valuable pieces with our least valuable piece
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
    std::vector<U16> captureMoves = fGenerator->GetCaptureMoves(); // Get only capture moves
    //std::cout << "Generated " << captureMoves.size() << " captures for colour " << (int)fBoard->GetColorToMove() << "\n";
    OrderMoves(captureMoves);

    for(U16 move : captureMoves) {
        fBoard->MakeMove(move);
        fBoard->PrintDetailedMove(move);
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

float Engine::Search(U8 depth, float alpha, float beta) {
    // Returns the evaluation of a position after searching to the specified depth. 
    // Evaluations are returned in centi-pawns (100 cp = 1 pawn). Larger positive
    // values are better for white. This function includes alpha-beta pruning and
    // is based on the negamax function.
    if(depth == 0) {
        fNMovesSearched++;
        return Evaluate();
    }

    fGenerator->GenerateLegalMoves(fBoard);
    std::vector<U16> moves = fGenerator->GetLegalMoves();

    const Color movingColor = fBoard->GetColorToMove();

    if(moves.size() == 0) {
         // These can change on recursive calls
        const Color otherColor = movingColor == Color::White ? Color::Black : Color::White;
        const bool inCheck = fGenerator->IsUnderAttack(fBoard->GetBoard(movingColor, Piece::King), otherColor, fBoard);
        if(inCheck) { // No legal moves and you are in check(mate) so negative infinity in correct "direction"
            return movingColor == Color::White ? MIN_EVAL : MAX_EVAL; 
        } else { // Must be a stalemate - so completely even position
            return 0.0;
        }
    }

    // Order moves to speed-up alpha-beta pruning
    OrderMoves(moves);

    for(U16 move : moves) {
        fBoard->MakeMove(move);
        // Minus sign to flip perspective (what is good for our opponent is bad for us -- negamax)
        float evaluation = -Search(depth - 1, -beta, -alpha); // opposite way around to args of function
        fBoard->UndoMove();
        if(evaluation >= beta)
            return beta;
        alpha = std::max(alpha, evaluation);
    }
    return alpha;
}

U16 Engine::GetBestMove() {
    // TODO: Add hash table stuff
    const U8 searchDepth = 4;
    auto start = std::chrono::high_resolution_clock::now();
    U16 bestMove = 0;
    // Must take into account colour such that more negative values are better for black
    Color colorToMove = fBoard->GetColorToMove();
    float bestEvaluation = colorToMove == Color::White ? MIN_EVAL : MAX_EVAL;
    fNMovesSearched = 0;

    // Get the legal moves that we have to choose from (i.e. depth = 1 moves)
    std::vector<U16> primaryMoves = fGenerator->GetLegalMoves();

    // Order moves to speed up alpha-beta pruning
    OrderMoves(primaryMoves);

    for(U16 primaryMove : primaryMoves) {
        //PrintMove(primaryMove);
        //std::cout << "\n";
        fBoard->MakeMove(primaryMove);
        float evaluation = Search(searchDepth - 1, MIN_EVAL, MAX_EVAL);
        std::cout << "eval = " << evaluation * 0.01 << "\n";
        fBoard->UndoMove();
        if(colorToMove == Color::White && evaluation > bestEvaluation) { // Use > not >= to prefer faster paths to mate
            bestEvaluation = evaluation;
            bestMove = primaryMove;
        } else if(colorToMove == Color::Black && evaluation < bestEvaluation) {
            bestEvaluation = evaluation;
            bestMove = primaryMove;
        }
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Search took " << duration.count() << " ms (" << duration.count() * 0.001 <<" s)\n";
    std::cout << "Evaluation = " << bestEvaluation / 100.0 << "\n";
    std::cout << "Positions searched = " << fNMovesSearched << "\n";

    // TODO: Catch if best move was never updated such that we yield all zeros (null/invalid move)
    return bestMove;
}


/*U16 Engine::GetBestMove(bool verbose) {
    fNHashesFound = 0;
    auto start = std::chrono::high_resolution_clock::now();
    U16 bestMove{0};
    Color colorToMove = fBoard->GetColorToMove();
    // If white is playing the worst eval is -999 (i.e. black completely winning)
    float bestEval = colorToMove == Color::White ? -MAX_EVAL : MAX_EVAL;
    int nMovesSearched = 0;

    // For each of the moves we want to find the "best" evaluation
    std::vector<U16> moves = fGenerator->GetLegalMoves();
    // Order the moves for faster searching
    OrderMoves(moves);
    for(U16 move : moves) { // this loop accounts for one order of depth already
        fBoard->MakeMove(move);
        //std::cout << "Depth: " << 0 << " ";  
        //fBoard->PrintDetailedMove(move);
        std::pair<float, int> result = Minimax(fMaxDepth - 1, -MAX_EVAL, MAX_EVAL);
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
        std::cout << "Evaluated: " << nMovesSearched << " positions (" << fNHashesFound << " hashes used)\n";
    }
    return bestMove;
}*/

U16 Engine::GetRandomMove() {   
    std::random_device seeder;
    std::mt19937 engine(seeder());
    std::uniform_int_distribution<U16> dist2(0, fGenerator->GetNLegalMoves() - 1);
    return fGenerator->GetMoveAt(dist2(engine));
}
