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
    Color movingColor = fColorToMove == Color::White ? Color::Black : Color::White;
    U32 move = fMadeMoves.back();

    U64 *origin = GetBoardPointer(movingColor, GetMovePiece(move));
    
    // Set piece back at the starting position
    set_bit(*origin, get_LSB(GetMoveOrigin(move)));

    // Clear the piece at the target position
    clear_bit(*origin, get_LSB(GetMoveTarget(move)));

    // Handle pieces being taken
    if(GetMoveTakenPiece(move) != Piece::Null) {
        U64 *targ = GetBoardPointer(fColorToMove, GetMoveTakenPiece(move));
        // Check, move could be en-passant
        if(GetMoveIsEnPassant(move)) {
            // special case old bit-board already re-instated need to put piece back in correct place now
            // crossing of the origin RANK and target FILE = taken piece position 
            set_bit(*targ, get_LSB(get_rank(GetMoveOrigin(move)) & get_file(GetMoveTarget(move))));
        } else {
            set_bit(*targ, get_LSB(GetMoveTarget(move))); // Put the piece back
            if(GetMoveTakenPiece(move) == Piece::Rook) { // Rook being taken counts as a move for the rook if not moved before
                if(GetMoveTarget(move) & RANK_1 & FILE_H) {
                    fWhiteKingsideRookMoved--;
                } else if(GetMoveTarget(move) & RANK_1 & FILE_A) {
                    fWhiteQueensideRookMoved--;
                } else if(GetMoveTarget(move) & RANK_8 & FILE_H) {
                    fBlackKingsideRookMoved--;
                } else if(GetMoveTarget(move) & RANK_8 & FILE_A) {
                    fBlackQueensideRookMoved--;
                }
            }
        }
    } else if(GetMoveIsCastling(move)) { // Need to move the rook as well
        U64 *rook = GetBoardPointer(movingColor, Piece::Rook);
        U64 targRank = movingColor == Color::White ? RANK_1 : RANK_8;
        // See where the origin was (that tells us which rook needs moving and to where)
        if(GetMoveTarget(move) & FILE_G) { // Kingside white castling (rook h1 -> f1)
            set_bit(*rook, get_LSB(targRank & FILE_H));
            clear_bit(*rook, get_LSB(targRank & FILE_F));
            if(movingColor == Color::White) {
                fWhiteKingsideRookMoved--;
            } else {
                fBlackKingsideRookMoved--;
            }            
        // Don't need to check rank, implicitly done by GetMoveIsCastling(move)
        } else if(GetMoveTarget(move) & FILE_C) {  // Queenside white castling (rook a1 -> d1)
            set_bit(*rook, get_LSB(targRank & FILE_A));
            clear_bit(*rook, get_LSB(targRank & FILE_D));
            if(movingColor == Color::White) {
                fWhiteQueensideRookMoved--;
            } else {
                fBlackQueensideRookMoved--;
            }    
        }
    }

    if(GetMovePiece(move) == Piece::King) {
        if(movingColor == Color::White) {
            fWhiteKingMoved--;
        } else {
            fBlackKingMoved--;
        }
    }

    if(GetMovePiece(move) == Piece::Rook) {
        if(movingColor == Color::White) {
            if(GetMoveOrigin(move) & FILE_A & RANK_1) {
                fWhiteQueensideRookMoved--;
            } else if(GetMoveOrigin(move) & FILE_H & RANK_1) {
                fWhiteKingsideRookMoved--;
            }
        } else {
            if(GetMoveOrigin(move) & FILE_A & RANK_8) {
                fBlackQueensideRookMoved--;
            } else if(GetMoveOrigin(move) & FILE_H & RANK_8) {
                fBlackKingsideRookMoved--;
            }
        }
    }

    if(GetMoveIsPromotion(move)) {
        // Clear bit from promotional bitboard
        U64 *promotionBoard = GetBoardPointer(movingColor, GetMovePromotionPiece(move));
        clear_bit(*promotionBoard, get_LSB(GetMoveTarget(move)));
    }

    fColorToMove = movingColor;
    fMadeMoves.pop_back();
    fUnique--;
}

void Board::MakeMove(const U32 move) {
    if(fGameState != State::Play)
        return;

    U64 *origin = GetBoardPointer(fColorToMove, GetMovePiece(move));
    
    // Remove piece from the starting position
    clear_bit(*origin, get_LSB(GetMoveOrigin(move)));

    // Set the piece at the new position
    set_bit(*origin, get_LSB(GetMoveTarget(move)));

    // Handle pieces being taken
    if(GetMoveTakenPiece(move) != Piece::Null) {
        U64 *targ = GetBoardPointer(fColorToMove == Color::White ? Color::Black : Color::White, GetMoveTakenPiece(move));
        // Check, move could be en-passant
        if(GetMoveIsEnPassant(move)) {
            clear_bit(*targ, get_LSB(get_rank(GetMoveOrigin(move)) & get_file(GetMoveTarget(move))));
        } else {
            clear_bit(*targ, get_LSB(GetMoveTarget(move)));
            if(GetMoveTakenPiece(move) == Piece::Rook) {
                if(GetMoveTarget(move) & RANK_1 & FILE_H) {
                    fWhiteKingsideRookMoved++;
                } else if(GetMoveTarget(move) & RANK_1 & FILE_A) {
                    fWhiteQueensideRookMoved++;
                } else if(GetMoveTarget(move) & RANK_8 & FILE_H) {
                    fBlackKingsideRookMoved++;
                } else if(GetMoveTarget(move) & RANK_8 & FILE_A) {
                    fBlackQueensideRookMoved++;
                }
            }
        }
    } else if(GetMoveIsCastling(move)) { // Need to move the rook as well
        U64 *rook = GetBoardPointer(fColorToMove, Piece::Rook);
        // See where the origin was (that tells us which rook needs moving and to where)
        if(GetMoveTarget(move) & RANK_1 & FILE_G) { // Kingside white castling (rook h1 -> f1)
            clear_bit(*rook, get_LSB(RANK_1 & FILE_H));
            set_bit(*rook, get_LSB(RANK_1 & FILE_F));
            fWhiteKingsideRookMoved++;
        } else if(GetMoveTarget(move) & RANK_1 & FILE_C) {  // Queenside white castling (rook a1 -> d1)
            clear_bit(*rook, get_LSB(RANK_1 & FILE_A));
            set_bit(*rook, get_LSB(RANK_1 & FILE_D));
            fWhiteQueensideRookMoved++;
        } else if(GetMoveTarget(move) & RANK_8 & FILE_G) { // Kingside black castling
            clear_bit(*rook, get_LSB(RANK_8 & FILE_H));
            set_bit(*rook, get_LSB(RANK_8 & FILE_F));
            fBlackKingsideRookMoved++;
        } else if(GetMoveTarget(move) & RANK_8 & FILE_C) { // Queenside black castling
            clear_bit(*rook, get_LSB(RANK_8 & FILE_A));
            set_bit(*rook, get_LSB(RANK_8 & FILE_D));
            fBlackQueensideRookMoved++;
        }
    }

    if(GetMovePiece(move) == Piece::King) {
        if(fColorToMove == Color::White) {
            fWhiteKingMoved++;
        } else {
            fBlackKingMoved++;
        }
    }

    if(GetMovePiece(move) == Piece::Rook) {
        if(fColorToMove == Color::White) {
            if(GetMoveOrigin(move) & FILE_A & RANK_1) {
                fWhiteQueensideRookMoved++;
            } else if(GetMoveOrigin(move) & FILE_H & RANK_1) {
                fWhiteKingsideRookMoved++;
            }
        } else {
            if(GetMoveOrigin(move) & FILE_A & RANK_8) {
                fBlackQueensideRookMoved++;
            } else if(GetMoveOrigin(move) & FILE_H & RANK_8) {
                fBlackKingsideRookMoved++;
            }
        }
    }

    if(GetMoveIsPromotion(move)) {
        clear_bit(*origin, get_LSB(GetMoveTarget(move))); // Undo the setting that already happened
        U64 *targBoard = GetBoardPointer(fColorToMove, GetMovePromotionPiece(move) == Piece::Null ? Piece::Queen : GetMovePromotionPiece(move));
        set_bit(*targBoard, get_LSB(GetMoveTarget(move)));
    }

    fHalfMoves++;
    if(GetMovePiece(move) == Piece::Pawn || GetMoveTakenPiece(move) != Piece::Null)
        fHalfMoves = 0;

    // TODO: Move now "made", see if opposing king is now in check?

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
    uint8_t offset = color == Color::White ? -1 : 5;
    fBoards[(int)piece + offset] = board;
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
    for(int iBoard = 0; iBoard < 12; iBoard++) {
        if(pos & fBoards[iBoard]) {
            // We already know the mapping e.g. pawn, knight, bishop, rook, queen, king (white, black)
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