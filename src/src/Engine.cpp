#include <iostream>
#include <fstream>
#include <string>

#include "Engine.hpp"

Engine::Engine(bool init) {
    fMaxDepth = 3;
    if(init) 
        Prepare();
}

void Engine::Prepare() {
    for(int iBit = 0; iBit < NSQUARES; iBit++) {
        U64 pos = 0; // TODO: Must be a better way to do this e.g. a one-liner
        set_bit(pos, iBit);
        BuildPawnAttackTable(pos);
        BuildKnightAttackTable(pos);
        BuildRookAttackTable(pos);
        BuildBishopAttackTable(pos);
        BuildQueenAttackTable(pos);
        BuildKingAttackTable(pos);
    }
}

void Engine::BuildPawnAttackTable(U64 pos) {
    // Note this does not include special pawn moves like en-passant or promotion
    U64 attacks = 0;
    // White pawn case (attacks in a northern direction)
    attacks |= north(pos);
    // Only valid when occupied by enemy piece aas these are taking moves
    attacks |= (north_east(pos) | north_west(pos));
    if(pos & RANK_2)
        attacks |= north(north(pos)); // Assumes nothing blocking the pawns path
    fWhitePawnAttacks[get_LSB(pos)] = attacks;

    attacks = 0; // Now case for the black pawn attacking in a southern direction
    attacks |= south(pos);
    attacks |= (south_east(pos) | south_west(pos));
    if(pos & RANK_7)
        attacks |= south(south(pos));
    fBlackPawnAttacks[get_LSB(pos)] = attacks;
}

void Engine::BuildKnightAttackTable(U64 pos) {
    // TODO: Check pos has exactly 1 on bit
    fKnightAttacks[get_LSB(pos)] = north(north_east(pos)) | north(north_west(pos)) | south(south_east(pos)) | south(south_west(pos)) | east(north_east(pos)) | east(south_east(pos)) | west(north_west(pos)) | west(south_west(pos));
}

void Engine::BuildBishopAttackTable(U64 pos) {
    // Vertical distance from primary diagonal is just abs(x - y) (zero indexed)
    // negative value = shift primary DOWN, positive value = shift primary UP
    U64 attacks = 0;
    // Vertical distance from the primary diagonal
    int dPrimaryDiag = (get_file_number(pos) - 1) - (8 - get_rank_number(pos));
    // Vertical distance from the secondary diagonal
    int dSecondaryDiag = (8 - get_file_number(pos)) - (8 - get_rank_number(pos));

    attacks = dPrimaryDiag > 0 ? PRIMARY_DIAGONAL << (abs(dPrimaryDiag) * 8) : PRIMARY_DIAGONAL >> (abs(dPrimaryDiag) * 8);

    attacks ^= dSecondaryDiag > 0 ? SECONDARY_DIAGONAL << (abs(dSecondaryDiag) * 8) : SECONDARY_DIAGONAL >> (abs(dSecondaryDiag) * 8);

    fBishopAttacks[get_LSB(pos)] = attacks;
}

void Engine::BuildQueenAttackTable(U64 pos) {
    U64 attacks = 0;
    // Vertical distance from the primary diagonal
    int dPrimaryDiag = (get_file_number(pos) - 1) - (8 - get_rank_number(pos));
    // Vertical distance from the secondary diagonal
    int dSecondaryDiag = (8 - get_file_number(pos)) - (8 - get_rank_number(pos));
    attacks = dPrimaryDiag > 0 ? PRIMARY_DIAGONAL << (abs(dPrimaryDiag) * 8) : PRIMARY_DIAGONAL >> (abs(dPrimaryDiag) * 8);
    attacks ^= dSecondaryDiag > 0 ? SECONDARY_DIAGONAL << (abs(dSecondaryDiag) * 8) : SECONDARY_DIAGONAL >> (abs(dSecondaryDiag) * 8);
    attacks ^= (get_rank(pos) ^ get_file(pos));
    fQueenAttacks[get_LSB(pos)] = attacks;
}

void Engine::BuildRookAttackTable(U64 pos) {
    // XOR as you cannot move to the square you currently occupy
    fRookAttacks[get_LSB(pos)] = get_rank(pos) ^ get_file(pos); 
}

void Engine::BuildKingAttackTable(U64 pos) {
    fKingAttacks[get_LSB(pos)] = north(pos) | east(pos) | west(pos) | south(pos) | north_east(pos) | north_west(pos) | south_east(pos) | south_west(pos);
}



float Engine::Evaluate(Board board) {
    float eval = GetMaterialEvaluation(board);
    return eval; // Return in centipawns rather than pawns
}

float Engine::GetMaterialEvaluation(Board board) {
    // TODO: Should value of pieces be functions of number of pieces on board?
    // E.g. bishops at start are weak but later on are really powerful on clear board
    float material = 0.;

    /*
    for(Piece p : PIECES) {
        if(p == Piece::King)
            continue; // Always kings on the board and king safety evaluated differently
        float white_value = GetPositionalEvaluation(board->GetBoard(Color::White, p), p, Color::White);
        float black_value = GetPositionalEvaluation(board->GetBoard(Color::Black, p), p, Color::Black);
        material += (white_value - black_value);
    }*/

    // For now use very simple material only heuristic + look-ahead to calculate board position
    // add heuristics once all mechanics are working correctly
    
    material += (__builtin_popcountll(board.GetBoard(Color::White, Piece::Pawn)) - __builtin_popcountll(board.GetBoard(Color::Black, Piece::Pawn))) * VALUE_PAWN;
    material += (__builtin_popcountll(board.GetBoard(Color::White, Piece::Bishop)) - __builtin_popcountll(board.GetBoard(Color::Black, Piece::Bishop))) * VALUE_BISHOP;
    material += (__builtin_popcountll(board.GetBoard(Color::White, Piece::Knight)) - __builtin_popcountll(board.GetBoard(Color::Black, Piece::Knight))) * VALUE_KNIGHT;
    material += (__builtin_popcountll(board.GetBoard(Color::White, Piece::Rook)) - __builtin_popcountll(board.GetBoard(Color::Black, Piece::Rook))) * VALUE_ROOK;
    material += (__builtin_popcountll(board.GetBoard(Color::White, Piece::Queen)) - __builtin_popcountll(board.GetBoard(Color::Black, Piece::Queen))) * VALUE_QUEEN;
    return material;
}

float Engine::Minimax(Board board, int depth, float alpha, float beta, Color maximisingPlayer) {
    // Working on a copy of the board object
    // Returns the maximum / minimum evaluation of a given position
    if(depth > fMaxDepth)
        depth = fMaxDepth;
    if(board.GetState() != State::Play || depth == 0)
        return Evaluate(board); // Return static evaluation of the current board
    if(maximisingPlayer == Color::White) {
        float maxEval = -99999.;
        board.GenerateLegalMoves();
        for(Move move : board.GetLegalMoves()) { // iterate through all possible moves for white in current position
            // Make the move, send a copy of the board down
            board.MakeMove(&move);
            float eval = Minimax(board, depth - 1, alpha, beta, Color::Black); // child is board after the move is made
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if(beta <= alpha)
                break;
            board.UndoMove();
        }
        return maxEval;
    } else {
        float minEval = 99999.;
        board.GenerateLegalMoves();
        for(Move move : board.GetLegalMoves()) {
            board.MakeMove(&move);
            float eval = Minimax(board, depth - 1, alpha, beta, Color::White);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if(beta <= alpha)
                break;
            board.UndoMove();
        }
        return minEval;
    }   
}

Move Engine::GetBestMove(Board board) {
    board.GenerateLegalMoves();
    std::mt19937 rng(fRandomDevice());
    std::uniform_int_distribution<std::mt19937::result_type> dist2(0,1); // distribution in range [0, 1]

    Move bestMove;
    float bestEval = board.GetColorToMove() == Color::White ? -999. : 999;
    // For each of the moves we want to find the "best" evaluation
    for(Move move : board.GetLegalMoves()) {
        float eval = Minimax(board, fMaxDepth, -99999., 99999., board.GetColorToMove()); // Seg fault happening in here
        if(board.GetColorToMove() == Color::White && eval >= bestEval) {
            bestEval = eval;
            if(eval == bestEval && (bool)dist2(rng)) // Randomly pick which of the best evaluated moves to use
                bestMove = move;

        } else if(board.GetColorToMove() == Color::Black && eval <= bestEval) {
            bestEval = eval;
            if(eval == bestEval && (bool)dist2(rng))
                bestMove = move;
        }
    }
    return bestMove;
}