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
    fHalfMoves = 0;
    fGameState = State::Play;
    fWhiteKingMoved = 0;
    fBlackKingMoved = 0;
    fWhiteKingsideRookMoved = 0;
    fWhiteQueensideRookMoved = 0;
    fBlackKingsideRookMoved = 0;
    fBlackQueensideRookMoved = 0;
    fWasLoadedFromFEN = false;
    fEnPassantFENTarget = 0;
    fColorToMove = Color::White;

    fLegalMoves.clear();
    fMadeMoves.clear();
}

void Board::UndoMove() {
    // Essentially just does the inverse of MakeMove
    if(fMadeMoves.size() < 1)
        Reset(); // Cannot undo more moves than exist in the move tree
    // Last move in the stack will be from player of opposing colour

    const Color movingColor = fColorToMove == Color::White ? Color::Black : Color::White;
    U32 move = fMadeMoves.back();

    const Piece movedPiece = GetMovePiece(move);
    const Piece takenPiece = GetMoveTakenPiece(move);
    const U64 start = GetMoveOrigin(move);
    const U64 target = GetMoveTarget(move);
    const uint8_t targetLSB = get_LSB(target);

    U64 *origin = GetBoardPointer(movingColor, movedPiece);
    
    // Set piece back at the starting position
    set_bit(*origin, get_LSB(start));

    // Clear the piece at the target position
    clear_bit(*origin, targetLSB);

    // Handle pieces being taken
    if(takenPiece != Piece::Null) {
        U64 *targ = GetBoardPointer(fColorToMove, takenPiece);
        // Check, move could be en-passant
        if(GetMoveIsEnPassant(move)) {
            // special case old bit-board already re-instated need to put piece back in correct place now
            // crossing of the origin RANK and target FILE = taken piece position 
            set_bit(*targ, get_LSB(get_rank(start) & get_file(target)));
        } else {
            set_bit(*targ, targetLSB); // Put the piece back
            if(takenPiece == Piece::Rook) { // Rook being taken counts as a move for the rook if not moved before
                if(target & SQUARE_H1) {
                    fWhiteKingsideRookMoved--;
                } else if(target & SQUARE_A1) {
                    fWhiteQueensideRookMoved--;
                } else if(target & SQUARE_H8) {
                    fBlackKingsideRookMoved--;
                } else if(target & SQUARE_A8) {
                    fBlackQueensideRookMoved--;
                }
            }
        }
    } else if(GetMoveIsCastling(move)) { // Need to move the rook as well
        U64 *rook = GetBoardPointer(movingColor, Piece::Rook);
        U64 targRank = movingColor == Color::White ? RANK_1 : RANK_8;
        // See where the origin was (that tells us which rook needs moving and to where)
        if(target & FILE_G) { // Kingside white castling (rook h1 -> f1)
            set_bit(*rook, get_LSB(targRank & FILE_H));
            clear_bit(*rook, get_LSB(targRank & FILE_F));
            movingColor == Color::White ? fWhiteKingsideRookMoved-- : fBlackKingsideRookMoved--;          
        // Don't need to check rank, implicitly done by GetMoveIsCastling(move)
        } else if(target & FILE_C) {  // Queenside white castling (rook a1 -> d1)
            set_bit(*rook, get_LSB(targRank & FILE_A));
            clear_bit(*rook, get_LSB(targRank & FILE_D));
            movingColor == Color::White ? fWhiteQueensideRookMoved-- : fBlackQueensideRookMoved--;
        }
    }

    if(movedPiece == Piece::King)
        movingColor == Color::White ? fWhiteKingMoved-- : fBlackKingMoved--;

    if(movedPiece == Piece::Rook) {
        if(movingColor == Color::White) {
            if(start & SQUARE_A1) {
                fWhiteQueensideRookMoved--;
            } else if(start & SQUARE_H1) {
                fWhiteKingsideRookMoved--;
            }
        } else {
            if(start & SQUARE_A8) {
                fBlackQueensideRookMoved--;
            } else if(start & SQUARE_H8) {
                fBlackKingsideRookMoved--;
            }
        }
    }

    if(GetMoveIsPromotion(move)) {
        // Clear bit from promotional bitboard
        U64 *promotionBoard = GetBoardPointer(movingColor, GetMovePromotionPiece(move));
        clear_bit(*promotionBoard, targetLSB);
    }

    fColorToMove = movingColor;
    fMadeMoves.pop_back();
    fUnique--;
}

void Board::MakeMove(const U32 move) {
    if(fGameState != State::Play)
        return;

    const Piece movedPiece = GetMovePiece(move);
    const Piece takenPiece = GetMoveTakenPiece(move);
    const U64 start = GetMoveOrigin(move);
    const U64 target = GetMoveTarget(move);
    const uint8_t targetLSB = get_LSB(target);
    U64 *origin = GetBoardPointer(fColorToMove, movedPiece);
    
    // Remove piece from the starting position
    clear_bit(*origin, get_LSB(start));

    // Set the piece at the new position
    set_bit(*origin, targetLSB);

    // Handle pieces being taken
    if(takenPiece != Piece::Null) {
        U64 *targ = GetBoardPointer(fColorToMove == Color::White ? Color::Black : Color::White, takenPiece);
        // Check, move could be en-passant
        if(GetMoveIsEnPassant(move)) {
            clear_bit(*targ, get_LSB(get_rank(start) & get_file(target)));
        } else {
            clear_bit(*targ, targetLSB);
            if(takenPiece == Piece::Rook) { // Taking the rook counts as it "moving" so no castling available
                if(target & SQUARE_H1) {
                    fWhiteKingsideRookMoved++;
                } else if(target & SQUARE_A1) {
                    fWhiteQueensideRookMoved++;
                } else if(target & SQUARE_H8) {
                    fBlackKingsideRookMoved++;
                } else if(target & SQUARE_A8) {
                    fBlackQueensideRookMoved++;
                }
            }
        }
    } else if(GetMoveIsCastling(move)) { // Need to move the rook as well
        U64 *rook = GetBoardPointer(fColorToMove, Piece::Rook);
        // See where the origin was (that tells us which rook needs moving and to where)
        if(target & SQUARE_G1) { // Kingside white castling (rook h1 -> f1)
            clear_bit(*rook, get_LSB(SQUARE_H1));
            set_bit(*rook, get_LSB(SQUARE_F1));
            fWhiteKingsideRookMoved++;
        } else if(target & SQUARE_C1) {  // Queenside white castling (rook a1 -> d1)
            clear_bit(*rook, get_LSB(SQUARE_A1));
            set_bit(*rook, get_LSB(SQUARE_D1));
            fWhiteQueensideRookMoved++;
        } else if(target & SQUARE_G8) { // Kingside black castling
            clear_bit(*rook, get_LSB(SQUARE_H8));
            set_bit(*rook, get_LSB(SQUARE_F8));
            fBlackKingsideRookMoved++;
        } else if(target & SQUARE_C8) { // Queenside black castling
            clear_bit(*rook, get_LSB(SQUARE_A8));
            set_bit(*rook, get_LSB(SQUARE_D8));
            fBlackQueensideRookMoved++;
        }
    }

    if(movedPiece == Piece::King)
        fColorToMove == Color::White ? fWhiteKingMoved++ : fBlackKingMoved++;

    if(movedPiece == Piece::Rook) {
        if(fColorToMove == Color::White) {
            if(start & SQUARE_A1) {
                fWhiteQueensideRookMoved++;
            } else if(start & SQUARE_H1) {
                fWhiteKingsideRookMoved++;
            }
        } else {
            if(start & SQUARE_A8) {
                fBlackQueensideRookMoved++;
            } else if(start & SQUARE_H8) {
                fBlackKingsideRookMoved++;
            }
        }
    }

    if(GetMoveIsPromotion(move)) {
        clear_bit(*origin, targetLSB); // Undo the setting that already happened
        U64 *targBoard = GetBoardPointer(fColorToMove, GetMovePromotionPiece(move) == Piece::Null ? Piece::Queen : GetMovePromotionPiece(move));
        set_bit(*targBoard, targetLSB);
    }

    fHalfMoves++;
    if(movedPiece == Piece::Pawn || takenPiece != Piece::Null)
        fHalfMoves = 0;

    fColorToMove = fColorToMove == Color::White ? Color::Black : Color::White;
    fMadeMoves.push_back(move);
    fUnique++;
}

U64 Board::GetBoard(const Color color, const U64 occupiedPosition) {
    uint8_t offset = color == Color::White ? -1 : 5;
    for(Piece p : PIECES) {
        if(fBoards[(int)p + offset] & occupiedPosition)
            return fBoards[(int)p + offset];
    }
    return U64{0};
}

U64 Board::GetBoard(const Color color) {
    uint8_t startOffset = color == Color::White ? 0 : 6;
    uint8_t endOffset = startOffset + 6;
    return std::accumulate(fBoards + startOffset, fBoards + endOffset, 0ULL, std::bit_or<U64>());
}

U64 Board::GetBoard(const Color color, const Piece piece) {
    return color == Color::White ? fBoards[(int)piece - 1] : fBoards[(int)piece + 5];
}

U64* Board::GetBoardPointer(const Color color, const Piece piece) {
    return color == Color::White ? &fBoards[(int)piece - 1] : &fBoards[(int)piece + 5];
}

void Board::SetBoard(const Color color, const Piece piece, const U64 board) {
    if(color == Color::White) {
        fBoards[(int)piece - 1] = board;
    } else {
        fBoards[(int)piece + 5] = board;
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

    // Assume castling not possible
    bool whiteKingCanCastle = false;
    bool blackKingCanCastle = false;

    bool whiteKingsideRookMoved = true;
    bool whiteQueensideRookMoved = true;
    bool blackKingsideRookMoved = true;
    bool blackQueensideRookMoved = true;

    int iChar = 0;
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
            } else if (ngaps == 2) { // Castling terms
                whiteKingCanCastle |= (c == 'K' || c == 'Q');
                blackKingCanCastle |= (c == 'k' || c == 'q');
            } else if(ngaps == 3 && !fEnPassantFENTarget) { // En-passant possibilities
                int rankNo = fen.at(iChar + 1) - '0';
                fEnPassantFENTarget = get_file_from_char(c) & get_rank_from_number(rankNo);
            } else if(ngaps == 4) {
                char nextChar = fen.at(iChar + 1);
                if(nextChar == ' ') {
                    fHalfMoves = c - '0';
                } else {
                    // Join two chars into a string using std::stringstream
                    std::stringstream ss;
                    ss << c << nextChar;
                    std::string result = ss.str();
                    fHalfMoves = std::stoi(result);
                }
                
             } else {
                U64 pos = get_rank_from_number(rank) & get_file_from_number(file);
                Color pieceColor = (isupper(c)) ? Color::White : Color::Black;
                Piece pieceType = GetPieceFromChar(c);
                U64 board = GetBoard(pieceColor, pieceType) | pos;
                if(pieceColor == Color::White && pieceType == Piece::Rook) {
                    if(pos & RANK_1 & FILE_A) {
                        whiteQueensideRookMoved = false;
                    } else if(pos & RANK_1 & FILE_H) {
                        whiteKingsideRookMoved = false;
                    }
                } else if(pieceColor == Color::Black && pieceType == Piece::Rook) {
                    if(pos & RANK_8 & FILE_A) {
                        blackQueensideRookMoved = false;
                    } else if(pos & RANK_8 & FILE_H) {
                        blackKingsideRookMoved = false;
                    }
                }
                SetBoard(pieceColor, pieceType, board);
                file++;
            }
        } else if (c == ' ') {
            ngaps++;
        }
        iChar++;
    }

    fWasLoadedFromFEN = true;
    fWhiteKingMoved = !whiteKingCanCastle;
    fBlackKingMoved = !blackKingCanCastle;
    fWhiteKingsideRookMoved = whiteKingsideRookMoved;
    fWhiteQueensideRookMoved = whiteQueensideRookMoved;
    fBlackKingsideRookMoved = blackKingsideRookMoved;
    fBlackQueensideRookMoved = blackQueensideRookMoved;
}

std::pair<Color, Piece> Board::GetIsOccupied(const U64 pos) {
    for (int iBoard = 0; iBoard < 12; iBoard++) {
        if(pos & fBoards[iBoard]) {
            // We already know the mapping e.g. 0 = pawn, knight, bishop, rook, queen, king (white, black)
            uint8_t pieceIndex = iBoard >= 6 ? iBoard - 5 : iBoard + 1; // Must fall in range 1--6 (inclusive)
            Piece pieceType = static_cast<Piece>(pieceIndex);
            return std::make_pair(iBoard < 6 ? Color::White : Color::Black, pieceType);
        }
    }
    return std::make_pair(Color::White, Piece::Null);
}

std::pair<Color, Piece> Board::GetIsOccupied(const U64 pos, const Color color) {
    // Similar to GetIsOccupied(pos) but only searches the boards of the provided color
    uint8_t offset = color == Color::White ? 0 : 6;
    for (int iBoard = offset; iBoard < offset + 6; iBoard++) {
        if(pos & fBoards[iBoard]) {
            uint8_t pieceIndex = iBoard >= 6 ? iBoard - 5 : iBoard + 1; // Must fall in range 1--6 (inclusive)
            Piece pieceType = static_cast<Piece>(pieceIndex);
            return std::make_pair(color, pieceType);
        }
    }
    return std::make_pair(color, Piece::Null);
}

void Board::PrintDetailedMove(U32 move) {
    U64 origin = GetMoveOrigin(move);
    U64 target = GetMoveTarget(move);
    Piece piece = GetMovePiece(move);
    Piece takenPiece = GetMoveTakenPiece(move);
    char pieceChar = GetPieceChar(piece);
    
    int rank = get_rank_number(target);
    int file = get_file_number(target);
    char fileChar = get_file_char(file);

    std::string moveStr = "";
    if(piece != Piece::Pawn)
        moveStr += pieceChar;

    moveStr += fileChar;
    moveStr += std::to_string(rank);

    if(takenPiece != Piece::Null)
        moveStr += "x";

    // if(checking the king)
    // add a "+"

    std::cout << moveStr << "\n";
}