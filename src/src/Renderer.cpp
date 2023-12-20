#include "Renderer.hpp"

Renderer::Renderer() : 
fLightColor{sf::Color(255, 206, 158)}, 
fDarkColor{sf::Color(209, 139, 71)} {
    fWindow = new sf::RenderWindow(sf::VideoMode(fWindowWidth, fWindowHeight), "Chess Board");
}

Renderer::~Renderer() {
    delete fWindow;
}

void Renderer::Update(const std::unique_ptr<Board> &board) {
    fWindow->clear();
    DrawChessBoard();
    fWindow->display();
};

void Renderer::DrawChessBoard() {
    const int squareSize = fWindowWidth / 8;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            sf::RectangleShape square(sf::Vector2f(squareSize, squareSize));
            square.setPosition(i * squareSize, j * squareSize);
            square.setFillColor((i + j) % 2 == 0 ? fLightColor : fDarkColor);
            fWindow->draw(square);
        }
    }

    // drawChessPiece(window, Piece::Pawn, Color::White, sf::Vector2i(2, 3), squareSize);
    // int rank_number = 5;
    // int file_number = 4;
    // drawChessPiece(window, Piece::Pawn, Color::Black, sf::Vector2i(file_number - 1, 8 - rank_number), squareSize);
};

void Renderer::DrawChessPiece(Piece piece, Color color, const int rank, const int file) {
    /*   sf::Texture pieceTexture;
    std::string filename;

    // Determine the filename based on piece type and color
    switch (type) {
        case Piece::Pawn:
            filename = (color == Color::White) ? "white_pawn.png" : "black_pawn.png";
            break;
        // Add cases for other piece types (Rook, Knight, Bishop, Queen, King) here
        // ...

        default:
            // Handle an unknown piece type
            return;
    }

    // Load the texture from file
    if (!pieceTexture.loadFromFile("../assets/" + filename)) {
        // Handle the case where the texture cannot be loadeds
        std::cout << "Failed to load file\n";
        return;
    }

    // Create a sprite and set its position
    sf::Sprite pieceSprite(pieceTexture);
    const float squarePad = 0.2 * static_cast<float>(squareSize);
    pieceSprite.setPosition((position.x * squareSize) + (squarePad / 2), (position.y * squareSize) + (squarePad / 2));

    // Scale the sprite to fit the square, with some additional padding
    pieceSprite.setScale(static_cast<float>(squareSize) * 0.8 / pieceTexture.getSize().x,
                         static_cast<float>(squareSize) * 0.8 / pieceTexture.getSize().y);

    // Draw the piece on the window
    window.draw(pieceSprite);*/
};
