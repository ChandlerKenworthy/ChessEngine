#include <memory>

#include "Move.hpp"
#include "Board.hpp"
#include "Renderer.hpp"
#include "Engine.hpp"

int main() {
    const std::unique_ptr<Board> b = std::make_unique<Board>();
    const std::unique_ptr<Renderer> gui = std::make_unique<Renderer>();
    const std::unique_ptr<Engine> engine = std::make_unique<Engine>(true);

    std::pair<Color, Piece> selectedPiece = std::make_pair(Color::White, Piece::Null);
    U64 pieceTile = 0;
    U64 selectedTile = 0;

    bool makeLegalMove = false;
    U32 userMove;
    
    gui->Update(b); // Draw board initially

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
                        SetMove(userMove, pieceTile, selectedTile, selectedPiece.second, Piece::Null);
                        if(engine->GetMoveIsLegal(&userMove)) {
                            makeLegalMove = true;
                        } else {
                            std::cout << "Move was found to be illegal!\n";
                        }
                    }
                    selectedPiece = std::make_pair(Color::White, Piece::Null);
                    pieceTile = 0;
                }
                // Find which piece (if any) the user clicked on
                // or if already clicked a piece drop this piece at the tile clicked on
            } else if(event.type == sf::Event::KeyPressed) {
                if(event.key.code == sf::Keyboard::B) {
                    SetMovePromotionPiece(userMove, Piece::Bishop);
                } else if(event.key.code == sf::Keyboard::R) {
                    SetMovePromotionPiece(userMove, Piece::Rook);
                } else if(event.key.code == sf::Keyboard::N) {
                    SetMovePromotionPiece(userMove, Piece::Knight);
                } else { // Default to queen promotion
                    SetMovePromotionPiece(userMove, Piece::Queen);
                }
            } // TODO: When pressing a key after a legal move is ready it tries to make a null move then swithces possession. 
            // Still getting the "We got a keyboard without any keys." issue as well currently...

            if(makeLegalMove) {
                b->MakeMove(userMove);
                gui->Update(b); // Only update GUI when a move was actually made

                // Clear variables
                makeLegalMove = false;
                userMove = 0;
            }
        }
    }

    return 0;
}
