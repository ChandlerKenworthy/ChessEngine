#include <iostream>

#include "Board.hpp"

Board::Board() {
    Reset();
}

void Board::Reset() {
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
    fWhiteHasCastled = false; // Only whether castling has already been performed
    fBlackHasCastled = false;
    fColorToMove = Color::White;

    fLegalMoves.clear();
    fMadeMoves.clear();
}

void Board::GenerateLegalMoves() {
    GeneratePseudoLegalMoves(); // fLegalMoves now full of pseudo-legal moves
    U64 playerKing = GetBoard(fColorToMove, Piece::King);

    // add castling if available

    // check for en-passant (was the last move a pawn move 2 squares forward?)
    AddEnPassant();

    // Make the pseudo-legal move and see if our king now lives on an attack ray of another piece
    RemoveIllegalMoves(); // Must be after AddEnPassant and castling to ensure strictness

    // set if other king now in check as a result of this move
}

void Board::RemoveIllegalMoves() {
    // Check each move of fLegalMoves to see if, after being made the board is in a legal game state
    // if it is yay, if it is not remove the move
}

void Board::AddEnPassant() {
    Move *lastMove = &fMadeMoves.back();
    if(lastMove->piece != Piece::Pawn)
        return;
    U64 targetPawnFile = get_file(lastMove->target);
    U64 pawns = GetBoard(fColorToMove, Piece::Pawn);
    U64 enPassantPawns = 0;
    if((fColorToMove == Color::White) && (lastMove->origin & RANK_7) && (lastMove->target & RANK_5)) {
        // Was a double-move forward with a pawn by black last turn
        // Check if the move placed the pawn on an adjacent file to any of your pawns on rank 5
        enPassantPawns = pawns & RANK_5 & (east(targetPawnFile) | west(targetPawnFile));
    } else if((fColorToMove == Color::Black) && (lastMove->origin & RANK_2) && (lastMove->target & RANK_4)) {
        // Was a double-move forward with a pawn by black last turn
        // Check if the move placed the pawn on an adjacent file to any of your pawns on rank 4
        enPassantPawns = pawns & RANK_4 & (east(targetPawnFile) | west(targetPawnFile));
    }
    while(enPassantPawns) {
        U64 pawn = 0;
        set_bit(pawn, pop_LSB(enPassantPawns));
        U64 rank = fColorToMove == Color::White ? RANK_6 : RANK_3;
        fLegalMoves.push_back(Move{pawn, targetPawnFile & rank, Piece::Pawn, Piece::Pawn});
    }
}

void Board::GeneratePseudoLegalMoves() {
    // Generates the set of pseudo-legal moves for fColorToMove
    U64 ownPieces = GetBoard(fColorToMove);
    U64 otherPieces = GetBoard(fColorToMove == Color::White ? Color::Black : Color::White);
    fLegalMoves.clear();

    FillPseudoPawnMoves(ownPieces, otherPieces);   // Pawn moves
    FillPseudoKnightMoves(ownPieces, otherPieces); // Knight moves
    FillPseudoKingMoves(ownPieces, otherPieces);   // King moves
    FillPseudoBishopMoves(ownPieces, otherPieces); // Bishop moves
    FillPseudoRookMoves(ownPieces, otherPieces);   // Rook moves
    FillPseudoQueenMoves(ownPieces, otherPieces);  // Queen moves
}

void Board::FillPseudoPawnMoves(U64 ownPieces, U64 otherPieces) {
    U64 occ = ownPieces | otherPieces;
    U64 pawns = GetBoard(fColorToMove, Piece::Pawn);
    while(pawns) {
        U64 pawn = 0;
        set_bit(pawn, pop_LSB(pawns));
        U64 attacks = 0;
        if(fColorToMove == Color::White) {
            attacks |= ((pawn << 8) & ~occ) | ((pawn << 7) & otherPieces) | ((pawn << 9) & otherPieces);
            if((pawn & RANK_2) && ~((pawn << 8) & occ) && ~((pawn << 16) & occ)) 
                // Starting position - move 2 forward
                attacks |= (pawn << 16);
        } else {
            attacks |= ((pawn >> 8) & ~occ) | ((pawn >> 7) & otherPieces) | ((pawn >> 9) & otherPieces);
            if((pawn & RANK_7) && ~((pawn >> 8) & occ) && ~((pawn >> 16) & occ)) 
                // Starting position - move 2 forward
                attacks |= (pawn >> 16);
        }
        while(attacks) {
            U64 attack = 0;
            set_bit(attack, pop_LSB(attacks));
            Piece takenPiece = Piece::Null;
            if(attack & otherPieces) // Find the type of piece that is getting removed
                takenPiece = GetPiece(fColorToMove == Color::White ? Color::Black : Color::White, attack);
            fLegalMoves.push_back(Move{pawn, attack, Piece::Pawn, takenPiece});
        }
    }
}

Piece Board::GetPiece(Color color, U64 pos) {
    int adj = 0;
    if(color == Color::Black)
        adj = 6;
    for(Piece p : PIECES) {
        if(GetBoard(color, p) & pos)
            return p;
    }
    return Piece::Null;
}

void Board::FillPseudoQueenMoves(U64 ownPieces, U64 otherPieces) {
    U64 occupancy = ownPieces | otherPieces;
    U64 queen = GetBoard(fColorToMove, Piece::Queen);

    // TODO: maybe given a queen position have 4 lookup tables for positive and negative rays

    // Vertical distance from the primary diagonal
    int dPrimaryDiag = get_file_number(queen) - 9 + get_rank_number(queen);
    // Vertical distance from the secondary diagonal
    int dSecondaryDiag = get_rank_number(queen) - get_file_number(queen);

    U64 mask1 = 0;
    if(dPrimaryDiag == 0) {
        mask1 = PRIMARY_DIAGONAL;
    } else {
        mask1 = dPrimaryDiag > 0 ? PRIMARY_DIAGONAL << (abs(dPrimaryDiag) * 8) : PRIMARY_DIAGONAL >> (abs(dPrimaryDiag) * 8);
    }

    U64 mask2 = 0;
    if(dSecondaryDiag == 0) {
        mask2 = SECONDARY_DIAGONAL;
    } else {
        mask2 = dSecondaryDiag > 0 ? SECONDARY_DIAGONAL << (abs(dSecondaryDiag) * 8) : SECONDARY_DIAGONAL >> (abs(dSecondaryDiag) * 8);
    }

    U64 attacks = (hypQuint(queen, occupancy, mask1) | hypQuint(queen, occupancy, mask2) | hypQuint(queen, occupancy, get_rank(queen)) | hypQuint(queen, occupancy, get_file(queen))) & ~ownPieces;
    while(attacks) {
        U64 attack = 0;
        set_bit(attack, pop_LSB(attacks));
        Piece takenPiece = Piece::Null;
        if(attack & otherPieces)
            takenPiece = GetPiece(fColorToMove == Color::White ? Color::Black : Color::White, attack);
        fLegalMoves.push_back(Move{queen, attack, Piece::Queen, takenPiece});
    }
}

void Board::FillPseudoBishopMoves(U64 ownPieces, U64 otherPieces) {
    U64 occupancy = ownPieces | otherPieces;
    U64 bishops = GetBoard(fColorToMove, Piece::Bishop);
    while(bishops) {
        U64 bishop = 0;
        set_bit(bishop, pop_LSB(bishops));

        // TODO: maybe given a bishop position have 2 lookup tables for positive and negative ray
        // Vertical distance from the primary diagonal
        int dPrimaryDiag = get_file_number(bishop) - 9 + get_rank_number(bishop);
        // Vertical distance from the secondary diagonal
        int dSecondaryDiag = get_rank_number(bishop) - get_file_number(bishop);

        U64 mask1 = 0;
        if(dPrimaryDiag == 0) {
            mask1 = PRIMARY_DIAGONAL;
        } else {
            mask1 = dPrimaryDiag > 0 ? PRIMARY_DIAGONAL << (abs(dPrimaryDiag) * 8) : PRIMARY_DIAGONAL >> (abs(dPrimaryDiag) * 8);
        }

        U64 mask2 = 0;
        if(dSecondaryDiag == 0) {
            mask2 = SECONDARY_DIAGONAL;
        } else {
            mask2 = dSecondaryDiag > 0 ? SECONDARY_DIAGONAL << (abs(dSecondaryDiag) * 8) : SECONDARY_DIAGONAL >> (abs(dSecondaryDiag) * 8);
        }

        U64 attacks = (hypQuint(bishop, occupancy, mask1) | hypQuint(bishop, occupancy, mask2)) & ~ownPieces;
        while(attacks) {
            U64 attack = 0;
            set_bit(attack, pop_LSB(attacks));
            Piece takenPiece = Piece::Null;
            if(attack & otherPieces) // Find the type of piece that is getting removed
                takenPiece = GetPiece(fColorToMove == Color::White ? Color::Black : Color::White, attack);
            fLegalMoves.push_back(Move{bishop, attack, Piece::Bishop, takenPiece});
        }
    }
}

void Board::FillPseudoRookMoves(U64 ownPieces, U64 otherPieces) {
    U64 occupancy = ownPieces | otherPieces;
    U64 rooks = GetBoard(fColorToMove, Piece::Rook);
    while(rooks) {
        U64 rook = 0;
        set_bit(rook, pop_LSB(rooks));
        U64 attacks = (hypQuint(rook, occupancy, get_rank(rook)) | hypQuint(rook, occupancy, get_file(rook))) & ~ownPieces;
        while(attacks) {
            U64 attack = 0;
            set_bit(attack, pop_LSB(attacks));
            Piece takenPiece = Piece::Null;
            if(attack & otherPieces) // Find the type of piece that is getting removed
                takenPiece = GetPiece(fColorToMove == Color::White ? Color::Black : Color::White, attack);
            fLegalMoves.push_back(Move{rook, attack, Piece::Rook, takenPiece});
        }
    }
}

void Board::FillPseudoKingMoves(U64 ownPieces, U64 otherPieces) {
    U64 king = GetBoard(fColorToMove, Piece::King);
    U64 attacks = fKingAttacks[get_LSB(king)] & ~ownPieces;
    while(attacks) {
        U64 attack = 0;
        set_bit(attack, pop_LSB(attacks));
        Piece takenPiece = Piece::Null;
        if(attack & otherPieces)
            takenPiece = GetPiece(fColorToMove == Color::White ? Color::Black : Color::White, attack);
        fLegalMoves.push_back(Move{king, attack, Piece::King, takenPiece});
    }
}

void Board::FillPseudoKnightMoves(U64 ownPieces, U64 otherPieces) {
    U64 knights = GetBoard(fColorToMove, Piece::Knight);
    while(knights) {
        U64 knight = 0;
        set_bit(knight, pop_LSB(knights));
        U64 attacks = fKnightAttacks[get_LSB(knight)] & ~ownPieces;
        while(attacks) {
            U64 attack = 0;
            set_bit(attack, pop_LSB(attacks));
            Piece takenPiece = Piece::Null;
            if(attack & otherPieces) // Find the type of piece that is getting removed
                takenPiece = GetPiece(fColorToMove == Color::White ? Color::Black : Color::White, attack);
            fLegalMoves.push_back(Move{knight, attack, Piece::Knight, takenPiece});
        }
    }
}

void Board::UndoMove(int nMoves) {
    // undoes the last nMoves from the board
    if(nMoves > fMadeMoves.size())
        Reset(); // Cannot undo more moves than exist in the move tree
    // Last move in the stack will be from player of opposing colour
    Color movingColor = fColorToMove == Color::White ? Color::Black : Color::White;
    for(int i = nMoves; i > 0; i--) {
        Move m = fMadeMoves.back();
        U64 movedPieceBoard = GetBoard(movingColor, m.piece);
        clear_bit(movedPieceBoard, get_LSB(m.target)); // clear target bit on the relevant board
        set_bit(movedPieceBoard, get_LSB(m.origin)); // set the origin bit on relevant board
        SetBitBoard(movingColor, m.piece, movedPieceBoard);
        if(m.takenPiece != Piece::Null) {
            Color otherColor = movingColor == Color::White ? Color::Black : Color::White;
            U64 b = GetBoard(otherColor, m.takenPiece);
            if(m.WasEnPassant) {
                std::cout << "Wayyy i'm en passant\n";
                // special case old bit-board already re-instated need to put piece back in correct place now
                // crossing of the origin RANK and target FILE = taken piece position 
                set_bit(b, get_LSB(get_rank(m.origin) & get_file(m.target)));

            } else {
                set_bit(b, get_LSB(m.target));
            }
            SetBitBoard(otherColor, m.takenPiece, b);
        }
        // TODO: king is in check etc
        fMadeMoves.pop_back();
        movingColor = movingColor == Color::White ? Color::Black : Color::White;
    }
}

void Board::MakeMove(Move move) {
    // 0. Get appropriate boards invovled
    // 1. Check move is legal
    // 2. make the move (update bitbsoards)
    // 3. if it was a special move e.g. castling update variables
    // 4. update who is to move
    U64* originBoard = GetBoard(fColorToMove, move.origin);

    if(move.takenPiece != Piece::Null) {
        Color otherColor = fColorToMove == Color::White ? Color::Black : Color::White;
        U64 targetBoard = GetBoard(otherColor, move.takenPiece);
        if(move.WasEnPassant) {
            clear_bit(targetBoard, get_LSB(get_rank(move.origin) & get_file(move.target)));
        } else {
            clear_bit(targetBoard, get_LSB(move.target));
        }
        SetBitBoard(otherColor, move.takenPiece, targetBoard);
    }

    // Remove piece from the starting position
    clear_bit(*originBoard, get_LSB(move.origin));

    // Set the piece at the new position
    set_bit(*originBoard, get_LSB(move.target)); 

    fColorToMove = fColorToMove == Color::White ? Color::Black : Color::White;
    fMadeMoves.push_back(move);
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

U64 Board::GetBoard(Color color) {
    U64 board = 0;
    int start = color == Color::Black ? 6 : 0;
    for(Piece p : PIECES) {
        board |= GetBoard(color, p);
    }
    return board;
}

U64 Board::GetBoard(Color color, Piece piece) {
    if(color == Color::White)
        return fBoards[(int)piece];
    return fBoards[(int)piece + 6];
}

void Board::SetBitBoard(Color color, Piece piece, U64 board) {
    if(color == Color::White) {
        fBoards[(int)piece] = board;
    } else {
        fBoards[(int)piece + 6] = board;
    }
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