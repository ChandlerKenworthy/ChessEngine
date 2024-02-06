#include <iostream>
#include <fstream>
#include <string>

#include "Engine.hpp"

Engine::Engine(const bool init) {
    fMaxDepth = 4;
    fLastUnique = -1;
    if(init) 
        Prepare();

    fColor = Color::White;
    fOtherColor = Color::Black;
    fOccupancy = 0;
    fActiveKing = 0;
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

bool Engine::CheckFiftyMoveDraw(const std::unique_ptr<Board> &board) {
    if(board->GetHalfMoveClock() == 100) {
        board->SetState(State::FiftyMoveRule);
        return true;
    }   
    return false;
}

bool Engine::CheckInsufficientMaterial(const std::unique_ptr<Board> &board) {
    uint8_t nBlackKnights = CountSetBits(board->GetBoard(Color::Black, Piece::Knight));
    uint8_t nBlackBishops = CountSetBits(board->GetBoard(Color::Black, Piece::Bishop));
    uint8_t nWhiteKnights = CountSetBits(board->GetBoard(Color::White, Piece::Knight));
    uint8_t nWhiteBishops = CountSetBits(board->GetBoard(Color::White, Piece::Bishop));
    uint8_t nBlackPieces = CountSetBits(board->GetBoard(Color::Black));
    uint8_t nWhitePieces = CountSetBits(board->GetBoard(Color::White));

    if(nBlackPieces == 2 && (nBlackKnights == 1 || nBlackBishops == 1) && nWhitePieces == 1) {
        board->SetState(State::InSufficientMaterial);
        return true;
    } else if(nWhitePieces == 2 && (nWhiteBishops == 1 || nWhiteKnights == 1) && nBlackPieces == 1) {
        board->SetState(State::InSufficientMaterial);
        return true;
    }
    return false;
}

std::vector<U32> Engine::GenerateLegalMoves(const std::unique_ptr<Board> &board, bool /*returnMoves*/) {
    std::vector<U32> moves{};

    if(CheckFiftyMoveDraw(board))
        return moves;
    if(CheckInsufficientMaterial(board))
        return moves;

    // Don't use any class level variables we don't want them to change
    const Color movingColor = board->GetColorToMove();
    const Color otherColor = movingColor == Color::White ? Color::Black : Color::White;
    const U64 occ = board->GetOccupancy();
    const U64 movingKing = board->GetBoard(movingColor, Piece::King);

    GeneratePseudoLegalMoves(board, moves, movingColor, otherColor, occ, movingKing);
    GenerateCastlingMoves(board, moves, movingColor, otherColor, movingKing, occ);
    GenerateEnPassantMoves(board, moves, movingColor);
    UpdatePromotionMoves(moves);
    StripIllegalMoves(board, moves, otherColor, movingColor, movingKing);

    // For some reason this seems to break the perft tests
    if(moves.size() == 0) { // No legal moves, game is either stalemate or checkmate
        bool kingInCheck = IsUnderAttack(movingKing, otherColor, board);
        if(kingInCheck) {
            board->SetState(State::Checkmate);
        } else {
            board->SetState(State::Stalemate);
        }
    }
    return moves;
}

void Engine::GenerateLegalMoves(const std::unique_ptr<Board> &board) {
    fLegalMoves.clear();
    fLegalMoves = GenerateLegalMoves(board, true);
}

void Engine::UpdatePromotionMoves(std::vector<U32> &moves) {
    std::vector<U32> promoMoves;
    const std::vector<Piece> promoPieces = {Piece::Bishop, Piece::Knight, Piece::Rook, Piece::Queen};
    for (auto it = moves.begin(); it != moves.end(); ) {
        U32 move = *it;
        if (!GetMoveIsPromotion(move)) {
            ++it;
            continue;
        }
        // Use the erase-remove idiom to efficiently remove elements
        it = moves.erase(it);
        // Add a unique move for all 4 promotion types
        for (Piece promotionPiece : promoPieces) {
            U32 promotionMove = move;
            SetMovePromotionPiece(promotionMove, promotionPiece);
            promoMoves.push_back(promotionMove);
        }
    }
    // Append promoMoves to moves
    moves.insert(moves.end(), promoMoves.begin(), promoMoves.end());
}

void Engine::StripIllegalMoves(const std::unique_ptr<Board> &board, std::vector<U32> &moves, const Color otherColor, const Color activeColor, const U64 activeKing) {
    // Check all the illegal moves, e.g. do they result in your own king being in check?
    const U64 underAttack = GetAttacks(board, otherColor);

    if(activeKing & underAttack) // Player to move is in check, only moves resolving the check can be permitted
        PruneCheckMoves(board, moves, activeKing, activeColor);

    std::vector<std::pair<U64, U64>> pinnedPieces; // Position of the pinned piece and all squares (including the attacking piece) on the pinning ray (as all moves on this ray of the pinned position are of course legal)
    for(Direction d : DIRECTIONS) {
        AddAbolsutePins(board, &pinnedPieces, d, activeKing, activeColor, otherColor);
    }
    const U64 pinnedPositions = std::accumulate(
        pinnedPieces.begin(), pinnedPieces.end(), U64(0),
        [](U64 acc, const std::pair<U64, U64>& p) {
            return acc | p.first;
        }
    );

    for(int iMove = 0; iMove < (int)moves.size(); iMove++) {
        U32 m = moves[iMove];
        const U64 moveOrigin = GetMoveOrigin(m);
        // King cant move to squares the opponent attacks
        if((GetMovePiece(m) == Piece::King) && (GetMoveTarget(m) & underAttack)) {
            moves.erase(std::begin(moves) + iMove);
            iMove--;
        // Absolutely pinned pieces may not move, unless it is a capture of that piece or along pinning ray
        } else if(pinnedPositions & moveOrigin) { // Piece originates from a pinned position
            for(const std::pair<U64, U64> &pins : pinnedPieces) {
                // !Piece moving from pinned position to somewhere on the associated pinning ray (incl capture)
                if((moveOrigin & pins.first) && (GetMoveTarget(m) & ~pins.second)) {
                    // Moving to somewhere off the absolutely pinning ray (illegal)
                    moves.erase(std::begin(moves) + iMove);
                    iMove--;
                }
            }
        } else if(GetMoveIsEnPassant(m)) { // Need to be manually checked due to rook rays
            U64 activeRank = get_rank(moveOrigin);
            U64 kingOnRank = activeKing & activeRank; // King shares the rank with the en-passanting pawn
            U64 rookOnRank = activeRank & (board->GetBoard(otherColor, Piece::Rook) | board->GetBoard(otherColor, Piece::Queen)); // Rook or queen have sliding straight attacks

            if(kingOnRank && rookOnRank) { // King, two pawns and rook/queen(s) occupy the same rank
                // Is en-passant so the captured pawn occupies the same rank as well, if we remove our pawn and the
                // captured pawn does this leave our king on a rank with just itself and the attacking rook/queen
                U64 occupancy = board->GetOccupancy();

                // MSB is the left-most bit (i.e. that with lowest index)
                int attackingRookBit = get_MSB(activeKing) < get_MSB(rookOnRank) ? 63 - get_MSB(rookOnRank) : get_LSB(rookOnRank);
                U64 rook = 0; // Again could be a queen but it doesn't matter, this is the one checking king if move happens
                set_bit(rook, attackingRookBit);
                U64 rookShift = get_MSB(activeKing) < get_MSB(rookOnRank) ? east(rook) : west(rook);
                U64 takenPawn = activeColor == Color::White ? south(GetMoveTarget(m)) : north(GetMoveTarget(m));

                // Make a custom occupancy mask to cut the rook ray down
                U64 mask = activeKing | rook | rookShift;
                U64 rookRay = hypQuint(rook, mask, fPrimaryStraightAttacks[attackingRookBit]);

                // Subtract from the ray our pawns that we know intersect the ray
                rookRay ^= (moveOrigin | takenPawn | activeKing | rookShift);

                if(!(rookRay & occupancy)) { // No pieces on the ray so if en-passant happens king will be in check
                    moves.erase(std::begin(moves) + iMove);
                    iMove--;
                }
            }
        }
    }
}

void Engine::PruneCheckMoves(const std::unique_ptr<Board> &board, std::vector<U32> &moves, const U64 activeKing, const Color activeColor) {
    std::vector<U32> validMoves;

    for (U32 move : moves) {
        if (GetMoveIsCastling(move)) {
            continue; // Skip castling moves
        }

        board->MakeMove(move);
        U64 underAttack = GetAttacks(board, activeColor == Color::White ? Color::Black : Color::White);
        // King may have moved so can't use fActiveKing
        U64 newKing = GetMovePiece(move) == Piece::King ? board->GetBoard(activeColor, Piece::King) : activeKing;
        if(!(underAttack & newKing)) {
            validMoves.push_back(move); // Move is legal, add it to the vector
        }
        board->UndoMove();
    }
    // Replace fLegalMoves with validMoves
    moves = std::move(validMoves);
}


void Engine::AddAbolsutePins(const std::unique_ptr<Board> &board, std::vector<std::pair<U64, U64>> *v, Direction d, const U64 activeKing, const Color activeColor, const Color otherColor) {
    // Make artificial occupancy to block in the king and only get the north ray
    const uint8_t lsb = get_LSB(activeKing);
    U64 rayOccupancy = board->GetBoard(otherColor);
    const U64 defendingRook = board->GetBoard(otherColor, Piece::Rook);
    const U64 defendingBishop = board->GetBoard(otherColor, Piece::Bishop);
    const U64 defendingQueen = board->GetBoard(otherColor, Piece::Queen);
    const U64 ownPieces = board->GetBoard(activeColor);
    U64 rayMask = 0;
    U64 enemies = defendingQueen;
    U64 kingShift = 0;

    switch(d) {
        case Direction::North:
            kingShift |= south(activeKing);
            rayMask = fSecondaryStraightAttacks[lsb];
            enemies |= defendingRook;
            break;
        case Direction::East:
            kingShift |= west(activeKing);
            rayMask = fPrimaryStraightAttacks[lsb];
            enemies |= defendingRook;
            break;
        case Direction::West:
            kingShift |= east(activeKing);
            rayMask = fPrimaryStraightAttacks[lsb];
            enemies |= defendingRook;
            break;
        case Direction::South:
            kingShift |= north(activeKing);
            rayMask = fSecondaryStraightAttacks[lsb];
            enemies |= defendingRook;
            break;
        case Direction::NorthEast:
            kingShift |= south_west(activeKing);
            rayMask = fSecondaryDiagonalAttacks[lsb];
            enemies |= defendingBishop;
            break;
        case Direction::NorthWest:
            kingShift |= south_east(activeKing);
            rayMask = fPrimaryDiagonalAttacks[lsb];
            enemies |= defendingBishop;
            break;
        case Direction::SouthEast:
            kingShift |= north_west(activeKing);
            rayMask = fPrimaryDiagonalAttacks[lsb];
            enemies |= defendingBishop;
            break;
        case Direction::SouthWest:
            kingShift |= north_east(activeKing);
            rayMask = fSecondaryDiagonalAttacks[lsb];
            enemies |= defendingBishop;
            break;
        default:
            return;
            break;
    }

    rayOccupancy |= kingShift;
    U64 ray = hypQuint(activeKing, rayOccupancy, rayMask) ^ kingShift;
    U64 rayAndEnemy = ray & enemies;

    if(rayAndEnemy) { // Enemy piece on the ray pointing at the king (that could attack king, if not blocked)
        // Note that rayAndEnemy is actually the position of the attacking piece on the ray
        U64 potentialPin = ray & ownPieces; // All your pieces that exist on the ray (between king and attacking piece)
        if(CountSetBits(potentialPin) == 1) { // A single piece is on the ray and so absolutely pinned
            v->push_back(std::make_pair(potentialPin, ray));
        } // else : None of your pieces in the way so the king is in check from attacker
    }
}

U64 Engine::GetAttacks(const std::unique_ptr<Board> &board, const Color attackingColor) {
    U64 attacks = 0;
    U64 occ = board->GetOccupancy(); // Cannot replace with fOccupancy

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
    return attacks; // Don't exclude your own pieces since they are protected so king cannot take them
}

void Engine::GeneratePseudoLegalMoves(const std::unique_ptr<Board> &board, std::vector<U32> &moves, const Color activeColor, const Color otherColor, const U64 occupancy, const U64 activeKing) {
    GeneratePawnPseudoLegalMoves(board, moves, activeColor, otherColor, occupancy);
    GenerateKingPseudoLegalMoves(board, moves, activeColor, otherColor, activeKing);
    GenerateKnightPseudoLegalMoves(board, moves, activeColor, otherColor);
    GenerateBishopPseudoLegalMoves(board, moves, activeColor, otherColor, occupancy);
    GenerateRookPseudoLegalMoves(board, moves, activeColor, otherColor, occupancy);
    GenerateQueenPseudoLegalMoves(board, moves, activeColor, otherColor, occupancy);
}

void Engine::GenerateEnPassantMoves(const std::unique_ptr<Board> &board, std::vector<U32> &moves, const Color activeColor) {
    // En-passant not possible so throw away early
    if((!board->GetWasLoadedFromFEN() && board->GetNHalfMoves() < MIN_MOVES_FOR_ENPASSANT) || 
        (board->GetWasLoadedFromFEN() && board->GetNHalfMoves() < 1))
        return;

    // FEN loaded position with en-passant move immediately available
    if(board->GetWasLoadedFromFEN() && board->GetEnPassantFEN()) {
        U64 attackSquares = 0;
        U64 target = board->GetEnPassantFEN();
        if(activeColor == Color::White) {
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
            moves.push_back(move);
        }        
    }

    // Now run the usual code
    U32 lastMove = board->GetLastMove();
    U64 lastMoveTarget = GetMoveTarget(lastMove);

    // Faster return if you know en-passant will not be possible
    if(GetMovePiece(lastMove) != Piece::Pawn || GetMoveIsEnPassant(lastMove) || GetMoveIsCastling(lastMove))
        return;

    U64 pawns = board->GetBoard(activeColor, Piece::Pawn);
    U64 enPassantPawns = 0;

    if(activeColor == Color::White && (GetMoveOrigin(lastMove) & RANK_7) && (lastMoveTarget & RANK_5)) {
        // Was a double-move forward with a pawn by black last turn
        // Check if the move placed the pawn on an adjacent file to any of your pawns on rank 5
        enPassantPawns = (east(lastMoveTarget) | west(lastMoveTarget)) & pawns;
    } else if(activeColor == Color::Black && (GetMoveOrigin(lastMove) & RANK_2) && (lastMoveTarget & RANK_4)) {
        // Was a double-move forward with a pawn by white last turn
        enPassantPawns = (east(lastMoveTarget) | west(lastMoveTarget)) & pawns;
    }
    while(enPassantPawns) {
        U64 pawn = 0;
        set_bit(pawn, pop_LSB(enPassantPawns));
        U32 move = 0;
        SetMove(
            move, 
            pawn, 
            activeColor == Color::White ? north(lastMoveTarget) : south(lastMoveTarget), Piece::Pawn, 
            Piece::Pawn
        );
        SetMoveIsEnPassant(move, true);
        moves.push_back(move);
    }
}

void Engine::GenerateCastlingMoves(const std::unique_ptr<Board> &board, std::vector<U32> &moves, const Color activeColor, const Color otherColor, const U64 activeKing, const U64 occupancy) {
    if(board->GetNHalfMoves() < MIN_MOVES_FOR_CASTLING && !board->GetWasLoadedFromFEN())
        return;

    // Castling conditions for white
    U32 move = 0;
    if(activeColor == Color::White && !board->GetWhiteKingMoved()) {
        if(!board->GetWhiteKingsideRookMoved() && 
            IsCastlingPossible(KING_SIDE_CASTLING_MASK_WHITE, KING_SIDE_CASTLING_OCCUPANCY_MASK_WHITE, board, occupancy, otherColor)) 
        {
            SetMove(move, activeKing, SQUARE_G1, Piece::King, Piece::Null);
            SetMoveIsCastling(move, true);
            moves.push_back(move);
            move = 0;
        }
        if(!board->GetWhiteQueensideRookMoved() &&
            IsCastlingPossible(QUEEN_SIDE_CASTLING_MASK_WHITE, QUEEN_SIDE_CASTLING_OCCUPANCY_MASK_WHITE, board, occupancy, otherColor)) {
            SetMove(move, activeKing, SQUARE_C1, Piece::King, Piece::Null);
            SetMoveIsCastling(move, true);
            moves.push_back(move);
            move = 0;
        }
    // Castling conditions for black
    } else if(activeColor == Color::Black && !board->GetBlackKingMoved()) {
        if (!board->GetBlackKingsideRookMoved() &&
            IsCastlingPossible(KING_SIDE_CASTLING_MASK_BLACK, KING_SIDE_CASTLING_OCCUPANCY_MASK_BLACK, board, occupancy, otherColor)) 
        {
            SetMove(move, activeKing, SQUARE_G8, Piece::King, Piece::Null);
            SetMoveIsCastling(move, true);
            moves.push_back(move);
            move = 0;
        }
        if (!board->GetBlackQueensideRookMoved() &&
            IsCastlingPossible(QUEEN_SIDE_CASTLING_MASK_BLACK, QUEEN_SIDE_CASTLING_OCCUPANCY_MASK_BLACK, board, occupancy, otherColor)) 
        {
            SetMove(move, activeKing, SQUARE_C8, Piece::King, Piece::Null);
            SetMoveIsCastling(move, true);
            moves.push_back(move);
            move = 0;
        }
    }
}

bool Engine::IsCastlingPossible(const U64 castlingMask, const U64 occupancyMask, const std::unique_ptr<Board> &board, const U64 occupancy, const Color otherColor) {
    return !(occupancy & occupancyMask) && !IsUnderAttack(castlingMask, otherColor, board);
}

bool Engine::IsUnderAttack(const U64 mask, const Color attackingColor, const std::unique_ptr<Board> &board) {    
    const U64 attacker = board->GetBoard(attackingColor);
    const U64 attacks = GetAttacks(board, attackingColor);
    return attacks & mask & ~attacker;
}

void Engine::GeneratePawnPseudoLegalMoves(const std::unique_ptr<Board> &board, std::vector<U32> &moves, const Color activeColor, const Color otherColor, const U64 occupancy) {
    U64 pawns = board->GetBoard(activeColor, Piece::Pawn);
    U64 enemy = board->GetBoard(otherColor);
    U64 promotionRank = activeColor == Color::White ? RANK_8 : RANK_1;
    U64 startRank = activeColor == Color::White ? RANK_2 : RANK_7;
    while(pawns) {
        U64 pawn = 0;
        uint8_t lsb = pop_LSB(pawns);
        set_bit(pawn, lsb);

        // Only allow diagonal attacks if occupied by enemy piece
        U64 attacks = (activeColor == Color::White ? fWhitePawnDiagonalAttacks[lsb] : fBlackPawnDiagonalAttacks[lsb]) & enemy;

        // Get rid of 2-square attack if 1st or 2nd square is occupied, add single square attacks
        U64 oneSquareForward = (activeColor == Color::White ? north(pawn) : south(pawn)) & ~occupancy;
        attacks |= oneSquareForward;
        if(oneSquareForward && (pawn & startRank))
            attacks |= (activeColor == Color::White ? north(north(pawn)) : south(south(pawn))) & ~occupancy;
        while(attacks) {
            U64 attack = 0;
            set_bit(attack, pop_LSB(attacks));
            U32 move = 0;
            SetMove(move, pawn, attack, Piece::Pawn, board->GetIsOccupied(attack, otherColor).second);
            if(attack & promotionRank)
                SetMoveIsPromotion(move, true);
            moves.push_back(move);
        }
    }
}

void Engine::GenerateKingPseudoLegalMoves(const std::unique_ptr<Board> &board, std::vector<U32> &moves, const Color activeColor, const Color otherColor, const U64 activeKing) {
    U64 attacks = fKingAttacks[get_LSB(activeKing)] & ~board->GetBoard(activeColor);
    while(attacks) {
        U64 attack = 0;
            set_bit(attack, pop_LSB(attacks));
            U32 move = 0;
            SetMove(move, activeKing, attack, Piece::King, board->GetIsOccupied(attack, otherColor).second);
            moves.push_back(move);
    }
}

void Engine::GenerateKnightPseudoLegalMoves(const std::unique_ptr<Board> &board, std::vector<U32> &moves, const Color activeColor, const Color otherColor) {
    U64 knights = board->GetBoard(activeColor, Piece::Knight);
    while(knights) {
        U64 knight = 0;
        uint8_t lsb = pop_LSB(knights);
        set_bit(knight, lsb);
        U64 attacks = fKnightAttacks[lsb] & ~board->GetBoard(activeColor);
        while(attacks) {
            U64 attack = 0;
            set_bit(attack, pop_LSB(attacks));
            U32 move = 0;
            SetMove(move, knight, attack, Piece::Knight, board->GetIsOccupied(attack, otherColor).second);
            moves.push_back(move);
        }
    }
}

void Engine::GenerateBishopPseudoLegalMoves(const std::unique_ptr<Board> &board, std::vector<U32> &moves, const Color activeColor, const Color otherColor, const U64 occupancy) {
    U64 bishops = board->GetBoard(activeColor, Piece::Bishop);
    U64 you = board->GetBoard(activeColor);
    while(bishops) {
        U64 bishop = 0;
        uint8_t lsb = pop_LSB(bishops);
        set_bit(bishop, lsb);
        U64 attacks = (hypQuint(bishop, occupancy, fPrimaryDiagonalAttacks[lsb]) | hypQuint(bishop, occupancy, fSecondaryDiagonalAttacks[lsb])) & ~you;
        while(attacks) {
            U64 attack = 0;
            set_bit(attack, pop_LSB(attacks));
            U32 move = 0;
            SetMove(move, bishop, attack, Piece::Bishop, board->GetIsOccupied(attack, otherColor).second);
            moves.push_back(move);
        }
    }
}

void Engine::GenerateRookPseudoLegalMoves(const std::unique_ptr<Board> &board, std::vector<U32> &moves, const Color activeColor, const Color otherColor, const U64 occupancy) {
    U64 rooks = board->GetBoard(activeColor, Piece::Rook);
    U64 you = board->GetBoard(activeColor);
    while(rooks) {
        U64 rook = 0;
        uint8_t lsb = pop_LSB(rooks);
        set_bit(rook, lsb);
        U64 attacks = (hypQuint(rook, occupancy, fPrimaryStraightAttacks[lsb]) | hypQuint(rook, occupancy, fSecondaryStraightAttacks[lsb])) & ~you;
        while(attacks) {
            U64 attack = 0;
            set_bit(attack, pop_LSB(attacks));
            U32 move = 0;
            SetMove(move, rook, attack, Piece::Rook, board->GetIsOccupied(attack, otherColor).second);
            moves.push_back(move);
        }
    }
}

void Engine::GenerateQueenPseudoLegalMoves(const std::unique_ptr<Board> &board, std::vector<U32> &moves, const Color activeColor, const Color otherColor, const U64 occupancy) {
    U64 queens = board->GetBoard(activeColor, Piece::Queen); // Could have multiple due to promotion
    U64 you = board->GetBoard(activeColor);
    while(queens) {
        U64 queen = 0;
        uint8_t lsb = pop_LSB(queens);
        set_bit(queen, lsb);
        U64 attacks = (hypQuint(queen, occupancy, fPrimaryStraightAttacks[lsb]) | hypQuint(queen, occupancy, fSecondaryStraightAttacks[lsb]) | hypQuint(queen, occupancy, fPrimaryDiagonalAttacks[lsb]) | hypQuint(queen, occupancy, fSecondaryDiagonalAttacks[lsb])) & ~you;
        while(attacks) {
            U64 attack = 0;
            set_bit(attack, pop_LSB(attacks));
            U32 move = 0;
            SetMove(move, queen, attack, Piece::Queen, board->GetIsOccupied(attack, otherColor).second);
            moves.push_back(move);
        }
    }
}

bool Engine::GetMoveIsLegal(U32* move) {
    for(int iMove = 0; iMove < (int)fLegalMoves.size(); iMove++) {
        U32 legalMove = fLegalMoves[iMove];
        if((GetMoveOrigin(legalMove) & GetMoveOrigin(*move)) && (GetMoveTarget(legalMove) & GetMoveTarget(*move))) {
            *move = legalMove;
            return true;
        }
    }
    return false;
}

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
    auto start = std::chrono::high_resolution_clock::now();
    U32 bestMove{0};
    Color colorToMove = board->GetColorToMove();
    // If white is playing the worst eval is -999 (i.e. black completely winning)
    float bestEval = colorToMove == Color::White ? -9999. : 9999.;
    int nMovesSearched = 0;

    // Order the moves for faster searching
    OrderMoves(board, fLegalMoves);

    // For each of the moves we want to find the "best" evaluation
    for(U32 move : fLegalMoves) {
        board->MakeMove(move);
        std::pair<float, int> result = Minimax(board, fMaxDepth - 1, -99999., 99999.);
        board->UndoMove();
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
    //std::cout << "Time: " << duration.count() / 1000. << " seconds\n";
    //std::cout << "Evaluated: " << nMovesSearched << " positions\n";
    return bestMove;
}

int Engine::GetNLegalMoves() {
    return fLegalMoves.size();
};

U32 Engine::GetRandomMove() {
    std::random_device seeder;
    std::mt19937 engine(seeder());
    std::uniform_int_distribution<U32> dist2(0, GetNLegalMoves() - 1);

    return fLegalMoves[dist2(engine)];
}
