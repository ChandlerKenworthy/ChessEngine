#ifndef GAME_HPP
#define GAME_HPP

#include <iostream>
#include <bitset>
#include <algorithm>
#include <cctype>
#include <vector>
#include <random>
#include <memory>
#include <regex>

#include "Constants.hpp"
#include "Board.hpp"
#include "Engine.hpp"
#include "Renderer.hpp"

class Game {
    public:
        explicit Game(bool useGUI, int maxDepth);
        void Play(Color playerColor); // Initalises the game loop
        //const std::unique_ptr<Board>& GetBoard() { return std::make_unique<Board>(fBoard); }
        Move GetUserMove();
        void PrintEngineMove(Move move);
        std::string GetPieceString(Piece piece);
        Board GetBoard() const { return fBoard; };
    private:
        //#std::unique_ptr<Board> fBoard;
        Board fBoard;
        std::unique_ptr<Engine> fEngine;
        bool fUseGUI;
        std::unique_ptr<Renderer> fGUI;
};

#endif