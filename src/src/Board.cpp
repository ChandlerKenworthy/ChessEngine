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

    fUnique = 0;
    fGameState = State::Play;
    fWhiteKingMoved = false;
    fBlackKingMoved = false;
    fWhiteKingsideRookMoved = false;
    fWhiteQueensideRookMoved = false;
    fBlackKingsideRookMoved = false;
    fBlackQueensideRookMoved = false;
    fWasLoadedFromFEN = false;
    fColorToMove = Color::White;

    fLegalMoves.clear();
    fMadeMoves.clear();
}

Piece Board::GetPiece(Color color, U64 pos) {
    // Returns the type of piece, if any, at the specified position with the color given
    int adj = 0;
    if(color == Color::Black)
        adj = 6;
    for(Piece p : PIECES) {
        if(GetBoard(color, p) & pos)
            return p;
    }
    return Piece::Null;
}

void Board::UndoMove() {
    // undoes the last nMoves from the board
    if(fMadeMoves.size() < 1)
        Reset(); // Cannot undo more moves than exist in the move tree
    // Last move in the stack will be from player of opposing colour
    Color movingColor = fColorToMove == Color::White ? Color::Black : Color::White;
    Move m = fMadeMoves.back();
    U64 movedPieceBoard = GetBoard(movingColor, m.piece);
    clear_bit(movedPieceBoard, get_LSB(m.target)); // clear target bit on the relevant board
    set_bit(movedPieceBoard, get_LSB(m.origin)); // set the origin bit on relevant board
    SetBoard(movingColor, m.piece, movedPieceBoard);
    if(m.takenPiece != Piece::Null) {
        Color otherColor = movingColor == Color::White ? Color::Black : Color::White;
        U64 b = GetBoard(otherColor, m.takenPiece);
        if(m.WasEnPassant) {
            // special case old bit-board already re-instated need to put piece back in correct place now
            // crossing of the origin RANK and target FILE = taken piece position 
            set_bit(b, get_LSB(get_rank(m.origin) & get_file(m.target)));

        } else {
            set_bit(b, get_LSB(m.target));
        }
        SetBoard(otherColor, m.takenPiece, b);
        // TODO: king is in check etc
        fMadeMoves.pop_back();
        movingColor = movingColor == Color::White ? Color::Black : Color::White;
    }
    fColorToMove = movingColor == Color::White ? Color::Black : Color::White;
}

void Board::MakeMove(Move *move) {
    // Change position of moved piece
    // Take away any taken pieces
    // Move rook if castling was involved
    // Update king/rook has moved if either of these were moved
    // Update made moves and unique counters

    U64 *origin = GetBoardPointer(fColorToMove, move->piece);
    
    // Remove piece from the starting position
    clear_bit(*origin, get_LSB(move->origin));

    // Set the piece at the new position
    set_bit(*origin, get_LSB(move->target));

    // Handle pieces being taken
    if(move->takenPiece != Piece::Null) {
        U64 *targ = GetBoardPointer(fColorToMove == Color::White ? Color::Black : Color::White, move->takenPiece);
        // Check, move could be en-passant
        if(move->WasEnPassant) {
            clear_bit(*targ, get_LSB(get_rank(move->origin) & get_file(move->target)));
        } else {
            clear_bit(*targ, get_LSB(move->target));
        }
    }

    if(move->WasCastling) { // Need to move the rook as well
        std::cout << "Detected castling move\n";
        U64 *rook = GetBoardPointer(fColorToMove, Piece::Rook);
        // See where the origin was (that tells us which rook needs moving and to where)
        if(move->target & RANK_1 & FILE_G) { // Kingside white castling (rook h1 -> f1)
            clear_bit(*rook, get_LSB(RANK_1 & FILE_H));
            set_bit(*rook, get_LSB(RANK_1 & FILE_F));
            fWhiteKingsideRookMoved = true;
        } else if(move->target & RANK_1 & FILE_C) {  // Queenside white castling (rook a1 -> d1)
            clear_bit(*rook, get_LSB(RANK_1 & FILE_A));
            set_bit(*rook, get_LSB(RANK_1 & FILE_D));
            fWhiteQueensideRookMoved = true;
        } else if(move->target & RANK_8 & FILE_G) { // Kingside black castling
            clear_bit(*rook, get_LSB(RANK_8 & FILE_H));
            set_bit(*rook, get_LSB(RANK_8 & FILE_F));
            fBlackKingsideRookMoved = true;
        } else if(move->target & RANK_8 & FILE_C) { // Queenside black castling
            clear_bit(*rook, get_LSB(RANK_8 & FILE_A));
            set_bit(*rook, get_LSB(RANK_8 & FILE_D));
            fBlackQueensideRookMoved = true;
        }
        if(fColorToMove == Color::White) {
            fWhiteKingMoved = true;
        } else {
            fBlackKingMoved = true;
        }
    }

    if(move->piece == Piece::King) {
        if(fColorToMove == Color::White) {
            fWhiteKingMoved = true;
        } else {
            fBlackKingMoved = true;
        }
    }

    if(move->piece == Piece::Rook) { // TODO: Implement this properly
        if(fColorToMove == Color::White) {
            if(move->origin & FILE_A) {
                fWhiteQueensideRookMoved = true;
            } else if(move->origin & FILE_H) {
                fWhiteKingsideRookMoved = true;
            }
        } else {
            if(move->origin & FILE_A) {
                fBlackQueensideRookMoved = true;
            } else if(move->origin & FILE_H) {
                fBlackKingsideRookMoved = true;
            }
        }
    }

    fColorToMove = fColorToMove == Color::White ? Color::Black : Color::White;
    fMadeMoves.push_back(*move);
    fUnique++;

    // TODO: Check if game is over or not
}

U64 Board::GetBoard(Color color, U64 occupiedPosition) {
    int adj = 0;
    if(color == Color::Black)
        adj = 6;
    for(Piece p : PIECES) {
        if(fBoards[(int)p + adj] & occupiedPosition)
            return fBoards[(int)p + adj];
    }
    return U64{0};
}

U64 Board::GetBoard(Color color) {
    U64 board = 0;
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

U64* Board::GetBoardPointer(Color color, Piece piece) {
    if(color == Color::White)
        return &fBoards[(int)piece];
    return &fBoards[(int)piece + 6];
}

void Board::SetBoard(Color color, Piece piece, U64 board) {
    if(color == Color::White) {
        fBoards[(int)piece] = board;
    } else {
        fBoards[(int)piece + 6] = board;
    }
}

void Board::EmptyBoards() {
    U64 empty = 0;
    for(int i = 0; i < 12; i++) {
        fBoards[i] = empty;
    }
}

void Board::LoadFEN(const std::string &fen) {
    Reset();
    EmptyBoards();

    int rank = 8;
    int file = 1;
    int ngaps = 0;

    bool whiteCanCastle = false;
    bool blackCanCastle = false;

    for (char c : fen) {
        if (file > 8) {
            file = 1;
            rank--;
        }

        if(isdigit(c)) {
            file += c - '0';
        } else if(isalpha(c)) {
            if (ngaps == 1) {
                fColorToMove = (toupper(c) == 'B') ? Color::Black : Color::White;
            } else if (ngaps == 2) {
                whiteCanCastle |= (c == 'K' || c == 'Q');
                blackCanCastle |= (c == 'k' || c == 'q');
            } else {
                U64 pos = get_rank_from_number(rank) & get_file_from_number(file);
                Color pieceColor = (isupper(c)) ? Color::White : Color::Black;
                U64 board = GetBoard(pieceColor, GetPieceFromChar(c)) | pos;
                SetBoard(pieceColor, GetPieceFromChar(c), board);
                file++;
            }
        } else if (c == ' ') {
            ngaps++;
        }
    }

    //fWhiteHasCastled = !whiteCanCastle;
    //fBlackHasCastled = !blackCanCastle;
    fWasLoadedFromFEN = true;
}

std::pair<Color, Piece> Board::GetIsOccupied(U64 pos) {
    for(int iBoard = 0; iBoard < 12; iBoard++) {
        if(pos & fBoards[iBoard]) {
            Piece pieceType = Piece::Null;
            int x = iBoard;
            if(iBoard >= 6)
                x = iBoard - 6;

            if(x == 0) {
                pieceType = Piece::Pawn;
            } else if(x == 1) {
                pieceType = Piece::Bishop;
            } else if(x == 2) {
                pieceType = Piece::Knight;
            } else if(x == 3) {
                pieceType = Piece::Rook;
            } else if(x == 4) {
                pieceType = Piece::Queen;
            } else if(x == 5) {
                pieceType = Piece::King;
            }
            return std::make_pair(iBoard < 6 ? Color::White : Color::Black, pieceType);
        }
    }
    return std::make_pair(Color::White, Piece::Null);
}