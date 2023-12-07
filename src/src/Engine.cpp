#include <iostream>
#include <fstream>
#include <string>

#include "Engine.hpp"

Engine::Engine(bool init) {
    fMaxDepth = 3;
    if(init) Initalize();
}

void Engine::Initalize() {
    // Load the positional arrays
    std::ifstream whitePawns("../src/data/whitePawns.txt");
    int i = 63;
    for(std::string line; getline(whitePawns, line, ' ');) {
        fWhitePawnPos[i] = std::stof(line);
        i--;
    }
    whitePawns.close();

    std::ifstream blackPawns("../src/data/blackPawns.txt");
    i = 63;
    for(std::string line; getline(blackPawns, line, ' ');) {
        fBlackPawnPos[i] = std::stof(line);
        i--;
    }
    blackPawns.close();

    std::ifstream whiteQueen("../src/data/whiteQueen.txt");
    i = 63;
    for(std::string line; getline(whiteQueen, line, ' ');) {
        fWhiteQueenPos[i] = std::stof(line);
        i--;
    }
    whiteQueen.close();

    std::ifstream blackQueen("../src/data/blackQueen.txt");
    i = 63;
    for(std::string line; getline(blackQueen, line, ' ');) {
        fBlackQueenPos[i] = std::stof(line);
        i--;
    }
    blackQueen.close();

    std::ifstream whiteRook("../src/data/whiteRook.txt");
    i = 63;
    for(std::string line; getline(whiteRook, line, ' ');) {
        fWhiteRookPos[i] = std::stof(line);
        i--;
    }
    whiteRook.close();

    std::ifstream blackRook("../src/data/blackRook.txt");
    i = 63;
    for(std::string line; getline(blackRook, line, ' ');) {
        fBlackRookPos[i] = std::stof(line);
        i--;
    }
    blackRook.close();

    std::ifstream whiteBishop("../src/data/whiteBishop.txt");
    i = 63;
    for(std::string line; getline(whiteBishop, line, ' ');) {
        fWhiteBishopPos[i] = std::stof(line);
        i--;
    }
    whiteBishop.close();

    std::ifstream blackBishop("../src/data/blackBishop.txt");
    i = 63;
    for(std::string line; getline(blackBishop, line, ' ');) {
        fBlackBishopPos[i] = std::stof(line);
        i--;
    }
    blackBishop.close();

    
    std::ifstream whiteKnight("../src/data/whiteKnight.txt");
    i = 63;
    for(std::string line; getline(whiteKnight, line, ' ');) {
        fWhiteKnightPos[i] = std::stof(line);
        i--;
    }
    whiteRook.close();

    std::ifstream blackKnight("../src/data/blackKnight.txt");
    i = 63;
    for(std::string line; getline(blackKnight, line, ' ');) {
        fBlackKnightPos[i] = std::stof(line);
        i--;
    }
    blackKnight.close();



    std::ifstream whiteKing("../src/data/whiteKing.txt");
    i = 63;
    for(std::string line; getline(whiteKing, line, ' ');) {
        fWhiteKingPos[i] = std::stof(line);
        i--;
    }
    whiteKing.close();

    std::ifstream blackKing("../src/data/blackKing.txt");
    i = 63;
    for(std::string line; getline(blackKing, line, ' ');) {
        fBlackKingPos[i] = std::stof(line);
        i--;
    }
    blackKing.close();
}

float Engine::Evaluate(const std::unique_ptr<Board> &board) {
    
    float eval = GetMaterialEvaluation(board);
    return eval; // Return in centipawns rather than pawns
}

float Engine::GetPositionalEvaluation(U64 position, Piece piece, Color pieceColor) {
    float base_value = 0.;
    switch (piece) {
    case Piece::Pawn:
        base_value = VALUE_PAWN;
        break;
    case Piece::Bishop:
        base_value = VALUE_BISHOP;
        break;
    case Piece::Knight:
        base_value = VALUE_KNIGHT;
        break;
    case Piece::Rook:
        base_value = VALUE_ROOK;
        break;
    case Piece::Queen:
        base_value = VALUE_QUEEN;
        break;
    case Piece::King:
        base_value = VALUE_KING;
        break;
    default:
        std::cout << "[Warning] Something went wrong searching for a piece value\n";
        break;
    }
    float positionTotal = 0.;
    while(position) {
        U64 singlePiece = pop_LSB(position);
        positionTotal += (GetPositionValue(get_LSB(singlePiece), piece, pieceColor) * base_value);
    }

    return positionTotal;
}

float Engine::GetPositionValue(int index, Piece piece, Color color) {
    switch (piece) {
    case Piece::Pawn:
        return color == Color::White ? fWhitePawnPos[index] : fBlackPawnPos[index];
        break;
    case Piece::Knight:
        return color == Color::White ? fWhiteKnightPos[index] : fBlackKnightPos[index];
        break;
    case Piece::Bishop:
        return color == Color::White ? fWhiteBishopPos[index] : fBlackBishopPos[index];
        break;
    case Piece::Rook:
        return color == Color::White ? fWhiteRookPos[index] : fBlackRookPos[index];
        break;
    case Piece::Queen:
        return color == Color::White ? fWhiteQueenPos[index] : fBlackQueenPos[index];
        break;
    case Piece::King:
        return color == Color::White ? fWhiteKingPos[index] : fBlackKingPos[index];
        break;
    default:
        std::cout << "[Warning] Invalid piece searched in GetPositionValue\n";
        return 0.;
        break;
    }
}

float Engine::GetMaterialEvaluation(const std::unique_ptr<Board> &board) {
    // TODO: Should value of pieces be functions of number of pieces on board?
    // E.g. bishops at start are weak but later on are really powerful on clear board
    float material = 0.;

    for(Piece p : PIECES) {
        if(p == Piece::King)
            continue; // Always kings on the board and king safety evaluated differently
        float white_value = GetPositionalEvaluation(board->GetBoard(Color::White, p), p, Color::White);
        float black_value = GetPositionalEvaluation(board->GetBoard(Color::Black, p), p, Color::Black);
        material += (white_value - black_value);
    }

    /*
    material += (__builtin_popcountll(board->GetBoard(Color::White, Piece::Pawn)) - __builtin_popcountll(board->GetBoard(Color::Black, Piece::Pawn))) * VALUE_PAWN;
    material += (__builtin_popcountll(board->GetBoard(Color::White, Piece::Bishop)) - __builtin_popcountll(board->GetBoard(Color::Black, Piece::Bishop))) * VALUE_BISHOP;
    material += (__builtin_popcountll(board->GetBoard(Color::White, Piece::Knight)) - __builtin_popcountll(board->GetBoard(Color::Black, Piece::Knight))) * VALUE_KNIGHT;
    material += (__builtin_popcountll(board->GetBoard(Color::White, Piece::Rook)) - __builtin_popcountll(board->GetBoard(Color::Black, Piece::Rook))) * VALUE_ROOK;
    material += (__builtin_popcountll(board->GetBoard(Color::White, Piece::Queen)) - __builtin_popcountll(board->GetBoard(Color::Black, Piece::Queen))) * VALUE_QUEEN;
    */
    return material;
}

/*
float Engine::Minimax(Board *board, int depth, float alpha, float beta, Color maximisingPlayer) {
    // Returns the maximum / minimum evaluation of a given position
    if(depth > fMaxDepth)
        depth = fMaxDepth;
    if(board->GetGameIsOver() || depth == 0)
        return Evaluate(board);
    if(maximisingPlayer == Color::White) {
        float maxEval = -99999.;
        for(move : moves) { // iterate through all possible moves for white in current position
            float eval = Minimax(child, depth - 1, alpha, beta, Color::Black); // child is board after the move is made
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if(beta <= alpha)
                break;
        }
        return maxEval;
    } else {
        float minEval = 99999.;
        for(move : moves) {
            float eval = Minimax(child, depth - 1, alpha, beta, Color::White);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if(beta <= alpha)
                break;
        }
        return minEval;
    }   
}
*/