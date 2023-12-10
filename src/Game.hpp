#ifndef GAME_HPP
#define GAME_HPP

#include <iostream>
#include <bitset>
#include <vector>
#include <random>
#include <memory>
#include <regex>

#include "Constants.hpp"
#include "Board.hpp"
#include "Engine.hpp"

class Game {
    public:
        explicit Game();
        void Play(Color playerColor); // Initalises the game loop
        const std::unique_ptr<Board>& GetBoard() { return fBoard; }
        Move GetUserMove();
    private:
        std::unique_ptr<Board> fBoard;
        std::unique_ptr<Engine> fEngine;
};

#endif