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
    fKingAttacks[get_LSB(pos)] = north(pos) | east(pos) | west(pos) | south(pos) | north_east(pos) | north_west(pos) | south_east(pos) | south_west(pos);
}

void Generator::FillKnightAttackTable(const U64 pos) {
    fKnightAttacks[get_LSB(pos)] = north(north_east(pos)) | north(north_west(pos)) | south(south_east(pos)) | south(south_west(pos)) | east(north_east(pos)) | east(south_east(pos)) | west(north_west(pos)) | west(south_west(pos));
}

void Generator::FillPawnAttackTable(const U64 pos) {
    // Note this does not include special pawn moves like en-passant or promotion
    U64 attacks = 0;
    const U8 lsb = get_LSB(pos);
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
    fPrimaryStraightAttacks[get_LSB(pos)] = get_rank(pos) ^ pos;
    fSecondaryStraightAttacks[get_LSB(pos)] = get_file(pos) ^ pos;
}

void Generator::FillDiagonalAttackTables(const U64 pos) {
    // Vertical distance from primary diagonal is just abs(x - y) (zero indexed)
    // negative value = shift primary DOWN, positive value = shift primary UP
    const U8 fileNumber = get_file_number(pos);
    const U8 rankNumber = get_rank_number(pos);
    const U8 lsb = get_LSB(pos);

    const int dPrimaryDiag = (fileNumber - 1) - (8 - rankNumber); // Vertical distance from the primary diagonal
    const int dSecondaryDiag = (8 - fileNumber) - (8 - rankNumber); // Vertical distance from the secondary diagonal

    U64 primaryAttacks = dPrimaryDiag > 0 ? PRIMARY_DIAGONAL << (abs(dPrimaryDiag) * 8) : PRIMARY_DIAGONAL >> (abs(dPrimaryDiag) * 8);
    U64 secondaryAttacks = dSecondaryDiag > 0 ? SECONDARY_DIAGONAL << (abs(dSecondaryDiag) * 8) : SECONDARY_DIAGONAL >> (abs(dSecondaryDiag) * 8);

    fPrimaryDiagonalAttacks[lsb] = primaryAttacks ^ pos;
    fSecondaryDiagonalAttacks[lsb] = secondaryAttacks ^ pos;
}

void Generator::GenerateLegalMoves(const std::unique_ptr<Board> &board) {
    // TODO: implement me
    // TODO: Make me multi-threaded?

    if(CheckFiftyMoveDraw(board))
        return;
    if(CheckInsufficientMaterial(board))
        return;

    fColor = board->GetColorToMove();
    fOtherColor = fColor == Color::White ? Color::Black : Color::White;
    fOccupancy = board->GetOccupancy();
    fKing = board->GetBoard(fColor, Piece::King);

    GeneratePseudoLegalMoves(board);

    // GeneratePseudoLegalMoves(board, moves, movingColor, otherColor, occ, movingKing);
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
    const U8 nBlackKnights = CountSetBits(board->GetBoard(Color::Black, Piece::Knight));
    const U8 nBlackBishops = CountSetBits(board->GetBoard(Color::Black, Piece::Bishop));
    const U8 nWhiteKnights = CountSetBits(board->GetBoard(Color::White, Piece::Knight));
    const U8 nWhiteBishops = CountSetBits(board->GetBoard(Color::White, Piece::Bishop));
    const U8 nBlackPieces = CountSetBits(board->GetBoard(Color::Black));
    const U8 nWhitePieces = CountSetBits(board->GetBoard(Color::White));

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
    // TODO: implement me
}