#include <SFML/Graphics.hpp>
#include "Game.hpp"

void drawChessPiece(sf::RenderWindow& window, Piece type, Color color, sf::Vector2i position, int squareSize);

void drawChessBoard(sf::RenderWindow& window, const int windowWidth) {
    const int squareSize = windowWidth / 8;

    sf::Color color1(255, 206, 158); // Light squares
    sf::Color color2(209, 139, 71); // Dark squares

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            sf::RectangleShape square(sf::Vector2f(squareSize, squareSize));
            square.setPosition(i * squareSize, j * squareSize);
            square.setFillColor((i + j) % 2 == 0 ? color1 : color2);

            window.draw(square);
        }
    }

    // Draw pieces here based on your bit boards
    // Example: window.draw(myPieceSprite);
    drawChessPiece(window, Piece::Pawn, Color::White, sf::Vector2i(2, 3), squareSize);

    // To turn a rank/file into a (x,y) vector File number = 1, 2, 3... = x-coordinate + 1
    // rank number = 1, 2, 3, 4,  = 8 - y-coordinate
    int rank_number = 5;
    int file_number = 4;
    drawChessPiece(window, Piece::Pawn, Color::White, sf::Vector2i(file_number - 1, 8 - rank_number), squareSize);
}

void drawChessPiece(sf::RenderWindow& window, Piece type, Color color, sf::Vector2i position, int squareSize) {
    sf::Texture pieceTexture;
    std::string filename;

    // Determine the filename based on piece type and color
    switch (type) {
        case Piece::Pawn:
            filename = (color == Color::White) ? "white_pawn_edit.png" : "black_pawn.jpeg";
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
    window.draw(pieceSprite);
}

int main() {
    const int WINDOW_WIDTH = 600;
    const int WINDOW_HEIGHT = 600;

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Chess Board");

    while(window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Your rendering code goes here

        window.clear();  // Clear the window before rendering

        // Draw your chessboard or any other elements here
        drawChessBoard(window, WINDOW_WIDTH);

        window.display(); // Display the contents of the window
    }

    return 0;
}
