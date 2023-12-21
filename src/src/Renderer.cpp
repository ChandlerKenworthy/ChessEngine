#include "Renderer.hpp"

Renderer::Renderer() : 
fLightColor{sf::Color(255, 206, 158)}, 
fDarkColor{sf::Color(209, 139, 71)} {
    fWindow = new sf::RenderWindow(sf::VideoMode(fWindowWidth, fWindowHeight), "Chess Board");
}

Renderer::~Renderer() {
    delete fWindow;
}

void Renderer::Update(Board *board) {
    fWindow->clear();
    DrawChessBoard(board);
    fWindow->display();
};

void Renderer::DrawChessBoard(Board *board) {
    const int squareSize = fWindowWidth / 8;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            sf::RectangleShape square(sf::Vector2f(squareSize, squareSize));
            square.setPosition(i * squareSize, j * squareSize);
            square.setFillColor((i + j) % 2 == 0 ? fLightColor : fDarkColor);
            fWindow->draw(square);
        }
    }

    DrawSinglePiece(board, Piece::Queen);
    DrawSinglePiece(board, Piece::King);
    DrawMultiPiece(board, Piece::Rook);
    DrawMultiPiece(board, Piece::Rook);
    DrawMultiPiece(board, Piece::Bishop);
    DrawMultiPiece(board, Piece::Knight);
    DrawMultiPiece(board, Piece::Pawn);
};

void Renderer::DrawMultiPiece(Board *board, Piece piece) {
    // Rooks, bishops, knights, pawns
    U64 white = board->GetBoard(Color::White, piece);
    U64 black = board->GetBoard(Color::Black, piece);

    while(white) {
        U64 pos = 0;
        set_bit(pos, pop_LSB(white));
        int rank = get_rank_number(pos);
        int file = get_file_number(pos);
        DrawChessPiece(piece, Color::White, rank, file);
    }

    while(black) {
        U64 pos = 0;
        set_bit(pos, pop_LSB(black));
        int rank = get_rank_number(pos);
        int file = get_file_number(pos);
        DrawChessPiece(piece, Color::Black, rank, file);
    }
}

void Renderer::DrawSinglePiece(Board *board, Piece piece) {
    // Queen or King
    U64 white = board->GetBoard(Color::White, piece);
    U64 black = board->GetBoard(Color::Black, piece);

    if(white) {
        int rank = get_rank_number(white);
        int file = get_file_number(white);
        DrawChessPiece(piece, Color::White, rank, file);
    }
    if(black) {
        int rank = get_rank_number(black);
        int file = get_file_number(black);
        DrawChessPiece(piece, Color::Black, rank, file);
    }
}

void Renderer::DrawChessPiece(Piece piece, Color color, const int rank, const int file) {
    sf::Texture pieceTexture;
    std::string filename;

    // Determine the filename based on piece type and color
    switch(piece) {
        case Piece::Pawn:
            filename = (color == Color::White) ? "pawn_white.png" : "pawn_black.png";
            break;
        case Piece::Bishop:
            filename = (color == Color::White) ? "bishop_white.png" : "bishop_black.png";
            break;
        case Piece::Knight:
            filename = (color == Color::White) ? "knight_white.png" : "knight_black.png";
            break;
        case Piece::Rook:
            filename = (color == Color::White) ? "rook_white.png" : "rook_black.png";
            break;
        case Piece::Queen:
            filename = (color == Color::White) ? "queen_white.png" : "queen_black.png";
            break;
        case Piece::King:
            filename = (color == Color::White) ? "king_white.png" : "king_black.png";
            break;
        default:
            // Handle an unknown piece type
            return;
    }

    // Load the texture from file
    if (!pieceTexture.loadFromFile("../assets/" + filename)) {
        // Handle the case where the texture cannot be loadeds
        return;
    }

    // Create a sprite and set its position
    sf::Sprite pieceSprite(pieceTexture);
    const float squareSize = static_cast<float>(fWindowWidth) / 8.;
    const float squarePad = 0.2 * squareSize;
    int posy = 8 - rank;
    int posx = file - 1;
    pieceSprite.setPosition((posx * squareSize) + (squarePad / 2), (posy * squareSize) + (squarePad / 2));

    // Scale the sprite to fit the square, with some additional padding
    pieceSprite.setScale(static_cast<float>(squareSize) * 0.8 / pieceTexture.getSize().x,
                         static_cast<float>(squareSize) * 0.8 / pieceTexture.getSize().y);

    // Draw the piece on the window
    fWindow->draw(pieceSprite);
};
