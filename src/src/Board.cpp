#include <iostream>

#include "Board.hpp"

Board::Board() {
    // Setup boards
    fBoards[0] = RANK_2; // White pawns
    fBoards[1] = RANK_1 & (FILE_C | FILE_F); // White bishops
    fBoards[2] = RANK_1 & (FILE_B | FILE_G); // White knights
    fBoards[3] = RANK_1 & (FILE_A | FILE_H); // White rooks
    fBoards[4] = RANK_1 & FILE_D; // White queen
    fBoards[5] = RANK_1 & FILE_E; // White king
    fBoards[6] = RANK_7; // Black pawns
    fBoards[7] = RANK_8 & (FILE_C | FILE_F); // Black bishops
    fBoards[8] = RANK_8 & (FILE_B | FILE_G); // Black knights
    fBoards[9] = RANK_8 & (FILE_A | FILE_H); // Black rooks
    fBoards[10] = RANK_8 & FILE_D; // Black queen
    fBoards[11] = RANK_8 & FILE_E; // Black king

    fGameIsOver = false;
    fWhiteInCheckmate = false;
    fBlackInCheckmate = false;
    fWhiteCanCastle = true; // Only whether castling has already been performed
    fBlackCanCastle = true;
    fColorToMove = Color::White;

    fPseudoLegalMoves.clear();
}

void Board::GeneratePseudoLegalMoves() {
    // Generates the set of pseudo-legal moves for fColorToMove
    U64 ownPieces = GetBoard(fColorToMove);
    U64 otherPieces = GetBoard(fColorToMove == Color::White ? Color::Black : Color::White);
    fPseudoLegalMoves.clear();

    // Pawn moves
    // Knight moves
    FillPseudoKnightMoves(ownPieces);
    // King moves
    FillPseudoKingMoves(ownPieces);
    // Bishop moves
    // Rook moves
    FillPseudoRookMoves(ownPieces, otherPieces);
    // Queen moves
}

void Board::FillPseudoRookMoves(U64 ownPieces, U64 otherPieces) {
    U64 occupancy = ownPieces | otherPieces;
    U64 rooks = GetBoard(fColorToMove, Piece::Rook);
    while(rooks) {
        U64 rook = 0;
        set_bit(rook, pop_LSB(rooks));
        U64 potentialBlockers = occupancy & get_rank(rook);
        //U64 difference = potentialBlockers - (2 * squarebit(rook));

        
        // For this rook and blcoking pattern find all attacks and add them
    }
}

void Board::FillPseudoKingMoves(U64 ownPieces) {
    U64 king = GetBoard(fColorToMove, Piece::King);
    U64 attacks = fKingAttacks[get_LSB(king)] & ~ownPieces;
    while(attacks) {
        U64 attack = 0;
        set_bit(attack, pop_LSB(attacks));
        fPseudoLegalMoves.push_back(Move{king, attack, Piece::King});
    }
}

void Board::FillPseudoKnightMoves(U64 ownPieces) {
    U64 knights = GetBoard(fColorToMove, Piece::Knight);
    while(knights) {
        U64 knight = 0;
        set_bit(knight, pop_LSB(knights));
        U64 attacks = fKnightAttacks[get_LSB(knight)] & ~ownPieces;
        while(attacks) {
            U64 attack = 0;
            set_bit(attack, pop_LSB(attacks));
            fPseudoLegalMoves.push_back(Move{knight, attack, Piece::Knight});
        }
    }
}

void Board::MakeMove(Move move) {
    U64* originBoard = GetBoard(fColorToMove, move.origin);
    U64* targetBoard = GetBoard(fColorToMove == Color::White ? Color::Black : Color::White, move.target);

    if(targetBoard) // Was a piece of the other colour at the target position
        clear_bit(*targetBoard, get_LSB(move.origin));

    // Remove piece from the starting position
    clear_bit(*originBoard, get_LSB(move.origin));

    // Set the piece at the new position
    set_bit(*originBoard, get_LSB(move.target)); 


    // 0. Get appropriate boards invovled
    // 1. Check move is legal
    // 2. make the move (update bitbsoards)
    // 3. if it was a special move e.g. castling update variables
    // 4. update who is to move
}

U64* Board::GetBoard(Color color, U64 occupiedPosition) {
    int adj = 0;
    if(color == Color::Black)
        adj = 6;
    for(Piece p : PIECES) {
        if(fBoards[(int)p + adj] & occupiedPosition)
            return &fBoards[(int)p + adj];
    }
    return nullptr;
}

U64 Board::GetBoard(Color color) const {
    U64 board = 0;
    int start = color == Color::Black ? 6 : 0;
    for(Piece p : PIECES) {
        board |= GetBoard(color, p);
    }
    return board;
}

U64 Board::GetBoard(Color color, Piece piece) const {
    if(color == Color::White)
        return fBoards[(int)piece];
    return fBoards[(int)piece + 6];
}

void Board::GenerateAttackTables() {
    for(int i = 0; i < SQUARES; ++i) {
        U64 piece = 0;
        set_bit(piece, i);
        GenerateKnightAttacks(i, piece);
        GenerateKingAttacks(i, piece);
        GenerateRookAttacks(i, piece);
        GenerateBishopAttacks(i, piece);
        GenerateQueenAttacks(i, piece);
    }
}

void Board::GenerateQueenAttacks(int iPos, U64 position) {
    U64 attacks = 0;
    // Vertical distance from the primary diagonal
    int dPrimaryDiag = (get_file_number(position) - 1) - (7 - (get_rank_number(position) - 1));
    // Vertical distance from the secondary diagonal
    int dSecondaryDiag = 7 - (get_file_number(position) - 1) - (8 - get_rank_number(position));

    if(dPrimaryDiag == 0)
        attacks = PRIMARY_DIAGONAL;
    attacks = dPrimaryDiag > 0 ? PRIMARY_DIAGONAL << (abs(dPrimaryDiag) * 8) : PRIMARY_DIAGONAL >> (abs(dPrimaryDiag) * 8);

    if(dSecondaryDiag == 0)
        attacks |= SECONDARY_DIAGONAL;
    attacks |= dSecondaryDiag > 0 ? SECONDARY_DIAGONAL << (abs(dSecondaryDiag) * 8) : SECONDARY_DIAGONAL >> (abs(dSecondaryDiag) * 8);

    U64 rank = get_rank(position);
    U64 file = get_file(position);

    fQueenAttacks[iPos] = (attacks | rank | file) ^ position;;
}

void Board::GenerateBishopAttacks(int iPos, U64 position) {
    // Does not take into account blocking pieces, this is done later
    // Vertical distance from primary diagonal is just abs(x - y) (zero indexed)
    // negative value = shift primary DOWN, positive value = shift primary UP
    U64 attacks = 0;
    // Vertical distance from the primary diagonal
    int dPrimaryDiag = (get_file_number(position) - 1) - (7 - (get_rank_number(position) - 1));
    // Vertical distance from the secondary diagonal
    int dSecondaryDiag = 7 - (get_file_number(position) - 1) - (8 - get_rank_number(position));

    if(dPrimaryDiag == 0)
        attacks = PRIMARY_DIAGONAL;
    attacks = dPrimaryDiag > 0 ? PRIMARY_DIAGONAL << (abs(dPrimaryDiag) * 8) : PRIMARY_DIAGONAL >> (abs(dPrimaryDiag) * 8);

    if(dSecondaryDiag == 0)
        attacks |= SECONDARY_DIAGONAL;
    attacks |= dSecondaryDiag > 0 ? SECONDARY_DIAGONAL << (abs(dSecondaryDiag) * 8) : SECONDARY_DIAGONAL >> (abs(dSecondaryDiag) * 8);

    fBishopAttacks[iPos] = attacks ^ position;
}

void Board::GenerateRookAttacks(int iPos, U64 position) {
    // Does not take into account blocking pieces, this is done later
    U64 rank = get_rank(position);
    U64 file = get_file(position);
    fRookAttacks[iPos] = (rank | file) ^ position;
}

void Board::GenerateKnightAttacks(int iPos, U64 position) {
    // Fills the fKnightAttacks array with all attacking positions of the knight give the starting position
    U64 attacks = north(north_east(position)) | north(north_west(position)) | south(south_east(position)) | south(south_west(position)) | east(north_east(position)) | east(south_east(position)) | west(north_west(position)) | west(south_west(position));
    fKnightAttacks[iPos] = attacks;
}

void Board::GenerateKingAttacks(int iPos, U64 position) {
    U64 attacks = north(position) | east(position) | west(position) | south(position) | north_east(position) | north_west(position) | south_east(position) | south_west(position);
    fKingAttacks[iPos] = attacks;
}