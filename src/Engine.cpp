#include <iostream>
#include <fstream>
#include <string>

#include "Engine.hpp"

Engine::Engine(const std::shared_ptr<Generator> &generator, const std::shared_ptr<Board> &board, const int maxDepth) : fGenerator(generator), fBoard(board), fMaxCacheSize(400000), fMaxDepth(maxDepth) {
    fEvaluationCache.clear();
    const size_t initialBucketCount = fMaxCacheSize / 0.75; // Load factor of 0.75 is typically used for unordered_maps
    fEvaluationCache.reserve(initialBucketCount);
}


float Engine::Evaluate() {
    // Returns the evaluation already based on the side to move e.g. could return +1.6 or -1.2
    fGamePhase = fBoard->GetGamePhase();
    // See if we have evaluated this board before (via a transposition)
    // Hash not perfectly unique but "unique" enough, extremely unlikely to cause problems
    const U64 thisHash = fBoard->GetHash();
    // Search the transposition table to see if we have evaluated this position before
    auto it = fEvaluationCache.find(thisHash);
    if(it != fEvaluationCache.end()) { // We've evaluated this position before
        fNHashesFound++;
        fLruList.erase(it->second.second); // Remove from current position, Update LRU list
        fLruList.push_front(thisHash); // Move to front (most recently used)
        it->second.second = fLruList.begin(); // Update iterator in the map
        return it->second.first;
    }

    fOtherColor = fBoard->GetColorToMove() == Color::White ? Color::Black : Color::White;
    const int perspective = fBoard->GetColorToMove() == Color::White ? 1. : -1.;

    // Evaluate the position
    float evaluation = 0.0;
    // These functions are all defined such that positive values are GOOD for the colour to move
    // so if it's black to move we need to add on minus these values to the evaluation
    //evaluation += ForceKingToCornerEndgame();  // TODO: These functions must eval for both sides!!!
    if(fDifficulty > 1000)
        evaluation += EvaluatePassedPawns();
    if(fDifficulty > 800)
    evaluation += EvaluateKingSafety(); // Can be +ve or -ve
    // These functions are return negative values i.e. BAD for the colour to move, agian needs to be adjusted
    // by perspective
    if(fDifficulty > 700)
        evaluation += EvaluateBadBishops();
    if(fDifficulty > 900)
    evaluation += EvaluateIsolatedPawns();
    
    evaluation *= perspective;
    // Must be applied after the perspective flip
    evaluation += GetMaterialEvaluation(); // returns +ve if white advantage and -ve for black advantage

    // Store evaluation in the cache
    fEvaluationCache[thisHash] = {evaluation, fLruList.insert(fLruList.begin(), thisHash)};
    // Evict least recently used evaluations if cache exceeds maximum size
    if (fEvaluationCache.size() > fMaxCacheSize) {
        U64 toRemove = fLruList.back(); // Get the least recently used hash
        fLruList.pop_back(); // Remove from the end of LRU list
        fEvaluationCache.erase(toRemove); // Remove from the cache
    }
    return evaluation; // Return in centipawns rather than pawns
}

float Engine::EvaluateKingSafety() {
    float eval = 0.0;

    const Color colorToMove = fBoard->GetColorToMove();
    for(Color c : {Color::White, Color::Black}) {
            // The king whose safety is being evaluated
        const int isMover = c == colorToMove ? 1 : -1;
        const U64 king = fBoard->GetBoard(c, Piece::King);
        const U64 pawns = fBoard->GetBoard(c, Piece::Pawn);
        bool kingInCorner = c == Color::White ? king & WHITE_KING_CORNERS : king & BLACK_KING_CORNERS;
        // Count the number of pawns in front of a diaognally in front of the king
        int nGuardingPawns = 0;
        if(c == Color::White) {
            nGuardingPawns = __builtin_popcountll(pawns & (north(king) | north_east(king) | north_west(king)));
        } else {
            nGuardingPawns = __builtin_popcountll(pawns & (south(king) | south_east(king) | south_west(king)));
        }
        if(fGamePhase < 0.5) {
            // Early game reward kings that are safelty tucked in a corner behind the pawns
            // Penalise kings running aimlessly around the board on a suicide mission
            eval += isMover * (kingInCorner ? 10.0 : -10.0);
            eval += isMover * (fPawnGuardKingEval[nGuardingPawns]);
        } else {
            // Reverse of the above is true for later game phases
            eval += isMover * (kingInCorner ? -10.0 : 10.0);
        }
    }
    return eval;
}

float Engine::EvaluateBadBishops() {
    float penalty = 0.0;

    const Color ctm = fBoard->GetColorToMove();
    for(Color c : {Color::White, Color::Black}) {
        const U64 allBishops = fBoard->GetBoard(c, Piece::Bishop);
        U64 whiteBishops = allBishops & WHITE_SQUARES;
        U64 blackBishops = allBishops & BLACK_SQUARES;
        U64 pawns = fBoard->GetBoard(c, Piece::Pawn);
        const int perspective = ctm == c ? 1 : -1;

        // First search your white squared bishop
        const int step = c == Color::Black ? 1 : -1;
        const U8 maxRank = c == Color::Black ? 8 : 1;
        int j = 0;

        for(U64 bishops : {whiteBishops, blackBishops}) {
        while(bishops) {
            const U64 bishop = 1ULL << __builtin_ctzll(bishops);
            const U64 rank = get_rank(bishop);
            const U64 squares = j == 0 ? WHITE_SQUARES : BLACK_SQUARES;
            const U8 rankNo = get_rank_number(bishop);
            for(int i = 1; rank <= abs(maxRank - rankNo); i += step) {
                const U64 thisRank = RANKS[(rankNo + (i * step)) - 1];
                while(thisRank & pawns & squares) { // Pawns on both sides can block the bishop
                    penalty += (perspective * fBadBishopPawnRankAwayPenalty[i - 1]);
                    pawns &= pawns - 1;
                }
            }
            bishops &= bishops - 1;
        }
        j++;
    }
    }

    return penalty; // this will be a negative number
}

float Engine::EvaluateIsolatedPawns() {
    float penalty = 0.0;
    int nIsolated = 0;
    U64 myPawns = fBoard->GetBoard(Piece::Pawn);
    const U64 myPawnsStatic = myPawns;
    while(myPawns) {
        const U64 pawn = 1ULL << __builtin_ctzll(myPawns);
        const U64 file = get_file(myPawns);
        const U8 fileNumber = get_file_number(pawn);
        if(((east(file) | west(file)) & myPawnsStatic) == 0) {
            // Isolated pawn, add penalty based on position, pawns at centre are weaker
            penalty += fIsolatedPawnPenaltyByFile[fileNumber - 1]; // Values of fIsolatedPawnPenaltyByFile are negative
            nIsolated++;
        }
        myPawns &= myPawns - 1;
    }

    // To be fair you must also evaluate your opponents pawns in the same fashion
    nIsolated = 0;
    U64 enemyPawns = fBoard->GetBoard(fBoard->GetColorToMove() == Color::White ? Color::Black : Color::White, Piece::Pawn);
    const U64 enemyPawnsStatic = enemyPawns;
    while(enemyPawns) {
        const U64 pawn = 1ULL << __builtin_ctzll(enemyPawns);
        const U64 file = get_file(enemyPawns);
        const U8 fileNumber = get_file_number(pawn);
        if(((east(file) | west(file)) & enemyPawnsStatic) == 0) {
            // Isolated pawn, add penalty based on position, pawns at centre are weaker
            // Use -= since if our opponent has isolated pawns we apply a -ve penalty -ve.ly i.e. position is better for us
            penalty -= fIsolatedPawnPenaltyByFile[fileNumber - 1]; // Values of fIsolatedPawnPenaltyByFile are negative
            nIsolated++;
        }
        enemyPawns &= enemyPawns - 1;
    }
    return penalty;
}

float Engine::EvaluatePassedPawns() {
    U64 myPawns = fBoard->GetBoard(Piece::Pawn);
    U64 enemyPawns = fBoard->GetBoard(fOtherColor, Piece::Pawn);
    U8 promotionRankNumber = fOtherColor == Color::Black ? 8 : 1;
    float bonus = 0.0;
    while(myPawns) {
        const U64 pawn = 1ULL << __builtin_ctzll(myPawns);
        const U8 rankNo = get_rank_number(pawn);
        U64 passedMask = get_file(pawn);
        passedMask |= (east(passedMask) | west(passedMask));
        // Now only allow ranks higher than yours in the defined attacking direction
        for(int iRank = 0; iRank < 8; ++iRank) {
            if(fOtherColor == Color::Black) { // You are white
                if(iRank <= rankNo - 1)
                    passedMask &= !RANKS[iRank];
            } else { // You are black
                if(iRank >= rankNo - 1)
                    passedMask &= !RANKS[iRank];
            }
        }
        if(passedMask & !enemyPawns) {
            // this is in range [1, 6] so -1 to put in range of lookup table
            bonus += fPassPawnBonus[abs(promotionRankNumber - rankNo) - 1]; 
        }
        myPawns &= myPawns - 1;
    }

    myPawns = fBoard->GetBoard(Piece::Pawn);
    enemyPawns = fBoard->GetBoard(fOtherColor, Piece::Pawn);
    promotionRankNumber = fOtherColor == Color::Black ? 1 : 8;
    while(enemyPawns) {
        const U64 pawn = 1ULL << __builtin_ctzll(enemyPawns);
        const U8 rankNo = get_rank_number(pawn);
        U64 passedMask = get_file(pawn);
        passedMask |= (east(passedMask) | west(passedMask));
        // Now only allow ranks higher than yours in the defined attacking direction
        for(int iRank = 0; iRank < 8; ++iRank) {
            if(fOtherColor == Color::Black) { // Enemy is black
                if(iRank <= rankNo - 1)
                    passedMask &= !RANKS[iRank];
            } else { // You are black
                if(iRank >= rankNo - 1)
                    passedMask &= !RANKS[iRank];
            }
        }
        if(passedMask & !myPawns) { // enemy has a passed pawn
            // this is in range [1, 6] so -1 to put in range of lookup table
            bonus -= fPassPawnBonus[abs(promotionRankNumber - rankNo) - 1]; // subtract bonus, enemy having passed pawns is bad!
        }
        enemyPawns &= enemyPawns - 1;
    }

    return bonus;
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
    material += EvaluateKnightPositions();
    material += EvaluateQueenPositions();
    material += EvaluateRookPositions();
    material += EvaluateBishopPositions();
    //material += EvaluateKingPositions();

    //material += (__builtin_popcountll(fBoard->GetBoard(Color::White, Piece::Knight)) - __builtin_popcountll(fBoard->GetBoard(Color::Black, Piece::Knight))) * VALUE_PAWN;
    //material += (__builtin_popcountll(fBoard->GetBoard(Color::White, Piece::Bishop)) - __builtin_popcountll(fBoard->GetBoard(Color::Black, Piece::Bishop))) * VALUE_PAWN;
    //material += (__builtin_popcountll(fBoard->GetBoard(Color::White, Piece::Rook)) - __builtin_popcountll(fBoard->GetBoard(Color::Black, Piece::Rook))) * VALUE_PAWN;
    //material += (__builtin_popcountll(fBoard->GetBoard(Color::White, Piece::Queen)) - __builtin_popcountll(fBoard->GetBoard(Color::Black, Piece::Queen))) * VALUE_QUEEN;
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

float Engine::SearchAllCaptures(float alpha, float beta, bool maximising) {
    // TODO: Should this extend the search for checks as well?
    fGenerator->GenerateCaptureMoves(fBoard);
    if(fGenerator->GetNCaptureMoves() == 0) // Nothing to search, return evaluation of the position
        return Evaluate();

    std::vector<U16> captureMoves = fGenerator->GetCaptureMoves(); // Get only capture moves
    //for(U16 move : captureMoves) {
    //    fBoard->PrintDetailedMove(move);
    //}
    
    if(maximising) {
        float maxEval = MIN_EVAL;
        for(U16 move : captureMoves) {
            fBoard->MakeMove(move);
            float evaluation = SearchAllCaptures(alpha, beta, false);
            fNMovesSearched++;
            fBoard->UndoMove();
            maxEval = std::max(maxEval, evaluation); // White picks the move which maximises the score
            alpha = std::max(alpha, evaluation);
            if(beta <= alpha) // Prune the branch
                break;
        }
        return maxEval;
    } else {
        float minEval = MAX_EVAL;
        for(U16 move : captureMoves) {
            fBoard->MakeMove(move);
            float evaluation = SearchAllCaptures(alpha, beta, true);
            fNMovesSearched++;
            fBoard->UndoMove();
            minEval = std::min(minEval, evaluation); // White picks the move which maximises the score
            beta = std::min(beta, evaluation);
            if(beta <= alpha) // Prune the branch
                break;
        }
        return minEval;
    }
}

float Engine::Search(U8 depth, float alpha, float beta, bool maximising) {
    if(depth == 0) {
        fNMovesSearched++;
        //float x = SearchAllCaptures(alpha, beta, maximising);
        float x = Evaluate();
        //fBoard->PrintFEN();
        return x; //SearchAllCaptures(alpha, beta, maximising);
    }

    fGenerator->GenerateLegalMoves(fBoard);
    if(fGenerator->GetNLegalMoves() == 0) { // No need to search we are at the end of the game tree on this branch
        const Color movingColor = fBoard->GetColorToMove(); // Who has 0 legal moves remaining
        const Color otherColor = movingColor == Color::White ? Color::Black : Color::White;
        const bool inCheck = fGenerator->IsUnderAttack(fBoard->GetBoard(movingColor, Piece::King), otherColor, fBoard);
        if(inCheck) { // No legal moves and you are in check(mate) so negative infinity in correct "direction"
            return movingColor == Color::White ? MIN_EVAL : MAX_EVAL; 
        } else { // Must be a stalemate - so completely even position
            return 0.0f;
        }
    }

    std::vector<U16> moves = fGenerator->GetLegalMoves();
    if(maximising) {
        float maxEval = MIN_EVAL;
        for(U16 move : moves) {
            fBoard->MakeMove(move);
            float evaluation = Search(depth - 1, alpha, beta, false);
            fBoard->UndoMove();
            maxEval = std::max(maxEval, evaluation); // White picks the move which maximises the score
            alpha = std::max(alpha, evaluation);
            if(beta <= alpha) { // Prune the branch
                break;
            }
        }
        return maxEval;
    } else {
        float minEval = MAX_EVAL;
        for(U16 move : moves) {
            fBoard->MakeMove(move);
            float evaluation = Search(depth - 1, alpha, beta, true);
            fBoard->UndoMove();
            minEval = std::min(minEval, evaluation); // Black picks the move which minimises the score
            beta = std::min(beta, evaluation);
            if(beta <= alpha) {// Prune the branch
                break;
            }
        }
        return minEval;
    }
}

U16 Engine::GetBestMove(const bool verbose) {
    // This is based on the negamax algorithm (e.g. https://en.wikipedia.org/wiki/Negamax)
    // but in this case my Evaluate() function returns the perspective-based evaluation already. 
    // Such that +ve values favour white and -ve value favour black. Branches are then pruned 
    // based on this fact using alpha-beta pruning
    auto start = std::chrono::high_resolution_clock::now();
    U16 bestMove = 0;
    fNHashesFound = 0;

    // Get the legal moves that we have to choose from (i.e. depth = 1 moves)
    std::vector<U16> primaryMoves = fGenerator->GetLegalMoves();
    if(primaryMoves.size() == 1) {
        return primaryMoves.at(0);
    }

    // Order moves to speed up alpha-beta pruning
    OrderMoves(primaryMoves);

    // Must take into account colour such that more negative values are better for black
    Color colorToMove = fBoard->GetColorToMove();
    float bestEvaluation = colorToMove == Color::White ? MIN_EVAL : MAX_EVAL;
    float alpha0 = MIN_EVAL; // Opposite as 1 ply from first for loop
    float beta0 = MAX_EVAL; // Opposite as 1 ply from first for loop
    fNMovesSearched = 0;

    // This primary for loop accounts for 1 level of depth
    for(U16 primaryMove : primaryMoves) {
        fBoard->MakeMove(primaryMove);
        // Find the evaluation up to a specified depth using the minimax search algorithm
        // opposite way round as 1 ply of search already accounted for by this loop
        float evaluation = Search(fMaxDepth - 1, alpha0, beta0, colorToMove == Color::White ? false : true);
        fBoard->UndoMove();
        if((evaluation > bestEvaluation && colorToMove == Color::White) ||
            evaluation < bestEvaluation && colorToMove == Color::Black) {
            bestEvaluation = evaluation;
            bestMove = primaryMove;
        }
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    if(verbose) {
        std::cout << "Search took " << duration.count() << " ms (" << duration.count() * 0.001 <<" s)\n";
        std::cout << "Evaluation = " << bestEvaluation << " centipawn\n";
        std::cout << "Positions searched = " << fNMovesSearched << " Hashes used = " << fNHashesFound << "\n";
    }

    // TODO: Catch if best move was never updated such that we yield all zeros (null/invalid move)
    return bestMove;
}

U16 Engine::GetRandomMove() {   
    std::random_device seeder;
    std::mt19937 engine(seeder());
    std::uniform_int_distribution<U16> dist2(0, fGenerator->GetNLegalMoves() - 1);
    return fGenerator->GetMoveAt(dist2(engine));
}
