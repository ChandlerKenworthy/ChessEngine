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
        void Update(Board *board);
        bool GetWindowIsOpen() { return fWindow->isOpen(); };
        void CloseWindow() { fWindow->close(); };
        bool PollEvent(sf::Event &event) { return fWindow->pollEvent(event); };
    private:
        const int fWindowWidth{800};
        const int fWindowHeight{800};
        sf::RenderWindow *fWindow; 
        sf::Font fFont;
        const sf::Color fLightColor; // Light squares
        const sf::Color fDarkColor;  // Dark squares
        void DrawChessBoard(Board *board);
        void DrawChessPiece(Piece piece, Color color, const int rank, const int file);
        void DrawPawns(U64 whitePawns, U64 blackPawns);
        void DrawKnights(U64 whiteKnights, U64 blackKnights);
        void DrawBishops(U64 whiteBishops, U64 blackBishops);
        void DrawRooks(U64 whiteRooks, U64 blackRooks);
        void DrawSinglePiece(Board *board, Piece piece);
        void DrawMultiPiece(Board *board, Piece piece);
};

#endif