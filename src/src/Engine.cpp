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
        U64 pos = 1ULL << iBit;
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

void Engine::CheckFiftyMoveDraw(const std::unique_ptr<Board> &board) {
    if(board->GetHalfMoveClock() == 100)
        board->SetState(State::FiftyMoveRule);
}

void Engine::CheckInsufficientMaterial(const std::unique_ptr<Board> &board) {
    uint8_t nBlackKnights = CountSetBits(board->GetBoard(Color::Black, Piece::Knight));
    uint8_t nBlackBishops = CountSetBits(board->GetBoard(Color::Black, Piece::Bishop));
    uint8_t nWhiteKnights = CountSetBits(board->GetBoard(Color::White, Piece::Knight));
    uint8_t nWhiteBishops = CountSetBits(board->GetBoard(Color::White, Piece::Bishop));
    uint8_t nBlackPieces = CountSetBits(board->GetBoard(Color::Black));
    uint8_t nWhitePieces = CountSetBits(board->GetBoard(Color::White));

    if(nBlackPieces == 2 && (nBlackKnights == 1 || nBlackBishops == 1) && nWhitePieces == 1) {
        board->SetState(State::InSufficientMaterial);
    } else if(nWhitePieces == 2 && (nWhiteBishops == 1 || nWhiteKnights == 1) && nBlackPieces == 1) {
        board->SetState(State::InSufficientMaterial);
    }
}

void Engine::GenerateLegalMoves(const std::unique_ptr<Board> &board) {
    fLegalMoves.clear();
    
    CheckFiftyMoveDraw(board);
    CheckInsufficientMaterial(board); 

    GeneratePseudoLegalMoves(board);
    GenerateCastlingMoves(board);
    GenerateEnPassantMoves(board);
    UpdatePromotionMoves();
    StripIllegalMoves(board);
    fLastUnique = board->GetUnique();
}

void Engine::UpdatePromotionMoves() {
    std::vector<U32> promoMoves = {};
    const std::vector<Piece> promoPieces = {Piece::Bishop, Piece::Knight, Piece::Rook, Piece::Queen};
    for(int iMove = 0; iMove < fLegalMoves.size(); iMove++) {
        U32 move = fLegalMoves[iMove];
        if(!GetMoveIsPromotion(move))
            continue;
        fLegalMoves.erase(std::begin(fLegalMoves) + iMove); // Remove the current promotion move with no defined promo piece
        // Add a unique move for all 4 promotion types (rook, knight, queen and bishop promotions)
        for (Piece promotionPiece : promoPieces) {
            U32 promotionMove = move;
            SetMovePromotionPiece(promotionMove, promotionPiece);
            promoMoves.push_back(promotionMove);
        }
        iMove--;
    }
    fLegalMoves.insert(fLegalMoves.end(), promoMoves.begin(), promoMoves.end());
}

void Engine::StripIllegalMoves(const std::unique_ptr<Board> &board) {
    // Check all the illegal moves, e.g. do they result in your own king being in check?
    const Color otherColor = board->GetColorToMove() == Color::White ? Color::Black : Color::White;
    const U64 underAttack = GetAttacks(board, otherColor);
    //std::cout << "\nPrinting whites attacks:\n";
    //PrintBitset(underAttack);
    const U64 king = board->GetBoard(board->GetColorToMove(), Piece::King); // The king of the colour about to move

    if(king & underAttack) // Player to move is in check, only moves resolving the check can be permitted
        PruneCheckMoves(board);

    std::vector<std::pair<U64, U64>> pinnedPieces; // Position of the pinned piece and all squares (including the attacking piece) on the pinning ray (as all moves on this ray of the pinned position are of course legal)
    for(Direction d : DIRECTIONS) {
        AddAbolsutePins(board, &pinnedPieces, d);
    }
    const U64 pinnedPositions = std::accumulate(
        pinnedPieces.begin(), pinnedPieces.end(), U64(0),
        [](U64 acc, const std::pair<U64, U64>& p) {
            return acc | p.first;
        }
    );
    
    for(int iMove = 0; iMove < fLegalMoves.size(); iMove++) {
        U32 m = fLegalMoves[iMove];
        // King cant move to squares the opponent attacks
        if((GetMovePiece(m) == Piece::King) && (GetMoveTarget(m) & underAttack)) {
            fLegalMoves.erase(std::begin(fLegalMoves) + iMove);
            iMove--;
        // Absolutely pinned pieces may not move, unless it is a capture of that piece or along pinning ray
        } else if(pinnedPositions & GetMoveOrigin(m)) { // Piece originates from a pinned position
            for(auto pins : pinnedPieces) {
                // !Piece moving from pinned position to somewhere on the associated pinning ray (incl capture)
                if((GetMoveOrigin(m) & pins.first) && (GetMoveTarget(m) & ~pins.second)) {
                    // Moving to somewhere off the absolutely pinning ray (illegal)
                    fLegalMoves.erase(std::begin(fLegalMoves) + iMove);
                    iMove--;
                }
            }
        } else if(GetMoveIsEnPassant(m)) { // Need to be manually checked due to rook rays
            board->MakeMove(m);
            Color otherColor = board->GetColorToMove() == Color::White ? Color::Black : Color::White;
            if(GetAttacks(board, board->GetColorToMove()) & board->GetBoard(otherColor, Piece::King)) {
                fLegalMoves.erase(std::begin(fLegalMoves) + iMove);
                iMove--;
            }
            board->UndoMove();
        }
    }
}

void Engine::PruneCheckMoves(const std::unique_ptr<Board> &board) {
    // TODO: Make this quicker i.e. don't make/undo every move as that is slow
    for(int iMove = 0; iMove < fLegalMoves.size(); iMove++) {
        U32 move = fLegalMoves[iMove];
        if(GetMoveIsCastling(move)) {
            fLegalMoves.erase(std::begin(fLegalMoves) + iMove);
            iMove--;
        } else {
            board->MakeMove(move);
            U64 underAttack = GetAttacks(board, board->GetColorToMove());
            U64 king = board->GetBoard(board->GetColorToMove() == Color::White ? Color::Black : Color::White, Piece::King);
            if(underAttack & king) { // Move was illegal so remove it
                fLegalMoves.erase(std::begin(fLegalMoves) + iMove);
                iMove--;
            }
            board->UndoMove();
        }
    }
}

void Engine::AddAbolsutePins(const std::unique_ptr<Board> &board, std::vector<std::pair<U64, U64>> *v, Direction d) {
    // Make artificial occupancy to block in the king and only get the north ray
    const U64 king = board->GetBoard(board->GetColorToMove(), Piece::King);
    const uint8_t lsb = get_LSB(king);
    Color otherColor = board->GetColorToMove() == Color::White ? Color::Black : Color::White;
    U64 rayOccupancy = board->GetBoard(otherColor);
    const U64 defendingRook = board->GetBoard(otherColor, Piece::Rook);
    const U64 defendingBishop = board->GetBoard(otherColor, Piece::Bishop);
    const U64 defendingQueen = board->GetBoard(otherColor, Piece::Queen);
    const U64 ownPieces = board->GetBoard(board->GetColorToMove());
    U64 rayMask = 0;
    U64 enemies = defendingQueen;
    U64 kingShift = 0;

    switch(d) {
        case Direction::North:
            kingShift |= south(king);
            rayMask = fSecondaryStraightAttacks[lsb];
            enemies |= defendingRook;
            break;
        case Direction::East:
            kingShift |= west(king);
            rayMask = fPrimaryStraightAttacks[lsb];
            enemies |= defendingRook;
            break;
        case Direction::West:
            kingShift |= east(king);
            rayMask = fPrimaryStraightAttacks[lsb];
            enemies |= defendingRook;
            break;
        case Direction::South:
            kingShift |= north(king);
            rayMask = fSecondaryStraightAttacks[lsb];
            enemies |= defendingRook;
            break;
        case Direction::NorthEast:
            kingShift |= south_west(king);
            rayMask = fSecondaryDiagonalAttacks[lsb];
            enemies |= defendingBishop;
            break;
        case Direction::NorthWest:
            kingShift |= south_east(king);
            rayMask = fPrimaryDiagonalAttacks[lsb];
            enemies |= defendingBishop;
            break;
        case Direction::SouthEast:
            kingShift |= north_west(king);
            rayMask = fPrimaryDiagonalAttacks[lsb];
            enemies |= defendingBishop;
            break;
        case Direction::SouthWest:
            kingShift |= north_east(king);
            rayMask = fSecondaryDiagonalAttacks[lsb];
            enemies |= defendingBishop;
            break;
        default:
            return;
            break;
    }

    rayOccupancy |= kingShift;
    U64 ray = hypQuint(king, rayOccupancy, rayMask) ^ kingShift;
    U64 rayAndEnemy = ray & enemies;

    if(rayAndEnemy) { // Enemy piece on the ray pointing at the king (that could attack king, if not blocked)
        // Note that rayAndEnemy is actually the position of the attacking piece on the ray
        U64 potentialPin = ray & ownPieces; // All your pieces that exist on the ray (between king and attacking piece)
        if(CountSetBits(potentialPin) == 1) { // A single piece is on the ray and so absolutely pinned
            v->push_back(std::make_pair(potentialPin, ray));
        } else {
            // None of your pieces in the way so the king is in check from attacker
        }
    }
}

U64 Engine::GetAttacks(const std::unique_ptr<Board> &board, const Color attackingColor) {
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
        uint8_t lsb = pop_LSB(bishops);
        set_bit(bishop, lsb);
        attacks |= (hypQuint(bishop, occ, fPrimaryDiagonalAttacks[lsb]) | hypQuint(bishop, occ, fSecondaryDiagonalAttacks[lsb]));
    }

    // Rooks
    U64 rooks = board->GetBoard(attackingColor, Piece::Rook);
    while(rooks) {
        U64 rook = 0;
        uint8_t lsb = pop_LSB(rooks);
        set_bit(rook, lsb);
        attacks |= (hypQuint(rook, occ, fPrimaryStraightAttacks[lsb]) | hypQuint(rook, occ, fSecondaryStraightAttacks[lsb]));
    }

    // Queens
    U64 queens = board->GetBoard(attackingColor, Piece::Queen);
    while(queens) {
        U64 queen = 0;
        uint8_t lsb = pop_LSB(queens);
        set_bit(queen, lsb);
        attacks |= (hypQuint(queen, occ, fPrimaryStraightAttacks[lsb]) | hypQuint(queen, occ, fSecondaryStraightAttacks[lsb]) | hypQuint(queen, occ, fPrimaryDiagonalAttacks[lsb]) | hypQuint(queen, occ, fSecondaryDiagonalAttacks[lsb]));
    }

    // King
    U64 king = board->GetBoard(attackingColor, Piece::King);
    attacks |= fKingAttacks[get_LSB(king)];
    return attacks;
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
    // En-passant not possible so throw away early
    if((!board->GetWasLoadedFromFEN() && board->GetNHalfMoves() < MIN_MOVES_FOR_ENPASSANT) || 
        board->GetWasLoadedFromFEN() && board->GetNHalfMoves() < 1)
        return;

    // FEN loaded position with en-passant move immediately available
    if(board->GetWasLoadedFromFEN() && board->GetEnPassantFEN()) {
        U64 attackSquares = 0;
        U64 target = board->GetEnPassantFEN();
        if(board->GetColorToMove() == Color::White) {
            attackSquares = (south_east(target) | south_west(target)) & board->GetBoard(Color::White, Piece::Pawn);
        } else {
            attackSquares = (north_east(target) | north_west(target)) & board->GetBoard(Color::Black, Piece::Pawn);
        }

        while(attackSquares) {
            U64 pawn = 0;
            set_bit(pawn, pop_LSB(attackSquares));
            U32 move = 0;
            SetMove(
                move, 
                pawn, 
                target, 
                Piece::Pawn, 
                Piece::Pawn
            );
            SetMoveIsEnPassant(move, true);
            fLegalMoves.push_back(move);
        }        
    }

    // Now run the usual code
    U32 lastMove = board->GetLastMove();

    // Faster return if you know en-passant will not be possible
    if(GetMovePiece(lastMove) != Piece::Pawn || GetMoveIsEnPassant(lastMove) || GetMoveIsCastling(lastMove))
        return;

    U64 pawns = board->GetBoard(board->GetColorToMove(), Piece::Pawn);
    U64 enPassantPawns = 0;

    if(board->GetColorToMove() == Color::White && (GetMoveOrigin(lastMove) & RANK_7) && (GetMoveTarget(lastMove) & RANK_5)) {
        // Was a double-move forward with a pawn by black last turn
        // Check if the move placed the pawn on an adjacent file to any of your pawns on rank 5
        enPassantPawns = (east(GetMoveTarget(lastMove)) | west(GetMoveTarget(lastMove))) & pawns;
    } else if(board->GetColorToMove() == Color::Black && (GetMoveOrigin(lastMove) & RANK_2) && (GetMoveTarget(lastMove) & RANK_4)) {
        // Was a double-move forward with a pawn by white last turn
        enPassantPawns = (east(GetMoveTarget(lastMove)) | west(GetMoveTarget(lastMove))) & pawns;
    }
    while(enPassantPawns) {
        U64 pawn = 0;
        set_bit(pawn, pop_LSB(enPassantPawns));
        U32 move = 0;
        SetMove(
            move, 
            pawn, 
            board->GetColorToMove() == Color::White ? north(GetMoveTarget(lastMove)) : south(GetMoveTarget(lastMove)), Piece::Pawn, 
            Piece::Pawn
        );
        SetMoveIsEnPassant(move, true);
        fLegalMoves.push_back(move);
    }
}

void Engine::GenerateCastlingMoves(const std::unique_ptr<Board> &board) {
    if(board->GetNHalfMoves() < MIN_MOVES_FOR_CASTLING && !board->GetWasLoadedFromFEN())
        return;

    U64 occupancy = board->GetOccupancy();
    U64 origin = board->GetBoard(board->GetColorToMove(), Piece::King);

    // Castling conditions for white
    U32 move = 0;
    if(board->GetColorToMove() == Color::White && !board->GetWhiteKingMoved()) {
        if(!board->GetWhiteKingsideRookMoved() && 
            IsCastlingPossible(KING_SIDE_CASTLING_MASK_WHITE, KING_SIDE_CASTLING_OCCUPANCY_MASK_WHITE, board)) 
        {
            SetMove(move, origin, RANK_1 & FILE_G, Piece::King, Piece::Null);
            SetMoveIsCastling(move, true);
            fLegalMoves.push_back(move);
            move = 0;
        }
        if(!board->GetWhiteQueensideRookMoved() &&
            IsCastlingPossible(QUEEN_SIDE_CASTLING_MASK_WHITE, QUEEN_SIDE_CASTLING_OCCUPANCY_MASK_WHITE, board)) {
            SetMove(move, origin, RANK_1 & FILE_C, Piece::King, Piece::Null);
            SetMoveIsCastling(move, true);
            fLegalMoves.push_back(move);
            move = 0;
        }
    // Castling conditions for black
    } else if(board->GetColorToMove() == Color::Black && !board->GetBlackKingMoved()) {
        if (!board->GetBlackKingsideRookMoved() &&
            IsCastlingPossible(KING_SIDE_CASTLING_MASK_BLACK, KING_SIDE_CASTLING_OCCUPANCY_MASK_BLACK, board)) 
        {
            SetMove(move, origin, RANK_8 & FILE_G, Piece::King, Piece::Null);
            SetMoveIsCastling(move, true);
            fLegalMoves.push_back(move);
            move = 0;
        }
        if (!board->GetBlackQueensideRookMoved() &&
            IsCastlingPossible(QUEEN_SIDE_CASTLING_MASK_BLACK, QUEEN_SIDE_CASTLING_OCCUPANCY_MASK_BLACK, board)) 
        {
            SetMove(move, origin, RANK_8 & FILE_C, Piece::King, Piece::Null);
            SetMoveIsCastling(move, true);
            fLegalMoves.push_back(move);
            move = 0;
        }
    }
}

bool Engine::IsCastlingPossible(U64 castlingMask, U64 occupancyMask, const std::unique_ptr<Board> &board) {
    U64 occ = board->GetOccupancy();
    return !(occ & occupancyMask) && !IsUnderAttack(castlingMask, board->GetColorToMove() == Color::White ? Color::Black : Color::White, board);
}

bool Engine::IsUnderAttack(U64 mask, Color attackingColor, const std::unique_ptr<Board> &board) {    
    U64 attacker = board->GetBoard(attackingColor);
    U64 attacks = GetAttacks(board, attackingColor);
    return attacks & mask & ~attacker;
}

void Engine::GeneratePawnPseudoLegalMoves(const std::unique_ptr<Board> &board) {
    U64 pawns = board->GetBoard(board->GetColorToMove(), Piece::Pawn);
    U64 enemy = board->GetBoard(board->GetColorToMove() == Color::White ? Color::Black : Color::White);
    U64 occ = board->GetOccupancy();
    U64 promotionRank = board->GetColorToMove() == Color::White ? RANK_8 : RANK_1;
    U64 startRank = board->GetColorToMove() == Color::White ? RANK_2 : RANK_7;
    while(pawns) {
        U64 pawn = 0;
        uint8_t lsb = pop_LSB(pawns);
        set_bit(pawn, lsb);

        // Only allow diagonal attacks if occupied by enemy piece
        U64 attacks = (board->GetColorToMove() == Color::White ? fWhitePawnDiagonalAttacks[lsb] : fBlackPawnDiagonalAttacks[lsb]) & enemy;

        // Get rid of 2-square attack if 1st or 2nd square is occupied, add single square attacks
        U64 oneSquareForward = (board->GetColorToMove() == Color::White ? north(pawn) : south(pawn)) & ~occ;
        attacks |= oneSquareForward;
        if(oneSquareForward && (pawn & startRank))
            attacks |= (board->GetColorToMove() == Color::White ? north(north(pawn)) : south(south(pawn))) & ~occ;
        while(attacks) {
            U64 attack = 0;
            set_bit(attack, pop_LSB(attacks));
            U32 move = 0;
            SetMove(move, pawn, attack, Piece::Pawn, board->GetIsOccupied(attack).second);
            if(attack & promotionRank)
                SetMoveIsPromotion(move, true);
            fLegalMoves.push_back(move);
        }
    }
}

void Engine::GenerateKingPseudoLegalMoves(const std::unique_ptr<Board> &board) {
    U64 king = board->GetBoard(board->GetColorToMove(), Piece::King);
    U64 attacks = fKingAttacks[get_LSB(king)] & ~board->GetBoard(board->GetColorToMove());
    while(attacks) {
        U64 attack = 0;
            set_bit(attack, pop_LSB(attacks));
            U32 move = 0;
            SetMove(move, king, attack, Piece::King, board->GetIsOccupied(attack).second);
            fLegalMoves.push_back(move);
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
            U32 move = 0;
            SetMove(move, knight, attack, Piece::Knight, board->GetIsOccupied(attack).second);
            fLegalMoves.push_back(move);
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
            U32 move = 0;
            SetMove(move, bishop, attack, Piece::Bishop, board->GetIsOccupied(attack).second);
            fLegalMoves.push_back(move);
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
        U64 attacks = (hypQuint(rook, occ, fPrimaryStraightAttacks[lsb]) | hypQuint(rook, occ, fSecondaryStraightAttacks[lsb])) & ~you;
        while(attacks) {
            U64 attack = 0;
            set_bit(attack, pop_LSB(attacks));
            U32 move = 0;
            SetMove(move, rook, attack, Piece::Rook, board->GetIsOccupied(attack).second);
            fLegalMoves.push_back(move);
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
            U32 move = 0;
            SetMove(move, queen, attack, Piece::Queen, board->GetIsOccupied(attack).second);
            fLegalMoves.push_back(move);
        }
    }                
}

bool Engine::GetMoveIsLegal(U32* move) {
    for(int iMove = 0; iMove < fLegalMoves.size(); iMove++) {
        U32 legalMove = fLegalMoves[iMove];
        if((GetMoveOrigin(legalMove) & GetMoveOrigin(*move)) && (GetMoveTarget(legalMove) & GetMoveTarget(*move))) {
            *move = legalMove;
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

U32 Engine::GetBestMove(Board board) {
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
   return U32(0);
}

int Engine::GetNLegalMoves(const std::unique_ptr<Board> &board) {
    return fLegalMoves.size();
};