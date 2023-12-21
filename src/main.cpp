#include "Game.hpp"
#include "Renderer.hpp"


int main() {
    Game myGame = Game();
    Renderer gui = Renderer();

    while(gui.GetWindowIsOpen()) {
        sf::Event event;
        while(gui.PollEvent(event)) {
            if(event.type == sf::Event::Closed) {
                gui.CloseWindow();
            }
        }

        Board b = myGame.GetBoard();
        gui.Update(&b);
    }

    return 0;
}
