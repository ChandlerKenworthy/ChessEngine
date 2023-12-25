#include <memory>

#include "Board.hpp"
#include "Renderer.hpp"
#include "Engine.hpp"

int main() {
    //Game myGame = Game(true, 4); // use false to toggle off the GUI, max depth = 10
    //myGame.Play(Color::White);

    const std::unique_ptr<Board> b = std::make_unique<Board>();
    const std::unique_ptr<Renderer> gui = std::make_unique<Renderer>();
    const std::unique_ptr<Engine> engine = std::make_unique<Engine>(true);

    std::pair<Color, Piece> selectedPiece = std::make_pair(Color::White, Piece::Null);
    U64 pieceTile = 0;
    U64 selectedTile = 0;

    while(gui->GetWindowIsOpen()) {
        sf::Event event;
        while(gui->PollEvent(event)) {
            if(event.type == sf::Event::Closed) {
                gui->CloseWindow();
            } else if(event.type == sf::Event::MouseButtonPressed) {
                selectedTile = gui->GetClickedSquare(event);
                // User clicked on a piece (not an empty square)
                std::pair<Color, Piece> clickedPosition = b->GetIsOccupied(selectedTile);
                if(clickedPosition.second != Piece::Null && clickedPosition.first == b->GetColorToMove()) { // set-bit on the bitboard of occupation
                    selectedPiece = clickedPosition;
                    pieceTile = selectedTile;
                } else { // User clicked on an empty square/enemy piece
                    if(selectedPiece.second != Piece::Null) {
                        engine->GenerateLegalMoves(b);
                        Move userMove = Move{pieceTile, selectedTile, selectedPiece.second};
                        if(engine->GetMoveIsLegal(&userMove)) {
                            std::cout << "Make move was called\n";
                            b->MakeMove(&userMove);
                        } else {
                            std::cout << "Move was found to be illegal!\n";
                        }
                    }
                    selectedPiece = std::make_pair(Color::White, Piece::Null);
                    pieceTile = 0;
                }
                // Find which piece (if any) the user clicked on
                // or if already clicked a piece drop this piece at the tile clicked on
            } else if(event.type == sf::Event::MouseButtonReleased) {
                // Do something
            } else if(event.type == sf::Event::MouseMoved) {
                // Do something
            }
        }
        gui->Update(b);
    }

    return 0;
}
