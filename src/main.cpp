#include "Game.hpp"

int main() {
    Game myGame = Game(true); // use false to toggle off the GUI
    myGame.Play(Color::White);

    return 0;
}
