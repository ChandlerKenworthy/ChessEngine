#include <memory>

#include "Board.hpp"
#include "Renderer.hpp"

int main() {
    //Game myGame = Game(true, 4); // use false to toggle off the GUI, max depth = 10
    //myGame.Play(Color::White);

    const std::unique_ptr<Board> b = std::make_unique<Board>();
    const std::unique_ptr<Renderer> gui = std::make_unique<Renderer>();

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
                if(clickedPosition.second != Piece::Null) { // set-bit on the bitboard of occupation
                    selectedPiece = clickedPosition;
                    pieceTile = selectedTile;
                } else { // User clicked on an empty square
                    // TODO: This should actually call the MakeMove function of board...
                    // since that will properly handle move legality and capturing etc.
                    if(selectedPiece.second != Piece::Null) {
                        U64 currBoard = b->GetBoard(selectedPiece.first, selectedPiece.second);
                        // take selected piece position and move to selectedTile
                        clear_bit(currBoard, get_LSB(pieceTile));
                        set_bit(currBoard, get_LSB(selectedTile));
                        b->SetBoard(selectedPiece.first, selectedPiece.second, currBoard);
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
