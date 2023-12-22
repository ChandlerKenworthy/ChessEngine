#include "Game.hpp"

int main() {
    Game myGame = Game(true, 4); // use false to toggle off the GUI, max depth = 10
    myGame.Play(Color::White);

    return 0;
}
