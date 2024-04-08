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

void Generator::GenerateCaptureMoves(const std::shared_ptr<Board> &board) {
    fCaptureMoves.clear();
    if(CheckFiftyMoveDraw(board))
        return;
    if(CheckInsufficientMaterial(board))
        return;

    fColor = board->GetColorToMove();
    fOtherColor = fColor == Color::White ? Color::Black : Color::White;
    fOccupancy = board->GetOccupancy();
    fEnemy = board->GetBoard(fOtherColor);
    fKing = board->GetBoard(fColor, Piece::King);
    fCaptureMoves.reserve(10);

    // Doesn't generate castling (this is not a capture)
    GeneratePseudoLegalCaptureMoves(board);
    GenerateEnPassantCaptureMoves(board);
    RemoveIllegalCaptureMoves(board);
}

void Generator::GenerateLegalMoves(const std::shared_ptr<Board> &board) { // TODO: Make me multi-threaded?
    fLegalMoves.clear();
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
    GenerateEnPassantMoves(board);
    RemoveIllegalMoves(board);

    if(fLegalMoves.size() == 0) { // No legal moves, game is either stalemate or checkmate
        bool kingInCheck = IsUnderAttack(fKing, fOtherColor, board);
        if(kingInCheck) {
            board->SetState(State::Checkmate);
        } else {
            board->SetState(State::Stalemate);
        }
    }
}

bool Generator::CheckFiftyMoveDraw(const std::shared_ptr<Board> &board) {
    if(board->GetHalfMoveClock() == 100) {
        board->SetState(State::FiftyMoveRule);
        return true;
    }   
    return false;
}

bool Generator::CheckInsufficientMaterial(const std::shared_ptr<Board> &board) {
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

void Generator::GeneratePseudoLegalCaptureMoves(const std::shared_ptr<Board> &board) {
    // King
    U64 kingAttacks = fKingAttacks[__builtin_ctzll(fKing)] & fEnemy;
    while(kingAttacks) {
        const U64 attack = 1ULL << __builtin_ctzll(kingAttacks);
        U16 move = 0;
        SetMove(move, fKing, attack);
        fCaptureMoves.push_back(move);
        kingAttacks &= kingAttacks - 1; // Clear the lowest set bit
    }

    // Knight
    U64 knights = board->GetBoard(fColor, Piece::Knight);
    while(knights) {
        const U8 lsb = __builtin_ctzll(knights);
        const U64 knight = 1ULL << lsb;
        U64 attacks = fKnightAttacks[lsb] & fEnemy;
        while(attacks) {
            const U64 attack = 1ULL << __builtin_ctzll(attacks);
            U16 move = 0;
            SetMove(move, knight, attack);
            fCaptureMoves.push_back(move);
            attacks &= attacks - 1; // Clear the lowest set bit
        }
        knights &= knights - 1;
    }

    // Rooks
    U64 rooks = board->GetBoard(fColor, Piece::Rook);
    while(rooks) {
        const U8 lsb = __builtin_ctzll(rooks);
        const U64 rook = 1ULL << lsb;
        U64 attacks = (hypQuint(rook, fOccupancy, fPrimaryStraightAttacks[lsb]) | hypQuint(rook, fOccupancy, fSecondaryStraightAttacks[lsb])) & fEnemy;
        while(attacks) {
            U64 attack = 1ULL << __builtin_ctzll(attacks);
            U16 move = 0;
            SetMove(move, rook, attack);
            fCaptureMoves.push_back(move);
            attacks &= attacks - 1;
        }
        rooks &= rooks - 1;
    }

    // Bishops
    U64 bishops = board->GetBoard(fColor, Piece::Bishop);
    while(bishops) {
        const U8 lsb = __builtin_ctzll(bishops);
        const U64 bishop = 1ULL << lsb;
        U64 attacks = (hypQuint(bishop, fOccupancy, fPrimaryDiagonalAttacks[lsb]) | hypQuint(bishop, fOccupancy, fSecondaryDiagonalAttacks[lsb])) & fEnemy;
        while(attacks) {
            U64 attack = 1ULL << __builtin_ctzll(attacks);
            U16 move = 0;
            SetMove(move, bishop, attack);
            fCaptureMoves.push_back(move);
            attacks &= attacks - 1;
        }
        bishops &= bishops - 1;
    }

    // Queens
    U64 queens = board->GetBoard(fColor, Piece::Queen); // Could have multiple due to promotion
    while(queens) {
        const U8 lsb = __builtin_ctzll(queens);
        const U64 queen = 1ULL << lsb;
        U64 attacks = (hypQuint(queen, fOccupancy, fPrimaryStraightAttacks[lsb]) | hypQuint(queen, fOccupancy, fSecondaryStraightAttacks[lsb]) | hypQuint(queen, fOccupancy, fPrimaryDiagonalAttacks[lsb]) | hypQuint(queen, fOccupancy, fSecondaryDiagonalAttacks[lsb])) & fEnemy;
        while(attacks) {
            U64 attack = 1ULL << __builtin_ctzll(attacks);
            U16 move = 0;
            SetMove(move, queen, attack);
            fCaptureMoves.push_back(move);
            attacks &= attacks - 1;
        }
        queens &= queens - 1;
    }

    // Pawns
    U64 pawns = board->GetBoard(fColor, Piece::Pawn);
    while(pawns) {
        const U8 lsb = __builtin_ctzll(pawns);
        const U64 pawn = 1ULL << lsb;
        U64 attacks = (fColor == Color::White ? (north_east(pawn) | north_west(pawn)) : (south_east(pawn) | south_west(pawn))) & fEnemy;
        while(attacks) {
            U64 attack = 1ULL << __builtin_ctzll(attacks);
            U16 move = 0;
            SetMove(move, pawn, attack);
            fCaptureMoves.push_back(move);
            attacks &= attacks - 1;
        }
        pawns &= pawns - 1;
    }
}

void Generator::GenerateEnPassantCaptureMoves(const std::shared_ptr<Board> &board) {
    // En-passant not possible so throw away early
    if((!board->GetWasLoadedFromFEN() && board->GetNMoves() < MIN_MOVES_FOR_ENPASSANT) || 
        (board->GetWasLoadedFromFEN() && !board->GetEnPassantFEN() && board->GetNMoves() < 1))
        return;

    // FEN loaded position with en-passant move immediately available
    if(board->GetWasLoadedFromFEN() && board->GetEnPassantFEN() && board->GetNMoves() < 1) {
        U64 attackSquares = 0;
        U64 target = board->GetEnPassantFEN();
        if(fColor == Color::White) {
            attackSquares = (south_east(target) | south_west(target)) & board->GetBoard(Color::White, Piece::Pawn);
        } else {
            attackSquares = (north_east(target) | north_west(target)) & board->GetBoard(Color::Black, Piece::Pawn);
        }

        while(attackSquares) {
            U64 pawn = 1ULL << __builtin_ctzll(attackSquares);
            U16 move = 0;
            SetMove(move, pawn, target);
            fLegalMoves.push_back(move);
            attackSquares &= attackSquares - 1;
        }
    }

    // No en-passant possible if no previous moves (e.g. FEN loaded position)
    if(board->GetNMoves() < 1) 
        return;

    // Can't be wrapped in an else bracket due to undoing moves
    // Now run the usual code
    U16 lastMove = board->GetLastMove();
    U64 lastMoveTarget = GetMoveTarget(lastMove);
    U64 lastMoveOrigin = GetMoveOrigin(lastMove);

    // Faster return if you know en-passant will not be possible
    if(board->GetLastPieceMoved() != Piece::Pawn || GetMoveIsCastling(lastMove)) // TODO: Add this back somehow || board->GetMoveIsEnPassant(lastMove)
        return;

    U64 enPassantPawns = 0;
    U64 attackingPawns = board->GetBoard(fColor, Piece::Pawn);
    if(((get_rank(lastMoveTarget) & RANK_5) && (fOtherColor == Color::Black) && (get_rank(lastMoveOrigin) & RANK_7))
    || ((get_rank(lastMoveTarget) & RANK_4) && (fOtherColor == Color::White) && (get_rank(lastMoveOrigin) & RANK_2))) {
        enPassantPawns = (east(lastMoveTarget) | west(lastMoveTarget)) & attackingPawns;
    }

    while(enPassantPawns) {
        const U64 pawn = 1ULL << __builtin_ctzll(enPassantPawns);
        U16 move = 0;
        SetMove(move, pawn, fColor == Color::White ? north(lastMoveTarget) : south(lastMoveTarget));
        fLegalMoves.push_back(move);
        enPassantPawns &= enPassantPawns - 1;
    }
}

void Generator::RemoveIllegalCaptureMoves(const std::shared_ptr<Board> &board) {
    // Check all the illegal moves, e.g. do they result in your own king being in check?
    // Check all the illegal moves, e.g. do they result in your own king being in check?
    const U64 underAttack = GetAttacks(board, fOtherColor);

    // TODO: lost a flippin pawn in here somehow
    if(fKing & underAttack) // Player to move is in check, only moves resolving the check can be permitted
        PruneCheckMoves(board, true);
    fPinnedPieces.clear(); // Empty the vector from the last call
    for(Direction d : DIRECTIONS) {
        AddAbolsutePins(board, d);
    }
    fPinnedPositions = std::accumulate(
        fPinnedPieces.begin(), fPinnedPieces.end(), U64(0),
        [](U64 acc, const std::pair<U64, U64>& p) {
            return acc | p.first;
        }
    );

    for(int iMove = 0; iMove < (int)fCaptureMoves.size(); iMove++) {
        const U16 m = fCaptureMoves[iMove];
        const U64 moveOrigin = GetMoveOrigin(m);
        const U64 moveTarget = GetMoveTarget(m);
        // King cant move to squares the opponent attacks
        if((board->GetMovePiece(m) == Piece::King) && (moveTarget & underAttack)) {
            fCaptureMoves.erase(std::begin(fCaptureMoves) + iMove);
            iMove--;
        } else if(fPinnedPositions & moveOrigin) { // Piece originates from a pinned position
            // Absolutely pinned pieces may not move, unless it is a capture of that piece or along pinning ray
            for(const std::pair<U64, U64> &pins : fPinnedPieces) {
                // !Piece moving from pinned position to somewhere on the associated pinning ray (incl capture)
                if((moveOrigin & pins.first) && (moveTarget & ~pins.second)) {
                    // Moving to somewhere off the absolutely pinning ray (illegal)
                    fCaptureMoves.erase(std::begin(fCaptureMoves) + iMove);
                    iMove--;
                }
            }
        } else if(board->GetMoveIsEnPassant(m, board->GetMovePiece(m), board->GetIsOccupied(moveTarget).second == Piece::Null)) { // Need to be manually checked due to rook rays
            U64 activeRank = get_rank(moveOrigin);
            U64 kingOnRank = fKing & activeRank; // King shares the rank with the en-passanting pawn
            U64 rookOnRank = activeRank & (board->GetBoard(fOtherColor, Piece::Rook) | board->GetBoard(fOtherColor, Piece::Queen)); // Rook or queen have sliding straight attacks

            if(kingOnRank && rookOnRank) { // King, two pawns and rook/queen(s) occupy the same rank
                // Is en-passant so the captured pawn occupies the same rank as well, if we remove our pawn and the
                // captured pawn does this leave our king on a rank with just itself and the attacking rook/queen

                // MSB is the left-most bit (i.e. that with lowest index)
                const U8 kingMSB = __builtin_clzll(fKing);
                const U8 rookMSB = __builtin_clzll(rookOnRank);

                int attackingRookBit = kingMSB < rookMSB ? 63 - rookMSB : __builtin_ctzll(rookOnRank);
                U64 rook = 1ULL << attackingRookBit; // Again could be a queen but it doesn't matter, this is the one checking king if move happens
                U64 rookShift = kingMSB < rookMSB ? east(rook) : west(rook);
                U64 takenPawn = fColor == Color::White ? south(moveTarget) : north(moveTarget);

                // Make a custom occupancy mask to cut the rook ray down
                U64 mask = fKing | rook | rookShift;
                U64 rookRay = hypQuint(rook, mask, fPrimaryStraightAttacks[attackingRookBit]);

                // Subtract from the ray our pawns that we know intersect the ray
                rookRay ^= (moveOrigin | takenPawn | fKing | rookShift);

                if(!(rookRay & fOccupancy)) { // No pieces on the ray so if en-passant happens king will be in check
                    fCaptureMoves.erase(std::begin(fCaptureMoves) + iMove);
                    iMove--;
                }
            }
        }
    }
}

void Generator::GeneratePseudoLegalMoves(const std::shared_ptr<Board> &board) {
    GeneratePawnPseudoLegalMoves(board);
    GenerateKingPseudoLegalMoves();
    GenerateKnightPseudoLegalMoves(board);
    GenerateBishopPseudoLegalMoves(board);
    GenerateRookPseudoLegalMoves(board);
    GenerateQueenPseudoLegalMoves(board);

    // Erase all moves moving onto a square of its own colour
    const U64 selfOccupancy = board->GetBoard(fColor);
    fLegalMoves.erase(std::remove_if(fLegalMoves.begin(), fLegalMoves.end(), [&](U16 move) {
        return GetMoveTarget(move) & selfOccupancy;
    }), fLegalMoves.end());
}

void Generator::GenerateKingPseudoLegalMoves() {
    U64 attacks = fKingAttacks[__builtin_ctzll(fKing)];
    while(attacks) {
        const U64 attack = 1ULL << __builtin_ctzll(attacks);
        U16 move = 0;
        SetMove(move, fKing, attack);
        fLegalMoves.push_back(move);
        attacks &= attacks - 1; // Clear the lowest set bit
    }
}

void Generator::GenerateKnightPseudoLegalMoves(const std::shared_ptr<Board> &board) {
    U64 knights = board->GetBoard(fColor, Piece::Knight);
    while(knights) {
        const U8 lsb = __builtin_ctzll(knights);
        const U64 knight = 1ULL << lsb;
        U64 attacks = fKnightAttacks[lsb];
        while(attacks) {
            const U64 attack = 1ULL << __builtin_ctzll(attacks);
            U16 move = 0;
            SetMove(move, knight, attack);
            fLegalMoves.push_back(move);
            attacks &= attacks - 1; // Clear the lowest set bit
        }
        knights &= knights - 1;
    }
}

void Generator::GenerateRookPseudoLegalMoves(const std::shared_ptr<Board> &board) {
    U64 rooks = board->GetBoard(fColor, Piece::Rook);
    while(rooks) {
        const U8 lsb = __builtin_ctzll(rooks);
        const U64 rook = 1ULL << lsb;
        U64 attacks = (hypQuint(rook, fOccupancy, fPrimaryStraightAttacks[lsb]) | hypQuint(rook, fOccupancy, fSecondaryStraightAttacks[lsb]));
        while(attacks) {
            U64 attack = 1ULL << __builtin_ctzll(attacks);
            U16 move = 0;
            SetMove(move, rook, attack);
            fLegalMoves.push_back(move);
            attacks &= attacks - 1;
        }
        rooks &= rooks - 1;
    }
}

void Generator::GenerateBishopPseudoLegalMoves(const std::shared_ptr<Board> &board) {
    U64 bishops = board->GetBoard(fColor, Piece::Bishop);
    while(bishops) {
        const U8 lsb = __builtin_ctzll(bishops);
        const U64 bishop = 1ULL << lsb;
        U64 attacks = (hypQuint(bishop, fOccupancy, fPrimaryDiagonalAttacks[lsb]) | hypQuint(bishop, fOccupancy, fSecondaryDiagonalAttacks[lsb]));
        while(attacks) {
            U64 attack = 1ULL << __builtin_ctzll(attacks);
            U16 move = 0;
            SetMove(move, bishop, attack);
            fLegalMoves.push_back(move);
            attacks &= attacks - 1;
        }
        bishops &= bishops - 1;
    }
}

void Generator::GenerateQueenPseudoLegalMoves(const std::shared_ptr<Board> &board) {
    U64 queens = board->GetBoard(fColor, Piece::Queen); // Could have multiple due to promotion
    while(queens) {
        const U8 lsb = __builtin_ctzll(queens);
        const U64 queen = 1ULL << lsb;
        U64 attacks = (hypQuint(queen, fOccupancy, fPrimaryStraightAttacks[lsb]) | hypQuint(queen, fOccupancy, fSecondaryStraightAttacks[lsb]) | hypQuint(queen, fOccupancy, fPrimaryDiagonalAttacks[lsb]) | hypQuint(queen, fOccupancy, fSecondaryDiagonalAttacks[lsb]));
        while(attacks) {
            U64 attack = 1ULL << __builtin_ctzll(attacks);
            U16 move = 0;
            SetMove(move, queen, attack);
            fLegalMoves.push_back(move);
            attacks &= attacks - 1;
        }
        queens &= queens - 1;
    }
}

void Generator::GeneratePawnPseudoLegalMoves(const std::shared_ptr<Board> &board) {
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
            U16 move = 0;
            SetMove(move, pawn, attack);
            if(attack & promotionRank) {
                for(Piece p : PROMOTION_PIECES) {
                    SetMovePromotionPiece(move, p);
                    fLegalMoves.push_back(move);
                }
            } else {
                fLegalMoves.push_back(move);
            }
            attacks &= attacks - 1;
        }
        attacks = 0;

        // Get rid of 2-square attack if 1st or 2nd square is occupied, add single square attacks
        U64 oneSquareForward = (fColor == Color::White ? north(pawn) : south(pawn)) & ~fOccupancy;
        attacks |= oneSquareForward;
        if(oneSquareForward && (pawn & startRank))
            attacks |= (fColor == Color::White ? north(north(pawn)) : south(south(pawn))) & ~fOccupancy;

        while(attacks) {
            const U64 attack = 1ULL << __builtin_ctzll(attacks);
            U16 move = 0;
            SetMove(move, pawn, attack); // Forward moves don't take pieces
            if(attack & promotionRank) {
                for(Piece p : PROMOTION_PIECES) {
                    SetMovePromotionPiece(move, p);
                    fLegalMoves.push_back(move);
                }
            } else {
                fLegalMoves.push_back(move);
            }
            attacks &= attacks - 1;
        }
        pawns &= pawns - 1; // Drop the least significant bit
    }
}

void Generator::GenerateCastlingMoves(const std::shared_ptr<Board> &board) {
    // Return quickly if we know castling is not possible
    if(board->GetNMoves() < MIN_MOVES_FOR_CASTLING && !board->GetWasLoadedFromFEN())
        return;

    U16 move = 0;
    if(fColor == Color::White && !board->GetWhiteKingMoved()) {
        if(!board->GetWhiteKingsideRookMoved() && 
            IsCastlingPossible(KING_SIDE_CASTLING_MASK_WHITE, KING_SIDE_CASTLING_OCCUPANCY_MASK_WHITE, board)) 
        {
            SetMove(move, fKing, SQUARE_G1);
            SetMoveIsCastling(move, true);
            fLegalMoves.push_back(move);
            move = 0;
        }
        if(!board->GetWhiteQueensideRookMoved() &&
            IsCastlingPossible(QUEEN_SIDE_CASTLING_MASK_WHITE, QUEEN_SIDE_CASTLING_OCCUPANCY_MASK_WHITE, board)) {
            SetMove(move, fKing, SQUARE_C1);
            SetMoveIsCastling(move, true);
            fLegalMoves.push_back(move);
            move = 0;
        }
    } else if(fColor == Color::Black && !board->GetBlackKingMoved()) {
        if(!board->GetBlackKingsideRookMoved() &&
            IsCastlingPossible(KING_SIDE_CASTLING_MASK_BLACK, KING_SIDE_CASTLING_OCCUPANCY_MASK_BLACK, board)) 
        {
            SetMove(move, fKing, SQUARE_G8);
            SetMoveIsCastling(move, true);
            fLegalMoves.push_back(move);
            move = 0;
        }
        if (!board->GetBlackQueensideRookMoved() &&
            IsCastlingPossible(QUEEN_SIDE_CASTLING_MASK_BLACK, QUEEN_SIDE_CASTLING_OCCUPANCY_MASK_BLACK, board)) 
        {
            SetMove(move, fKing, SQUARE_C8);
            SetMoveIsCastling(move, true);
            fLegalMoves.push_back(move);
            move = 0;
        }
    }
}

bool Generator::IsCastlingPossible(U64 castlingMask, U64 occupancyMask, const std::shared_ptr<Board> &board) {
    return !(fOccupancy & occupancyMask) && !IsUnderAttack(castlingMask, fOtherColor, board);
}

bool Generator::IsUnderAttack(const U64 mask, const Color attackingColor, const std::shared_ptr<Board> &board) {    
    const U64 attacker = board->GetBoard(attackingColor);
    const U64 attacks = GetAttacks(board, attackingColor);
    return attacks & mask & ~attacker;
}

U64 Generator::GetAttacks(const std::shared_ptr<Board> &board, const Color attackingColor) {
    U64 attacks = 0;
    const U64 occ = board->GetOccupancy(); // Cannot replace with fOccupancy?????

    // Pawns (only diagonal forwards check as only care about attacks)
    const U64 pawns = board->GetBoard(attackingColor, Piece::Pawn);
    if(attackingColor == Color::White) {
        attacks |= (north_east(pawns) | north_west(pawns));
    } else {
        attacks |= (south_east(pawns) | south_west(pawns));   
    }

    // Knight attacks
    U64 knights = board->GetBoard(attackingColor, Piece::Knight);
    while(knights) {
        attacks |= fKnightAttacks[__builtin_ctzll(knights)];
        knights &= knights - 1;
    }

    // King
    U64 king = board->GetBoard(attackingColor, Piece::King);
    attacks |= fKingAttacks[__builtin_ctzll(king)];

    // Bishops
    U64 bishops = board->GetBoard(attackingColor, Piece::Bishop);
    while(bishops) {
        const U8 lsb = __builtin_ctzll(bishops);
        const U64 bishop = 1ULL << lsb;
        attacks |= (hypQuint(bishop, occ, fPrimaryDiagonalAttacks[lsb]) | hypQuint(bishop, occ, fSecondaryDiagonalAttacks[lsb]));
        bishops &= bishops - 1;
    }

    // Rooks
    U64 rooks = board->GetBoard(attackingColor, Piece::Rook);
    while(rooks) {
        const U8 lsb = __builtin_ctzll(rooks);
        const U64 rook = 1ULL << lsb;
        attacks |= (hypQuint(rook, occ, fPrimaryStraightAttacks[lsb]) | hypQuint(rook, occ, fSecondaryStraightAttacks[lsb]));
        rooks &= rooks - 1;
    }

    // Queens
    U64 queens = board->GetBoard(attackingColor, Piece::Queen);
    while(queens) {
        const U8 lsb = __builtin_ctzll(queens);
        const U64 queen = 1ULL << lsb;
        attacks |= (hypQuint(queen, occ, fPrimaryStraightAttacks[lsb]) | hypQuint(queen, occ, fSecondaryStraightAttacks[lsb]) | hypQuint(queen, occ, fPrimaryDiagonalAttacks[lsb]) | hypQuint(queen, occ, fSecondaryDiagonalAttacks[lsb]));
        queens &= queens - 1;
    }

    return attacks; // Don't exclude your own pieces since they are protected so king cannot take them
}

void Generator::GenerateEnPassantMoves(const std::shared_ptr<Board> &board) {
    // En-passant not possible so throw away early
    if((!board->GetWasLoadedFromFEN() && board->GetNMoves() < MIN_MOVES_FOR_ENPASSANT) || 
        (board->GetWasLoadedFromFEN() && !board->GetEnPassantFEN() && board->GetNMoves() < 1))
        return;

    // FEN loaded position with en-passant move immediately available
    if(board->GetWasLoadedFromFEN() && board->GetEnPassantFEN() && board->GetNMoves() < 1) {
        U64 attackSquares = 0;
        U64 target = board->GetEnPassantFEN();
        if(fColor == Color::White) {
            attackSquares = (south_east(target) | south_west(target)) & board->GetBoard(Color::White, Piece::Pawn);
        } else {
            attackSquares = (north_east(target) | north_west(target)) & board->GetBoard(Color::Black, Piece::Pawn);
        }

        while(attackSquares) {
            U64 pawn = 1ULL << __builtin_ctzll(attackSquares);
            U16 move = 0;
            SetMove(move, pawn, target);
            fLegalMoves.push_back(move);
            attackSquares &= attackSquares - 1;
        }
    }

    // No en-passant possible if no previous moves (e.g. FEN loaded position)
    if(board->GetNMoves() < 1) 
        return;

    // Can't be wrapped in an else bracket due to undoing moves
    // Now run the usual code
    U16 lastMove = board->GetLastMove();
    U64 lastMoveTarget = GetMoveTarget(lastMove);
    U64 lastMoveOrigin = GetMoveOrigin(lastMove);

    // Faster return if you know en-passant will not be possible
    if(board->GetLastPieceMoved() != Piece::Pawn || GetMoveIsCastling(lastMove)) // TODO: Add this back somehow || board->GetMoveIsEnPassant(lastMove)
        return;

    U64 enPassantPawns = 0;
    U64 attackingPawns = board->GetBoard(fColor, Piece::Pawn);
    if(((get_rank(lastMoveTarget) & RANK_5) && (fOtherColor == Color::Black) && (get_rank(lastMoveOrigin) & RANK_7))
    || ((get_rank(lastMoveTarget) & RANK_4) && (fOtherColor == Color::White) && (get_rank(lastMoveOrigin) & RANK_2))) {
        enPassantPawns = (east(lastMoveTarget) | west(lastMoveTarget)) & attackingPawns;
    }

    while(enPassantPawns) {
        const U64 pawn = 1ULL << __builtin_ctzll(enPassantPawns);
        U16 move = 0;
        SetMove(move, pawn, fColor == Color::White ? north(lastMoveTarget) : south(lastMoveTarget));
        fLegalMoves.push_back(move);
        enPassantPawns &= enPassantPawns - 1;
    }
}

void Generator::RemoveIllegalMoves(const std::shared_ptr<Board> &board) {
    // TODO: Rewrite this from the ground up to make it faster and more readable...

    // Check all the illegal moves, e.g. do they result in your own king being in check?
    const U64 underAttack = GetAttacks(board, fOtherColor);

    // TODO: lost a flippin pawn in here somehow
    if(fKing & underAttack) // Player to move is in check, only moves resolving the check can be permitted
        PruneCheckMoves(board, false);

    fPinnedPieces.clear(); // Empty the vector from the last call
    for(Direction d : DIRECTIONS) {
        AddAbolsutePins(board, d);
    }

    fPinnedPositions = std::accumulate(
        fPinnedPieces.begin(), fPinnedPieces.end(), U64(0),
        [](U64 acc, const std::pair<U64, U64>& p) {
            return acc | p.first;
        }
    );

    for(int iMove = 0; iMove < (int)fLegalMoves.size(); iMove++) {
        const U16 m = fLegalMoves[iMove];
        const U64 moveOrigin = GetMoveOrigin(m);
        const U64 moveTarget = GetMoveTarget(m);
        // King cant move to squares the opponent attacks
        if((board->GetMovePiece(m) == Piece::King) && (moveTarget & underAttack)) {
            fLegalMoves.erase(std::begin(fLegalMoves) + iMove);
            iMove--;
        } else if(fPinnedPositions & moveOrigin) { // Piece originates from a pinned position
            // Absolutely pinned pieces may not move, unless it is a capture of that piece or along pinning ray
            for(const std::pair<U64, U64> &pins : fPinnedPieces) {
                // !Piece moving from pinned position to somewhere on the associated pinning ray (incl capture)
                if((moveOrigin & pins.first) && (moveTarget & ~pins.second)) {
                    // Moving to somewhere off the absolutely pinning ray (illegal)
                    fLegalMoves.erase(std::begin(fLegalMoves) + iMove);
                    iMove--;
                }
            }
        } else if(board->GetMoveIsEnPassant(m, board->GetMovePiece(m), board->GetIsOccupied(moveTarget).second == Piece::Null)) { // Need to be manually checked due to rook rays
            U64 activeRank = get_rank(moveOrigin);
            U64 kingOnRank = fKing & activeRank; // King shares the rank with the en-passanting pawn
            U64 rookOnRank = activeRank & (board->GetBoard(fOtherColor, Piece::Rook) | board->GetBoard(fOtherColor, Piece::Queen)); // Rook or queen have sliding straight attacks

            if(kingOnRank && rookOnRank) { // King, two pawns and rook/queen(s) occupy the same rank
                // Is en-passant so the captured pawn occupies the same rank as well, if we remove our pawn and the
                // captured pawn does this leave our king on a rank with just itself and the attacking rook/queen

                // MSB is the left-most bit (i.e. that with lowest index)
                const U8 kingMSB = __builtin_clzll(fKing);
                const U8 rookMSB = __builtin_clzll(rookOnRank);

                int attackingRookBit = kingMSB < rookMSB ? 63 - rookMSB : __builtin_ctzll(rookOnRank);
                U64 rook = 1ULL << attackingRookBit; // Again could be a queen but it doesn't matter, this is the one checking king if move happens
                U64 rookShift = kingMSB < rookMSB ? east(rook) : west(rook);
                U64 takenPawn = fColor == Color::White ? south(moveTarget) : north(moveTarget);

                // Make a custom occupancy mask to cut the rook ray down
                U64 mask = fKing | rook | rookShift;
                U64 rookRay = hypQuint(rook, mask, fPrimaryStraightAttacks[attackingRookBit]);

                // Subtract from the ray our pawns that we know intersect the ray
                rookRay ^= (moveOrigin | takenPawn | fKing | rookShift);

                if(!(rookRay & fOccupancy)) { // No pieces on the ray so if en-passant happens king will be in check
                    fLegalMoves.erase(std::begin(fLegalMoves) + iMove);
                    iMove--;
                }
            }
        }
    }
}

void Generator::AddAbolsutePins(const std::shared_ptr<Board> &board, Direction d) {
    // Make artificial occupancy to block in the king and only get the north ray
    const U8 lsb = __builtin_ctzll(fKing);
    U64 rayOccupancy = board->GetBoard(fOtherColor);
    const U64 defendingRook = board->GetBoard(fOtherColor, Piece::Rook);
    const U64 defendingBishop = board->GetBoard(fOtherColor, Piece::Bishop);
    const U64 ownPieces = board->GetBoard(fColor);
    U64 rayMask = 0;
    U64 enemies = board->GetBoard(fOtherColor, Piece::Queen); // Queen can pin both ways so is always an "enemy"
    U64 kingShift = 0;

    switch(d) {
        case Direction::North:
            kingShift |= south(fKing);
            rayMask = fSecondaryStraightAttacks[lsb];
            enemies |= defendingRook;
            break;
        case Direction::East:
            kingShift |= west(fKing);
            rayMask = fPrimaryStraightAttacks[lsb];
            enemies |= defendingRook;
            break;
        case Direction::West:
            kingShift |= east(fKing);
            rayMask = fPrimaryStraightAttacks[lsb];
            enemies |= defendingRook;
            break;
        case Direction::South:
            kingShift |= north(fKing);
            rayMask = fSecondaryStraightAttacks[lsb];
            enemies |= defendingRook;
            break;
        case Direction::NorthEast:
            kingShift |= south_west(fKing);
            rayMask = fSecondaryDiagonalAttacks[lsb];
            enemies |= defendingBishop;
            break;
        case Direction::NorthWest:
            kingShift |= south_east(fKing);
            rayMask = fPrimaryDiagonalAttacks[lsb];
            enemies |= defendingBishop;
            break;
        case Direction::SouthEast:
            kingShift |= north_west(fKing);
            rayMask = fPrimaryDiagonalAttacks[lsb];
            enemies |= defendingBishop;
            break;
        case Direction::SouthWest:
            kingShift |= north_east(fKing);
            rayMask = fSecondaryDiagonalAttacks[lsb];
            enemies |= defendingBishop;
            break;
        default:
            return;
            break;
    }

    rayOccupancy |= kingShift;
    U64 ray = hypQuint(fKing, rayOccupancy, rayMask) ^ kingShift;
    U64 rayAndEnemy = ray & enemies;

    if(rayAndEnemy) { // Enemy piece on the ray pointing at the king (that could attack king, if not blocked)
        // Note that rayAndEnemy is actually the position of the attacking piece on the ray
        U64 potentialPin = ray & ownPieces; // All your pieces that exist on the ray (between king and attacking piece)
        if(CountSetBits(potentialPin) == 1) { // A single piece is on the ray and so absolutely pinned
            fPinnedPieces.push_back(std::make_pair(potentialPin, ray));
        } // else : None of your pieces in the way so the king is in check from attacker
    }
}

U64 Generator::GetPawnAttacks(const std::shared_ptr<Board> &board, bool colorToMoveAttacks) {
    Color attackingColor = colorToMoveAttacks ? board->GetColorToMove() : (board->GetColorToMove() == Color::White ? Color::Black : Color::White);
    U64 pawns = board->GetBoard(attackingColor, Piece::Pawn);
    // Drop the absolutely pinned pawns for improved accuracy
    pawns &= ~fPinnedPositions;
    
    if(attackingColor == Color::White) {
        return north_east(pawns) | north_west(pawns);
    } else {
        return south_east(pawns) | south_west(pawns);
    }
}

void Generator::PruneCheckMoves(const std::shared_ptr<Board> &board, const bool copyToCapures) {
    std::vector<U16> validMoves;
    for (U16 move : (copyToCapures ? fCaptureMoves : fLegalMoves)) {
        if (GetMoveIsCastling(move)) {
            continue; // Skip castling moves
        }
        bool pieceIsKing = board->GetMovePiece(move) == Piece::King;
        board->MakeMove(move);
        U64 underAttack = GetAttacks(board, fColor == Color::White ? Color::Black : Color::White);
        // King may have moved so can't use fActiveKing
        U64 newKing = pieceIsKing ? board->GetBoard(fColor, Piece::King) : fKing;
        if(!(underAttack & newKing)) {
            validMoves.push_back(move); // Move is legal, add it to the vector
        }
        board->UndoMove();
    }
    // Replace fLegalMoves with validMoves
    if(copyToCapures) {
        fCaptureMoves = std::move(validMoves);
    } else {
        fLegalMoves = std::move(validMoves);
    }
}

bool Generator::GetMoveIsLegal(U16 &move) {
    const U64 moveOrigin = GetMoveOrigin(move);
    const U64 moveTarget = GetMoveTarget(move);

    for(U16 m : fLegalMoves) {
        if((moveOrigin & GetMoveOrigin(m)) && (moveTarget & GetMoveTarget(m))) {
            move = m;
            return true;
        }
    }
    return false;
}
