#include <iostream>

#include "Board.hpp"

Board::Board() : fPawnPhase(0), fKnightPhase(1), fBishopPhase(1), fRookPhase(2), fQueenPhase(4) {
    InitZobristKeys();
    Reset();
    fTotalPhase = fPawnPhase*16 + fKnightPhase*4 + fBishopPhase*4 + fRookPhase*4 + fQueenPhase*2;
}

void Board::InitZobristKeys() {
    for (int i = 0; i < NUM_SQUARES; ++i) {
        for(int j = 0; j < NUM_PIECE_TYPES; ++j) {
            fKeys.pieceKeys[i][j] = GetRandomKey();
        }
    }
    fKeys.sideToMoveKey[0] = GetRandomKey(); // White to move
    fKeys.sideToMoveKey[1] = GetRandomKey(); // Black to move
    for(int i = 0; i < 4; ++i) {
        fKeys.castlingKeys[i] = GetRandomKey();
    }
    fKeys.enPassantKey = GetRandomKey();
}

U64 Board::GetHash() {
    U64 hash = 0;

    // Piece placement
    for(int i = 0; i < NSQUARES; ++i) {
        U64 square = 1ULL << i;
        std::pair<Color, Piece> occupation = GetIsOccupied(square);

        if(occupation.second != Piece::Null) {
            // Piece is in range [1,6], then add zero for white or 6 for black
            hash ^= fKeys.pieceKeys[__builtin_ctzll(square)][(int)occupation.second + (occupation.first == Color::White ? 0 : 6)];
        }
    }
    // Side to move
    hash ^= fKeys.sideToMoveKey[(int)fColorToMove];

    // Castling rights
    bool wKingside = fWhiteKingsideRookMoved == 0;
    bool bKingside = fBlackKingsideRookMoved == 0;
    bool wQueenside = fWhiteQueensideRookMoved == 0;
    bool bQueenside = fBlackQueensideRookMoved == 0;
    if(fWhiteKingMoved == 0) {
        if(wKingside)
            hash ^= fKeys.castlingKeys[0];
        if(wQueenside)
            hash ^= fKeys.castlingKeys[1];
    }
    if(fBlackKingMoved == 0) {
        if(bKingside)
            hash ^= fKeys.castlingKeys[2];
        if(bQueenside)
            hash ^= fKeys.castlingKeys[3];
    }

    // En passant square (i.e. en-passant now available)
    if(fMadeMoves.size() > 0) {
        const U16 lastMove = GetLastMove();
        const U64 target = GetMoveTarget(lastMove);
        const U8 startRank = get_rank_number(GetMoveOrigin(lastMove));
        const U8 endRank = get_rank_number(target);
        bool wasPawnMoved = GetIsOccupied(target).second == Piece::Pawn;
        const bool wasDoubeMove = fColorToMove == Color::White ? (startRank == 7 && endRank == 5) : (startRank == 2 && endRank == 4);

        if(wasPawnMoved && wasDoubeMove) {
            // potential for en-passant if either of the fColorToMove pieces pawns directly adjacent
            U64 pawns = GetBoard(fColorToMove, Piece::Pawn);
            if((east(target) | west(target)) & pawns)
                hash ^= fKeys.enPassantKey;
        }
    }

    return hash;
}

Board::Board(const Board& other) : fPawnPhase(0), fKnightPhase(1), fBishopPhase(1), fRookPhase(2), fQueenPhase(4) {
    // Copy over the bitboards
    for(int iBoard = 0; iBoard < 12; ++iBoard) {
        this->fBoards[iBoard] = other.fBoards[iBoard];
    }

    this->fTotalPhase = other.fTotalPhase;

    // Copy over the game state variables
    this->fUnique = other.fUnique;
    this->fMadeMoves = other.fMadeMoves;
    this->fHalfMoves = other.fHalfMoves;
    this->fGameState = other.fGameState;
    this->fWhiteKingMoved = other.fWhiteKingMoved;
    this->fBlackKingMoved = other.fBlackKingMoved;
    this->fWhiteKingsideRookMoved = other.fWhiteKingsideRookMoved;
    this->fWhiteQueensideRookMoved = other.fWhiteQueensideRookMoved;
    this->fBlackKingsideRookMoved = other.fBlackKingsideRookMoved;
    this->fBlackQueensideRookMoved = other.fBlackQueensideRookMoved;
    this->fEnPassantFENTarget = other.fEnPassantFENTarget;
    this->fColorToMove = other.fColorToMove;

    // Copy over how the board was initalised
    this->fWasLoadedFromFEN = other.fWasLoadedFromFEN;
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

    fMovedPieces = {};
    fTakenPieces = {};
    fMadeMoves = {};
}

Piece Board::GetMovePiece(const U16 move) const {
    const U64 origin = GetMoveOrigin(move);
    return GetIsOccupied(origin).second;
}

Piece Board::GetMoveTakenPiece(const U16 move) const {
    const U64 target = GetMoveTarget(move);
    return GetIsOccupied(target).second;
}

bool Board::GetMoveIsEnPassant(const U16 move, const Piece movedPiece, const bool targetIsNull) const {
    // To be en-passant we must be moving a pawn -- assumes move has not yet happened!
    if(movedPiece != Piece::Pawn)
        return false;
    
    const U64 origin = GetMoveOrigin(move);
    const U64 target = GetMoveTarget(move);

    // Move is diagonally forwards, regardless of colour
    if(!((north_east(origin) | north_west(origin) | south_west(origin) | south_east(origin)) & target))
        return false;

    // TODO: Square we land on must not haveb been occupied by any other piece when the move was made, only true for en-passant
    if(targetIsNull)
        return true;

    return false;
}

void Board::UndoMove() {
    // Essentially just does the inverse of MakeMove
    if(fMadeMoves.size() < 1) {
        Reset(); // Cannot undo more moves than exist in the move tree
        return;
    }

    // Last move in the stack will be from player of opposing colour
    const Color movingColor = fColorToMove == Color::White ? Color::Black : Color::White;
    U16 move = fMadeMoves.back();

    const Piece movedPiece = fMovedPieces.back();
    const Piece takenPiece = fTakenPieces.back();
    const U64 start = GetMoveOrigin(move);
    const U64 target = GetMoveTarget(move);
    const U8 targetLSB = __builtin_ctzll(target);
    U64 *origin = GetBoardPointer(movingColor, movedPiece);
    
    // Set piece back at the starting position
    set_bit(*origin, __builtin_ctzll(start));

    // Clear the piece at the target position
    clear_bit(*origin, targetLSB);

    // Handle en-passant properly
    if(GetMoveIsEnPassant(move, movedPiece, takenPiece == Piece::Null)) {
        // special case old bit-board already re-instated need to put piece back in correct place now
        // crossing of the origin RANK and target FILE = taken piece position 
        set_bit(*GetBoardPointer(fColorToMove, Piece::Pawn), __builtin_ctzll(get_rank(start) & get_file(target)));
    }

    // Handle pieces being taken
    if(takenPiece != Piece::Null) {
        U64 *targ = GetBoardPointer(fColorToMove, takenPiece);
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
    } else if(GetMoveIsCastling(move)) { // Need to move the rook as well
        U64 *rook = GetBoardPointer(movingColor, Piece::Rook);
        U64 targRank = movingColor == Color::White ? RANK_1 : RANK_8;
        // See where the origin was (that tells us which rook needs moving and to where)
        if(target & FILE_G) { // Kingside white castling (rook h1 -> f1)
            set_bit(*rook, __builtin_ctzll(targRank & FILE_H));
            clear_bit(*rook, __builtin_ctzll(targRank & FILE_F));
            movingColor == Color::White ? fWhiteKingsideRookMoved-- : fBlackKingsideRookMoved--;          
        // Don't need to check rank, implicitly done by GetMoveIsCastling(move)
        } else if(target & FILE_C) {  // Queenside white castling (rook a1 -> d1)
            set_bit(*rook, __builtin_ctzll(targRank & FILE_A));
            clear_bit(*rook, __builtin_ctzll(targRank & FILE_D));
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

    fHalfMoves--; // Only reduces if move was not a capture of pawn push
    if(movedPiece == Piece::Pawn || takenPiece != Piece::Null)
        fHalfMoves = 0;

    fMovedPieces.pop_back();
    fTakenPieces.pop_back();
    fGameState = State::Play;
    fColorToMove = movingColor;
    fMadeMoves.pop_back();
    fUnique--;
}

void Board::MakeMove(const U16 move) {
    const Piece movedPiece = GetMovePiece(move);
    const Piece takenPiece = GetMoveTakenPiece(move);
    const U64 start = GetMoveOrigin(move);
    const U64 target = GetMoveTarget(move);
    const U8 targetLSB = __builtin_ctzll(target);
    U64 *origin = GetBoardPointer(fColorToMove, movedPiece);
    
    // Remove piece from the starting position
    clear_bit(*origin, __builtin_ctzll(start));

    // Set the piece at the new position
    set_bit(*origin, targetLSB);

    // Handle en-passant happening (the takenPiece counts as null)
    if(GetMoveIsEnPassant(move, movedPiece, takenPiece == Piece::Null)) {
        clear_bit(*GetBoardPointer(fColorToMove == Color::White ? Color::Black : Color::White, Piece::Pawn), __builtin_ctzll(get_rank(start) & get_file(target)));
    }

    // Handle pieces being taken
    if(takenPiece != Piece::Null) {
        U64 *targ = GetBoardPointer(fColorToMove == Color::White ? Color::Black : Color::White, takenPiece);
        // Check, move could be en-passant
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
    } else if(GetMoveIsCastling(move)) { // Need to move the rook as well
        U64 *rook = GetBoardPointer(fColorToMove, Piece::Rook);
        // See where the origin was (that tells us which rook needs moving and to where)
        if(target & SQUARE_G1) { // Kingside white castling (rook h1 -> f1)
            clear_bit(*rook, __builtin_ctzll(SQUARE_H1));
            set_bit(*rook, __builtin_ctzll(SQUARE_F1));
            fWhiteKingsideRookMoved++;
        } else if(target & SQUARE_C1) {  // Queenside white castling (rook a1 -> d1)
            clear_bit(*rook, __builtin_ctzll(SQUARE_A1));
            set_bit(*rook, __builtin_ctzll(SQUARE_D1));
            fWhiteQueensideRookMoved++;
        } else if(target & SQUARE_G8) { // Kingside black castling
            clear_bit(*rook, __builtin_ctzll(SQUARE_H8));
            set_bit(*rook, __builtin_ctzll(SQUARE_F8));
            fBlackKingsideRookMoved++;
        } else if(target & SQUARE_C8) { // Queenside black castling
            clear_bit(*rook, __builtin_ctzll(SQUARE_A8));
            set_bit(*rook, __builtin_ctzll(SQUARE_D8));
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
    fMovedPieces.push_back(movedPiece);
    fTakenPieces.push_back(takenPiece);
    fUnique++;
}

U64 Board::GetBoard(const Color color, const U64 occupiedPosition) {
    U8 offset = color == Color::White ? -1 : 5;
    for(Piece p : PIECES) {
        if(fBoards[(int)p + offset] & occupiedPosition)
            return fBoards[(int)p + offset];
    }
    return U64{0};
}

U64 Board::GetBoard(const Color color) {
    const U8 startOffset = color == Color::White ? 0 : 6;
    const U8 endOffset = startOffset + 6;
    return std::accumulate(fBoards + startOffset, fBoards + endOffset, 0ULL, std::bit_or<U64>());
}

U64 Board::GetBoard(const Color color, const Piece piece) {
    return color == Color::White ? fBoards[(int)piece - 1] : fBoards[(int)piece + 5];
}

U64 Board::GetBoard(const Piece piece) {
    // Assumes you want the color to move's board for the presiding piece
    return fColorToMove == Color::White ? fBoards[(int)piece - 1] : fBoards[(int)piece + 5];
}

U64* Board::GetBoardPointer(const Color color, const Piece piece) {
    if(piece == Piece::Null)
        return nullptr;
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

std::pair<Color, Piece> Board::GetIsOccupied(const U64 pos) const {
    for (int iBoard = 0; iBoard < 12; iBoard++) {
        if(pos & fBoards[iBoard]) {
            // We already know the mapping e.g. 0 = pawn, bishop, knight, rook, queen, king (white, black)
            Piece pieceType = static_cast<Piece>(iBoard >= 6 ? iBoard - 5 : iBoard + 1);
            return std::make_pair(iBoard < 6 ? Color::White : Color::Black, pieceType);
        }
    }
    return std::make_pair(Color::White, Piece::Null);
}

std::pair<Color, Piece> Board::GetIsOccupied(const U64 pos, const Color color) const {
    // Similar to GetIsOccupied(pos) but only searches the boards of the provided color
    U8 offset = color == Color::White ? 0 : 6;
    for (int iBoard = offset; iBoard < offset + 6; iBoard++) {
        if(pos & fBoards[iBoard]) {
            Piece pieceType = static_cast<Piece>(iBoard >= 6 ? iBoard - 5 : iBoard + 1);
            return std::make_pair(color, pieceType);
        }
    }
    return std::make_pair(color, Piece::Null);
}

void Board::PrintDetailedMove(U16 move) {
    // TODO: These functions don't work anymore
    U64 target = GetMoveTarget(move);
    U64 origin = GetMoveOrigin(move);
    Piece piece = GetMovePiece(move);
    Piece takenPiece = GetMoveTakenPiece(move);
    char pieceChar = GetPieceChar(piece);
    
    int rank = get_rank_number(target);
    int file = get_file_number(target);
    char fileChar = get_file_char(file);  

    std::string moveStr = "";
    if(piece != Piece::Pawn)
        moveStr += pieceChar;

    if(takenPiece != Piece::Null && piece != Piece::Pawn) {
        moveStr += "x";
    } else if(takenPiece != Piece::Null && piece == Piece::Pawn) {
        moveStr += get_file_char(get_file_number(origin));
        moveStr += "x";
    }
    moveStr += fileChar;
    moveStr += std::to_string(rank);

    // if(checking the king)
    // add a "+"

    std::cout << moveStr << "\n";
}

float Board::GetGamePhase() {
    // Setup such that the initial game state has a phase of 0 and the endgame (K v K+P) = 1
    // Gets the number of each piece of material and subtracts from phase
    float phase = fTotalPhase;
    phase -= __builtin_popcountll(fBoards[0] | fBoards[6]) * fPawnPhase; // pawns
    phase -= __builtin_popcountll(fBoards[1] | fBoards[7]) * fBishopPhase; // bishops
    phase -= __builtin_popcountll(fBoards[2] | fBoards[8]) * fKnightPhase; // knights
    phase -= __builtin_popcountll(fBoards[3] | fBoards[9]) * fRookPhase; // rook
    phase -= __builtin_popcountll(fBoards[4] | fBoards[10]) * fQueenPhase; // queen
    return std::min(std::max(phase / fTotalPhase, (float)0.0), (float)1.0);
}

void Board::PrintFEN() const {
    std::ostringstream fen;
    // Piece placement
    for(int i = 7; i >= 0; --i) {
        U64 rank = RANKS[i]; // Get the bitboard for the current rank
        int emptySquares = 0;
        for(U64 file : FILES) {
            U64 square = file & rank;
            char piece = ' ';
            if (fBoards[0] & square) piece = 'P';
            else if (fBoards[2] & square) piece = 'N';
            else if (fBoards[1] & square) piece = 'B';
            else if (fBoards[3] & square) piece = 'R';
            else if (fBoards[4] & square) piece = 'Q';
            else if (fBoards[5] & square) piece = 'K';
            else if (fBoards[6] & square) piece = 'p';
            else if (fBoards[8] & square) piece = 'n';
            else if (fBoards[7] & square) piece = 'b';
            else if (fBoards[9] & square) piece = 'r';
            else if (fBoards[10] & square) piece = 'q';
            else if (fBoards[11] & square) piece = 'k';

            if (piece != ' ') {
                if (emptySquares > 0) {
                    fen << emptySquares;
                    emptySquares = 0;
                }
                fen << piece;
            } else {
                ++emptySquares;
            }

        }
        if (emptySquares > 0) {
            fen << emptySquares;
        }
        if(get_rank_number(rank) > 1) {
            fen << '/';
        }
    }

    // Side to move
    fen << ' ' << (fColorToMove == Color::White ? 'w' : 'b');

    // Other FEN fields (castling rights, en passant target square, halfmove clock, fullmove number)
    std::vector<char> castlingChars{};
    if(fWhiteKingMoved == 0) {
        if(fWhiteKingsideRookMoved == 0) {
            castlingChars.push_back('K');
        }
        if(fWhiteQueensideRookMoved == 0) {
            castlingChars.push_back('Q');
        } 
    }
    if(fBlackKingMoved == 0) {
        if(fBlackKingsideRookMoved == 0) {
            castlingChars.push_back('k');
        }
        if(fBlackQueensideRookMoved == 0) {
            castlingChars.push_back('q');
        } 
    }

    if(castlingChars.size() > 0) {
        fen << ' ';
        for(char c : castlingChars) {
            fen << c;
        }
    }

    // Available en-passant
    bool wasEnPassant = false;
    if(fMadeMoves.size() > 0) {
        U16 move = fMadeMoves.back();
        const U64 origin = GetMoveOrigin(move);
        const U64 target = GetMoveTarget(move);
        wasEnPassant = GetMoveIsEnPassant(move, fMovedPieces.back(), fTakenPieces.back() == Piece::Null);
        if(wasEnPassant) {
            int offset = fColorToMove == Color::White ? 1 : -1;
            fen << get_file_char(origin) << get_rank_number(target) + offset;
        }
    }

    if(!wasEnPassant)
        fen << ' ' << '-';

    // Add the half-move clock
    fen << ' ' << fHalfMoves << ' ';

    // Add the full move count
    int nFullMoves = (int)(fMadeMoves.size() / 2); // Always rounds down
    fen << nFullMoves;

    std::cout << fen.str() << "\n";
}