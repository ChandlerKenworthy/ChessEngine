#ifndef RENDER_HPP
#define RENDER_HPP

#include <memory>

#include <SFML/Graphics.hpp>
#include "Constants.hpp"
#include "Board.hpp"

class Renderer {
    public:
        explicit Renderer();
        ~Renderer();
        void Update(const std::unique_ptr<Board> &board);
        bool GetWindowIsOpen() { return fWindow->isOpen(); };
        void CloseWindow() { fWindow->close(); };
        bool PollEvent(sf::Event &event) { return fWindow->pollEvent(event); };
    private:
        const int fWindowWidth{600};
        const int fWindowHeight{600};
        sf::RenderWindow *fWindow; 
        const sf::Color fLightColor; // Light squares
        const sf::Color fDarkColor;  // Dark squares
        void DrawChessBoard();
        void DrawChessPiece(Piece piece, Color color, const int rank, const int file);

};

#endif