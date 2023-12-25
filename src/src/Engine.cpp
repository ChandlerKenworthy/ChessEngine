#include <iostream>
#include <fstream>
#include <string>

#include "Engine.hpp"

Engine::Engine(const bool init) {
    fMaxDepth = 3;
    fLastUnique = -1;
    if(init) 
        Prepare();
}

void Engine::Prepare() {
    for(int iBit = 0; iBit < NSQUARES; iBit++) {
        U64 pos = 0; // TODO: Must be a better way to do this e.g. a one-liner
        set_bit(pos, iBit);
        BuildPawnAttackTables(pos);
        BuildKnightAttackTable(pos);
        BuildKingAttackTable(pos);
        BuildStraightAttackTables(pos);
        BuildDiagonalAttackTables(pos);
    }
}

void Engine::BuildPawnAttackTables(const U64 pos) {
    // Note this does not include special pawn moves like en-passant or promotion
    U64 attacks = 0;
    uint8_t lsb = get_LSB(pos);
    // White pawn case (attacks in a northern direction)
    attacks |= north(pos);
    if(pos & RANK_2)
        attacks |= north(north(pos)); // Assumes nothing blocking the pawns path
    fWhitePawnForwardAttacks[lsb] = attacks;
    // Only valid when occupied by enemy piece aas these are taking moves
    fWhitePawnDiagonalAttacks[lsb] = north_east(pos) | north_west(pos);

    attacks = 0; // Now case for the black pawn attacking in a southern direction
    attacks |= south(pos);
    if(pos & RANK_7)
        attacks |= south(south(pos));
    fBlackPawnForwardAttacks[lsb] = attacks;
    fBlackPawnDiagonalAttacks[lsb] = south_east(pos) | south_west(pos);
}

void Engine::BuildKnightAttackTable(const U64 pos) {
    // TODO: Check pos has exactly 1 on bit
    fKnightAttacks[get_LSB(pos)] = north(north_east(pos)) | north(north_west(pos)) | south(south_east(pos)) | south(south_west(pos)) | east(north_east(pos)) | east(south_east(pos)) | west(north_west(pos)) | west(south_west(pos));
}

void Engine::BuildDiagonalAttackTables(const U64 pos) {
    // Vertical distance from primary diagonal is just abs(x - y) (zero indexed)
    // negative value = shift primary DOWN, positive value = shift primary UP
    // Vertical distance from the primary diagonal
    int dPrimaryDiag = (get_file_number(pos) - 1) - (8 - get_rank_number(pos));
    // Vertical distance from the secondary diagonal
    int dSecondaryDiag = (8 - get_file_number(pos)) - (8 - get_rank_number(pos));

    U64 primaryAttacks = dPrimaryDiag > 0 ? PRIMARY_DIAGONAL << (abs(dPrimaryDiag) * 8) : PRIMARY_DIAGONAL >> (abs(dPrimaryDiag) * 8);

    U64 secondaryAttacks = dSecondaryDiag > 0 ? SECONDARY_DIAGONAL << (abs(dSecondaryDiag) * 8) : SECONDARY_DIAGONAL >> (abs(dSecondaryDiag) * 8);

    fPrimaryDiagonalAttacks[get_LSB(pos)] = primaryAttacks ^ pos;
    fSecondaryDiagonalAttacks[get_LSB(pos)] = secondaryAttacks ^ pos;
}

void Engine::BuildStraightAttackTables(const U64 pos) {
    fPrimaryStraightAttacks[get_LSB(pos)] = get_rank(pos) ^ pos;
    fSecondaryStraightAttacks[get_LSB(pos)] = get_file(pos) ^ pos;
}

void Engine::BuildKingAttackTable(const U64 pos) {
    fKingAttacks[get_LSB(pos)] = north(pos) | east(pos) | west(pos) | south(pos) | north_east(pos) | north_west(pos) | south_east(pos) | south_west(pos);
}

void Engine::GenerateLegalMoves(const std::unique_ptr<Board> &board) {
    // Legal moves have already been calculated for this board configuration, don't recalculate
    if(fLastUnique == board->GetUnique())
        return;
    fLegalMoves.clear();
    GeneratePseudoLegalMoves(board);
    GenerateCastlingMoves(board);
    GenerateEnPassantMoves(board);
    // TODO: Prune out the illegal moves

    fLastUnique = board->GetUnique();
}

void Engine::GeneratePseudoLegalMoves(const std::unique_ptr<Board> &board) {
    GeneratePawnPseudoLegalMoves(board);
    GenerateKingPseudoLegalMoves(board);
    GenerateKnightPseudoLegalMoves(board);
    GenerateBishopPseudoLegalMoves(board);
    GenerateRookPseudoLegalMoves(board);
    GenerateQueenPseudoLegalMoves(board);
}

void Engine::GenerateEnPassantMoves(const std::unique_ptr<Board> &board) {
    if(board->GetNMovesMade() < MIN_MOVES_FOR_ENPASSANT) // Protect against seg fault + faster returns
        return;
    Move *lastMove = board->GetLastMove();
    // Faster return if you know en-passant will not be possible
    if(lastMove->piece != Piece::Pawn || lastMove->WasEnPassant || lastMove->WasCastling)
        return;

    U64 pawns = board->GetBoard(board->GetColorToMove(), Piece::Pawn);
    U64 enPassantPawns = 0;

    if(board->GetColorToMove() == Color::White && (lastMove->origin & RANK_7) && (lastMove->target & RANK_5)) {
        // Was a double-move forward with a pawn by black last turn
        // Check if the move placed the pawn on an adjacent file to any of your pawns on rank 5
        enPassantPawns = (east(lastMove->target) | west(lastMove->target)) & pawns;
    } else if(board->GetColorToMove() == Color::Black && (lastMove->origin & RANK_2) && (lastMove->target & RANK_4)) {
        // Was a double-move forward with a pawn by white last turn
        enPassantPawns = (east(lastMove->target) | west(lastMove->target)) & pawns;
    }
    while(enPassantPawns) {
        U64 pawn = 0;
        set_bit(pawn, pop_LSB(enPassantPawns));
        fLegalMoves.push_back(Move{
            pawn, 
            board->GetColorToMove() == Color::White ? north(lastMove->target) : south(lastMove->target), 
            Piece::Pawn, 
            Piece::Pawn,
            true
        });
    }
}

void Engine::GenerateCastlingMoves(const std::unique_ptr<Board> &board) {
    if(board->GetNMovesMade() < MIN_MOVES_FOR_CASTLING)
        return;

    U64 occupancy = board->GetBoard(Color::White) | board->GetBoard(Color::Black);
    U64 origin = board->GetBoard(board->GetColorToMove(), Piece::King);

    // Castling conditions for white
    if(board->GetColorToMove() == Color::White && !board->GetWhiteKingMoved()) {
        if(!board->GetWhiteKingsideRookMoved() && 
            IsCastlingPossible(KING_SIDE_CASTLING_MASK_WHITE, board)) 
        {
            fLegalMoves.push_back(Move{origin, RANK_1 & FILE_G, Piece::King, Piece::Null, false, true});
        }
        if(!board->GetWhiteQueensideRookMoved() &&
            IsCastlingPossible(QUEEN_SIDE_CASTLING_MASK_WHITE, board)) {
            fLegalMoves.push_back(Move{origin, RANK_1 & FILE_C, Piece::King, Piece::Null, false, true});
        }
    // Castling conditions for black
    } else if(board->GetColorToMove() == Color::Black && !board->GetBlackKingMoved()) {
        if (board->GetBlackKingsideRookMoved() &&
            IsCastlingPossible(KING_SIDE_CASTLING_MASK_BLACK, board)) 
        {
            fLegalMoves.push_back(Move{origin, RANK_1 & FILE_G, Piece::King, Piece::Null, false, true});
        }
        if (board->GetBlackQueensideRookMoved() &&
            IsCastlingPossible(QUEEN_SIDE_CASTLING_MASK_BLACK, board)) 
        {
            fLegalMoves.push_back(Move{origin, RANK_1 & FILE_C, Piece::King, Piece::Null, false, true});
        }
    }
}

bool Engine::IsCastlingPossible(U64 castlingMask, const std::unique_ptr<Board> &board) {
    U64 occ = board->GetBoard(Color::White) | board->GetBoard(Color::Black);
    return !(occ & castlingMask) && !IsUnderAttack(castlingMask, board->GetColorToMove() == Color::White ? Color::Black : Color::White, board);
}

bool Engine::IsUnderAttack(U64 mask, Color attackingColor, const std::unique_ptr<Board> &board) {
    // TODO: This will have to calculate pins at some point I expect...e.g. rook might not have all the moves
    // TODO: Factor out this function into separate chunks
    // check for pins by calculating sliding piece attakcs but take out the piece that wants to move in the occupancy
    U64 attacks = 0;
    U64 defender = board->GetBoard(attackingColor == Color::White ? Color::Black : Color::White);
    U64 attacker = board->GetBoard(attackingColor);
    U64 occ = defender | attacker;

    // Pawns (only diagonal forwards check as only care about attacks)
    U64 pawns = board->GetBoard(attackingColor, Piece::Pawn);
    while(pawns) {
        U64 pawn = 0;
        uint8_t lsb = pop_LSB(pawns);
        set_bit(pawn, lsb);
        attacks |= attackingColor == Color::White ? fWhitePawnDiagonalAttacks[lsb] : fBlackPawnDiagonalAttacks[lsb];
    }

    // Knights
    U64 knights = board->GetBoard(attackingColor, Piece::Knight);
    while(knights) {
        U64 knight = 0;
        uint8_t lsb = pop_LSB(knights);
        set_bit(knight, lsb);
        attacks |= fKnightAttacks[lsb];
    }
    // Bishops
    U64 bishops = board->GetBoard(attackingColor, Piece::Bishop);
    while(bishops) {
        U64 bishop = 0;
        uint8_t lsb = pop_LSB(bishop);
        set_bit(bishop, lsb);
        attacks |= (hypQuint(bishop, occ, fPrimaryDiagonalAttacks[lsb]) | hypQuint(bishop, occ, fSecondaryDiagonalAttacks[lsb]));
    }

    // Rooks
    U64 rooks = board->GetBoard(attackingColor, Piece::Rook);
    while(rooks) {
        U64 rook = 0;
        uint8_t lsb = pop_LSB(rook);
        set_bit(rook, lsb);
        attacks |= (hypQuint(rook, occ, fPrimaryStraightAttacks[lsb]) | hypQuint(rook, occ, fSecondaryStraightAttacks[lsb]));
    }

    // Queens
    U64 queens = board->GetBoard(attackingColor, Piece::Queen);
    while(queens) {
        U64 queen = 0;
        uint8_t lsb = pop_LSB(queen);
        set_bit(queen, lsb);
        attacks |= (hypQuint(queen, occ, fPrimaryStraightAttacks[lsb]) | hypQuint(queen, occ, fSecondaryStraightAttacks[lsb]) | hypQuint(queen, occ, fPrimaryDiagonalAttacks[lsb]) | hypQuint(queen, occ, fSecondaryDiagonalAttacks[lsb]));
    }

    // King
    U64 king = board->GetBoard(attackingColor, Piece::King);
    attacks |= fKingAttacks[get_LSB(king)];

    return attacks & mask & ~attacker;
}

void Engine::GeneratePawnPseudoLegalMoves(const std::unique_ptr<Board> &board) {
    U64 pawns = board->GetBoard(board->GetColorToMove(), Piece::Pawn);
    U64 enemy = board->GetBoard(board->GetColorToMove() == Color::White ? Color::Black : Color::White);
    U64 occ = enemy | board->GetBoard(board->GetColorToMove());
    while(pawns) {
        U64 pawn = 0;
        uint8_t lsb = pop_LSB(pawns);
        set_bit(pawn, lsb);

        // Only allow diagonal attacks if occupied by enemy piece
        U64 attacks = (board->GetColorToMove() == Color::White ? fWhitePawnDiagonalAttacks[lsb] : fBlackPawnDiagonalAttacks[lsb]) & enemy;

        // Get rid of 2-square attack if 1st or 2nd square is occupied, add single square attacks
        U64 oneSquareForward = (board->GetColorToMove() == Color::White ? north(pawn) : south(pawn)) & ~occ;
        attacks |= oneSquareForward;
        if(oneSquareForward)
            attacks |= (board->GetColorToMove() == Color::White ? north(north(pawn)) : south(south(pawn))) & ~occ;
        while(attacks) {
            U64 attack = 0;
            set_bit(attack, pop_LSB(attacks));
            fLegalMoves.push_back(Move{
                pawn,
                attack,
                Piece::Pawn,
                board->GetIsOccupied(attack).second,
            });
        }
    }
}

void Engine::GenerateKingPseudoLegalMoves(const std::unique_ptr<Board> &board) {
    U64 king = board->GetBoard(board->GetColorToMove(), Piece::King);
    U64 attacks = fKingAttacks[get_LSB(king)] & ~board->GetBoard(board->GetColorToMove());
    while(attacks) {
        U64 attack = 0;
        set_bit(attack, pop_LSB(attacks));
        Piece takenPiece = board->GetIsOccupied(attack).second;
        fLegalMoves.push_back(Move{
            king, 
            attack, 
            Piece::King, 
            takenPiece
        });
    }
}

void Engine::GenerateKnightPseudoLegalMoves(const std::unique_ptr<Board> &board) {
    U64 knights = board->GetBoard(board->GetColorToMove(), Piece::Knight);
    while(knights) {
        U64 knight = 0;
        uint8_t lsb = pop_LSB(knights);
        set_bit(knight, lsb);
        U64 attacks = fKnightAttacks[lsb] & ~board->GetBoard(board->GetColorToMove());
        while(attacks) {
            U64 attack = 0;
            set_bit(attack, pop_LSB(attacks));
            Piece takenPiece = board->GetIsOccupied(attack).second;
            fLegalMoves.push_back(Move{
                knight, 
                attack, 
                Piece::Knight, 
                takenPiece
            });
        }
    }
}

void Engine::GenerateBishopPseudoLegalMoves(const std::unique_ptr<Board> &board) {
    U64 bishops = board->GetBoard(board->GetColorToMove(), Piece::Bishop);
    U64 you = board->GetBoard(board->GetColorToMove());
    U64 occ = you | board->GetBoard(board->GetColorToMove() == Color::White ? Color::Black : Color::White);
    while(bishops) {
        U64 bishop = 0;
        uint8_t lsb = pop_LSB(bishops);
        set_bit(bishop, lsb);
        U64 attacks = (hypQuint(bishop, occ, fPrimaryDiagonalAttacks[lsb]) | hypQuint(bishop, occ, fSecondaryDiagonalAttacks[lsb])) & ~you;
        while(attacks) {
            U64 attack = 0;
            set_bit(attack, pop_LSB(attacks));
            fLegalMoves.push_back(Move{
                bishop,
                attack,
                Piece::Bishop,
                board->GetIsOccupied(attack).second
            });
        }
    }
}

void Engine::GenerateRookPseudoLegalMoves(const std::unique_ptr<Board> &board) {
    U64 rooks = board->GetBoard(board->GetColorToMove(), Piece::Rook);
    U64 you = board->GetBoard(board->GetColorToMove());
    U64 occ = you | board->GetBoard(board->GetColorToMove() == Color::White ? Color::Black : Color::White);
    while(rooks) {
        U64 rook = 0;
        uint8_t lsb = pop_LSB(rooks);
        set_bit(rook, lsb);
        U64 attacks = (hypQuint(rook, occ, fPrimaryDiagonalAttacks[lsb]) | hypQuint(rook, occ, fSecondaryStraightAttacks[lsb])) & ~you;
        while(attacks) {
            U64 attack = 0;
            set_bit(attack, pop_LSB(attacks));
            fLegalMoves.push_back(Move{
                rook,
                attack,
                Piece::Rook,
                board->GetIsOccupied(attack).second
            });
        }
    }
}

void Engine::GenerateQueenPseudoLegalMoves(const std::unique_ptr<Board> &board) {
    U64 queens = board->GetBoard(board->GetColorToMove(), Piece::Queen); // Could have multiple due to promotion
    U64 you = board->GetBoard(board->GetColorToMove());
    U64 occ = you | board->GetBoard(board->GetColorToMove() == Color::White ? Color::Black : Color::White);
    while(queens) {
        U64 queen = 0;
        uint8_t lsb = pop_LSB(queens);
        set_bit(queen, lsb);
        U64 attacks = (hypQuint(queen, occ, fPrimaryStraightAttacks[lsb]) | hypQuint(queen, occ, fSecondaryStraightAttacks[lsb]) | hypQuint(queen, occ, fPrimaryDiagonalAttacks[lsb]) | hypQuint(queen, occ, fSecondaryDiagonalAttacks[lsb])) & ~you;
        attacks = attacks & ~you;
        while(attacks) {
            U64 attack = 0;
            set_bit(attack, pop_LSB(attacks));
            fLegalMoves.push_back(Move{
                queen,
                attack,
                Piece::Queen,
                board->GetIsOccupied(attack).second // TODO: Is this needed for every move, maybe only do this when a move is made to save massive overhead?
            });
        }
    }                
}

bool Engine::GetMoveIsLegal(Move* move) {
    for(int iMove = 0; iMove < fLegalMoves.size(); iMove++) {
        Move* legalMove = &fLegalMoves[iMove];
        if((legalMove->origin & move->origin) && (legalMove->target & move->target)) {
            move->WasCastling = legalMove->WasCastling;
            move->WasEnPassant = legalMove->WasEnPassant;
            move->takenPiece = legalMove->takenPiece;
            return true;
        }
    }
    return false;
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
    /*if(depth > fMaxDepth)
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
    }*/
    return 0.;
}

Move Engine::GetBestMove(Board board) {
    /*board.GenerateLegalMoves();
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
    */
   return Move{U64{0}, U64{0}, Piece::Null};
}