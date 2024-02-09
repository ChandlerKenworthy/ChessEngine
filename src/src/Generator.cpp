#include "Generator.hpp"

Generator::Generator() {
    GenerateAttackTables();
}

void Generator::GenerateAttackTables() {
    for(int iSquare = 0; iSquare < NSQUARES; ++iSquare) {
        U64 position = 1ULL << iSquare;
        FillKingAttackTable(position);
        FillKnightAttackTable(position);
        FillPawnAttackTable(position);
        FillStraightAttackTables(position);
        FillDiagonalAttackTables(position);
    }
}

void Generator::FillKingAttackTable(const U64 pos) {
    fKingAttacks[__builtin_ctzll(pos)] = north(pos) | east(pos) | west(pos) | south(pos) | north_east(pos) | north_west(pos) | south_east(pos) | south_west(pos);
}

void Generator::FillKnightAttackTable(const U64 pos) {
    fKnightAttacks[__builtin_ctzll(pos)] = north(north_east(pos)) | north(north_west(pos)) | south(south_east(pos)) | south(south_west(pos)) | east(north_east(pos)) | east(south_east(pos)) | west(north_west(pos)) | west(south_west(pos));
}

void Generator::FillPawnAttackTable(const U64 pos) {
    // Note this does not include special pawn moves like en-passant or promotion
    U64 attacks = 0;
    const U8 lsb = __builtin_ctzll(pos);
    // White pawn case (attacks in a northern direction)
    attacks |= north(pos);
    if(pos & RANK_2)
        attacks |= north(north(pos)); // Assumes nothing blocking the pawns path
    fWhitePawnForwardAttacks[lsb] = attacks;
    // Only valid when occupied by enemy piece as these are taking moves
    fWhitePawnDiagonalAttacks[lsb] = north_east(pos) | north_west(pos);

    attacks = 0; // Now case for the black pawn attacking in a southern direction
    attacks |= south(pos);
    if(pos & RANK_7)
        attacks |= south(south(pos));
    fBlackPawnForwardAttacks[lsb] = attacks;
    fBlackPawnDiagonalAttacks[lsb] = south_east(pos) | south_west(pos);
}

void Generator::FillStraightAttackTables(const U64 pos) {
    U8 lsb = __builtin_ctzll(pos);
    fPrimaryStraightAttacks[lsb] = get_rank(pos) ^ pos;
    fSecondaryStraightAttacks[lsb] = get_file(pos) ^ pos;
}

void Generator::FillDiagonalAttackTables(const U64 pos) {
    // Vertical distance from primary diagonal is just abs(x - y) (zero indexed)
    // negative value = shift primary DOWN, positive value = shift primary UP
    const U8 fileNumber = get_file_number(pos);
    const U8 rankNumber = get_rank_number(pos);
    const U8 lsb = __builtin_ctzll(pos);

    const int dPrimaryDiag = (fileNumber - 1) - (8 - rankNumber); // Vertical distance from the primary diagonal
    const int dSecondaryDiag = (8 - fileNumber) - (8 - rankNumber); // Vertical distance from the secondary diagonal

    U64 primaryAttacks = dPrimaryDiag > 0 ? PRIMARY_DIAGONAL << (abs(dPrimaryDiag) * 8) : PRIMARY_DIAGONAL >> (abs(dPrimaryDiag) * 8);
    U64 secondaryAttacks = dSecondaryDiag > 0 ? SECONDARY_DIAGONAL << (abs(dSecondaryDiag) * 8) : SECONDARY_DIAGONAL >> (abs(dSecondaryDiag) * 8);

    fPrimaryDiagonalAttacks[lsb] = primaryAttacks ^ pos;
    fSecondaryDiagonalAttacks[lsb] = secondaryAttacks ^ pos;
}

void Generator::GenerateLegalMoves(const std::unique_ptr<Board> &board) {
    // TODO: Make me multi-threaded?
    if(CheckFiftyMoveDraw(board))
        return;
    if(CheckInsufficientMaterial(board))
        return;

    fColor = board->GetColorToMove();
    fOtherColor = fColor == Color::White ? Color::Black : Color::White;
    fOccupancy = board->GetOccupancy();
    fKing = board->GetBoard(fColor, Piece::King);
    fLegalMoves.reserve(AVERAGE_MOVES_PER_POSITION); // Avoid excess dynamic memory allocation

    GeneratePseudoLegalMoves(board);
    GenerateCastlingMoves(board);

    // GenerateCastlingMoves(board, moves, movingColor, otherColor, movingKing, occ);
    // GenerateEnPassantMoves(board, moves, movingColor);
    // UpdatePromotionMoves(moves);
    // StripIllegalMoves(board, moves, otherColor, movingColor, movingKing);

    //if(moves.size() == 0) { // No legal moves, game is either stalemate or checkmate
    //    bool kingInCheck = IsUnderAttack(movingKing, otherColor, board);
    //    if(kingInCheck) {
    //        board->SetState(State::Checkmate);
    //    } else {
    //        board->SetState(State::Stalemate);
    //    }
    //}
}

bool Generator::CheckFiftyMoveDraw(const std::unique_ptr<Board> &board) {
    if(board->GetHalfMoveClock() == 100) {
        board->SetState(State::FiftyMoveRule);
        return true;
    }   
    return false;
}

bool Generator::CheckInsufficientMaterial(const std::unique_ptr<Board> &board) {
    const U8 nBlackPieces = CountSetBits(board->GetBoard(Color::Black));
    const U8 nWhitePieces = CountSetBits(board->GetBoard(Color::White));
    if(nBlackPieces > 2 || nWhitePieces > 2)
        return false;
    
    const U8 nBlackKnights = CountSetBits(board->GetBoard(Color::Black, Piece::Knight));
    const U8 nBlackBishops = CountSetBits(board->GetBoard(Color::Black, Piece::Bishop));
    const U8 nWhiteKnights = CountSetBits(board->GetBoard(Color::White, Piece::Knight));
    const U8 nWhiteBishops = CountSetBits(board->GetBoard(Color::White, Piece::Bishop));

    if(nBlackPieces == 2 && (nBlackKnights == 1 || nBlackBishops == 1) && nWhitePieces == 1) {
        board->SetState(State::InSufficientMaterial);
        return true;
    } else if(nWhitePieces == 2 && (nWhiteBishops == 1 || nWhiteKnights == 1) && nBlackPieces == 1) {
        board->SetState(State::InSufficientMaterial);
        return true;
    }
    return false;
}

void Generator::GeneratePseudoLegalMoves(const std::unique_ptr<Board> &board) {
    GeneratePawnPseudoLegalMoves(board);
    GenerateKingPseudoLegalMoves(board);
    GenerateKnightPseudoLegalMoves(board);
    GenerateBishopPseudoLegalMoves(board);
    GenerateRookPseudoLegalMoves(board);
    GenerateQueenPseudoLegalMoves(board);

    // Erase all moves moving onto a square of its own colour
    const U64 selfOccupancy = board->GetBoard(fColor);
    fLegalMoves.erase(std::remove_if(fLegalMoves.begin(), fLegalMoves.end(), [&](U32 move) {
        return GetMoveTarget(move) & selfOccupancy;
    }), fLegalMoves.end());
}

void Generator::GenerateKingPseudoLegalMoves(const std::unique_ptr<Board> &board) {
    U64 attacks = fKingAttacks[__builtin_ctzll(fKing)];
    while(attacks) {
        const U64 attack = 1ULL << __builtin_ctzll(attacks);
        U32 move = 0;
        SetMove(move, fKing, attack, Piece::King, board->GetIsOccupied(attack, fOtherColor).second);
        fLegalMoves.push_back(move);
        attacks &= attacks - 1; // Clear the lowest set bit
    }
}

void Generator::GenerateKnightPseudoLegalMoves(const std::unique_ptr<Board> &board) {
    U64 knights = board->GetBoard(fColor, Piece::Knight);
    while(knights) {
        const U8 lsb = __builtin_ctzll(knights);
        const U64 knight = 1ULL << lsb;
        U64 attacks = fKnightAttacks[lsb];
        while(attacks) {
            const U64 attack = 1ULL << __builtin_ctzll(attacks);
            U32 move = 0;
            SetMove(move, knight, attack, Piece::Knight, board->GetIsOccupied(attack, fOtherColor).second);
            fLegalMoves.push_back(move);
            attacks &= attacks - 1; // Clear the lowest set bit
        }
        knights &= knights - 1;
    }
}

void Generator::GenerateRookPseudoLegalMoves(const std::unique_ptr<Board> &board) {
    U64 rooks = board->GetBoard(fColor, Piece::Rook);
    while(rooks) {
        const U8 lsb = __builtin_ctzll(rooks);
        const U64 rook = 1ULL << lsb;
        U64 attacks = (hypQuint(rook, fOccupancy, fPrimaryStraightAttacks[lsb]) | hypQuint(rook, fOccupancy, fSecondaryStraightAttacks[lsb]));
        while(attacks) {
            U64 attack = 1ULL << __builtin_ctzll(attacks);
            U32 move = 0;
            SetMove(move, rook, attack, Piece::Rook, board->GetIsOccupied(attack, fOtherColor).second);
            fLegalMoves.push_back(move);
            attacks &= attacks - 1;
        }
        rooks &= rooks - 1;
    }
}

void Generator::GenerateBishopPseudoLegalMoves(const std::unique_ptr<Board> &board) {
    U64 bishops = board->GetBoard(fColor, Piece::Bishop);
    while(bishops) {
        const U8 lsb = __builtin_ctzll(bishops);
        const U64 bishop = 1ULL << lsb;
        U64 attacks = (hypQuint(bishop, fOccupancy, fPrimaryDiagonalAttacks[lsb]) | hypQuint(bishop, fOccupancy, fSecondaryDiagonalAttacks[lsb]));
        while(attacks) {
            U64 attack = 1ULL << __builtin_ctzll(attacks);
            U32 move = 0;
            SetMove(move, bishop, attack, Piece::Bishop, board->GetIsOccupied(attack, fOtherColor).second);
            fLegalMoves.push_back(move);
            attacks &= attacks - 1;
        }
        bishops &= bishops - 1;
    }
}

void Generator::GenerateQueenPseudoLegalMoves(const std::unique_ptr<Board> &board) {
    U64 queens = board->GetBoard(fColor, Piece::Queen); // Could have multiple due to promotion
    while(queens) {
        const U8 lsb = __builtin_ctzll(queens);
        const U64 queen = 1ULL << lsb;
        U64 attacks = (hypQuint(queen, fOccupancy, fPrimaryStraightAttacks[lsb]) | hypQuint(queen, fOccupancy, fSecondaryStraightAttacks[lsb]) | hypQuint(queen, fOccupancy, fPrimaryDiagonalAttacks[lsb]) | hypQuint(queen, fOccupancy, fSecondaryDiagonalAttacks[lsb]));
        while(attacks) {
            U64 attack = 1ULL << __builtin_ctzll(attacks);
            U32 move = 0;
            SetMove(move, queen, attack, Piece::Queen, board->GetIsOccupied(attack, fOtherColor).second);
            fLegalMoves.push_back(move);
            attacks &= attacks - 1;
        }
        queens &= queens - 1;
    }
}

void Generator::GeneratePawnPseudoLegalMoves(const std::unique_ptr<Board> &board) {
    U64 pawns = board->GetBoard(fColor, Piece::Pawn);
    const U64 enemy = board->GetBoard(fOtherColor);
    const U64 promotionRank = fColor == Color::White ? RANK_8 : RANK_1;
    const U64 startRank = fColor == Color::White ? RANK_2 : RANK_7;
    while(pawns) {
        const U8 lsb = __builtin_ctzll(pawns);
        U64 pawn = 1ULL << lsb;

        // Only allow diagonal attacks if occupied by enemy piece
        U64 attacks = (fColor == Color::White ? fWhitePawnDiagonalAttacks[lsb] : fBlackPawnDiagonalAttacks[lsb]) & enemy;

        while(attacks) {
            const U64 attack = 1ULL << __builtin_ctzll(attacks);
            U32 move = 0;
            SetMove(move, pawn, attack, Piece::Pawn, board->GetIsOccupied(attack, fOtherColor).second);
            if(attack & promotionRank)
                SetMoveIsPromotion(move, true);
            fLegalMoves.push_back(move);
        }
        attacks = 0;

        // Get rid of 2-square attack if 1st or 2nd square is occupied, add single square attacks
        U64 oneSquareForward = (fColor == Color::White ? north(pawn) : south(pawn)) & ~fOccupancy;
        attacks |= oneSquareForward;
        if(oneSquareForward && (pawn & startRank))
            attacks |= (fColor == Color::White ? north(north(pawn)) : south(south(pawn))) & ~fOccupancy;

        while(attacks) {
            const U64 attack = 1ULL << __builtin_ctzll(attacks);
            U32 move = 0;
            SetMove(move, pawn, attack, Piece::Pawn, Piece::Null); // Forward moves don't take pieces
            if(attack & promotionRank)
                SetMoveIsPromotion(move, true);
            fLegalMoves.push_back(move);
        }
        pawns &= pawns - 1; // Drop the least significant bit
    }
}