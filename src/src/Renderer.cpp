#include "Renderer.hpp"

Renderer::Renderer() : 
fLightColor{sf::Color(255, 206, 158)}, 
fDarkColor{sf::Color(209, 139, 71)} {
    fWindow = new sf::RenderWindow(sf::VideoMode(fWindowWidth, fWindowHeight), "Chess Board");
    
    if (!fFont.loadFromFile("../assets/Roboto-Bold.ttf")) {
        // Handle the case where the font cannot be loaded
    }
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

            if(i == 7) {
                // Draw rank numbers on the left side of the board
                sf::Text fileText(std::to_string(8 - j), fFont, 18);
                fileText.setPosition(0.1 * squareSize, (j+0.05) * squareSize);
                fileText.setFillColor((i + j) % 2 == 0 ? fLightColor : fDarkColor);
                fWindow->draw(fileText);
                
            }

            if(j == 7) {
                // Draw file letters at the bottom of the board
                char fileLetter;
                if(i == 0) {
                    fileLetter = 'A';
                } else if(i == 1) {
                    fileLetter = 'B';
                } else if(i == 2) {
                    fileLetter = 'C';
                } else if(i == 3) {
                    fileLetter = 'D';
                } else if(i == 4) {
                    fileLetter = 'E';
                } else if(i == 5) {
                    fileLetter = 'F';
                } else if(i == 6) {
                    fileLetter = 'G';
                } else {
                    fileLetter = 'H';
                }
                sf::Text rankText(fileLetter, fFont, 18);
                rankText.setPosition((i + 0.8) * squareSize, (j + 0.75) * squareSize);
                rankText.setFillColor((i + j) % 2 == 0 ? fDarkColor : fLightColor);
                fWindow->draw(rankText);
            }
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
            std::cout << "Unknown piece type\n";
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
    int posy = 8 - rank;
    int posx = file - 1;

    // Scale the sprite to fit the square, with some additional padding
    // Via the max width/height 
    float maxLength = std::max(pieceTexture.getSize().x, pieceTexture.getSize().y);
    float scale = squareSize * 0.68 / maxLength;
    float xmargin = (squareSize - (pieceTexture.getSize().x * scale)) / 2;
    float ymargin = (squareSize - (pieceTexture.getSize().y * scale)) / 2;

    pieceSprite.setPosition((posx * squareSize) + xmargin, (posy * squareSize) + ymargin);
    pieceSprite.setScale(scale, scale);

    // Draw the piece on the window
    fWindow->draw(pieceSprite);
};
