#include <memory>

#include "Board.hpp"
#include "Renderer.hpp"

int main() {
    //Game myGame = Game(true, 4); // use false to toggle off the GUI, max depth = 10
    //myGame.Play(Color::White);

    const std::unique_ptr<Board> b = std::make_unique<Board>();
    const std::unique_ptr<Renderer> gui = std::make_unique<Renderer>();

    while(gui->GetWindowIsOpen()) {
        sf::Event event;
        while(gui->PollEvent(event)) {
            if(event.type == sf::Event::Closed) {
                gui->CloseWindow();
            } else if(event.type == sf::Event::MouseButtonPressed) {
                // Do something
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
