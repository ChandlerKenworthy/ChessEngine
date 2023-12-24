/**
 * @file Renderer.hpp
 * @brief Definition of the Renderer class.
 */

#ifndef RENDER_HPP
#define RENDER_HPP

#include <memory>
#include <vector>

#include <SFML/Graphics.hpp>
#include "Constants.hpp"
#include "Board.hpp"

/**
 * @class Renderer
 * @brief Class to handle displaying the chess board and handling user interactions with the GUI.
 * 
 * The Renderer class provides an interface to easily display, update and handle GUI based events. It should be used only for handling GUI elements and not be involved in updating e.g. bitboards based on player input.
 */ 
class Renderer {
    public:
        /**
         * @brief Construct a renderer object to handle GUI actions
        */
        explicit Renderer();
        ~Renderer();
        /**
         * @brief Clears the window and re-draws the chess board.
         * @param board The chess game to draw on the board.
        */
        void Update(const std::unique_ptr<Board> &board);
        /**
         * @brief Get whether the GUI windows is currently open
        */
        bool GetWindowIsOpen() { return fWindow->isOpen(); };


        void HandlePress(int rank, int file);


        /**
         * @brief Close the currently open window (if it exists)
        */
        void CloseWindow() { fWindow->close(); };
        /**
         * @brief Poll the given event and pop it to the top of the event queue
         * @param event Address of the event that happened
        */
        bool PollEvent(sf::Event &event) { return fWindow->pollEvent(event); };
        /**
         * @brief Get the width of the current window
        */
        int GetWindowWidth() const { return fWindowWidth; };
        /**
         * @brief Get the size of each individual tile on the GUI chess board
        */
        float GetSquareSize() const { return (float)fWindowWidth / 8.; };
    private:
        const int fWindowWidth; ///< Width (and height) of the GUI window in pixels
        int fSquareWidth; ///< Width (and height) of each individual chess square in the GUI in pixels.
        //std::vector<std::pair<int, int>> fHighlightedSquares;
        sf::RenderWindow *fWindow;  ///< The window object for the GUI
        sf::Font fFont; ///< Font to use for rendering the rank and file numbers
        const sf::Color fLightColor; ///< Color of the light squares on the GUI
        const sf::Color fDarkColor; ///< Color of the dark squares on the GUI

        //const sf::Color fYellowLightColor; ///< Color of the light squares on the GUI
        //const sf::Color fYellowDarkColor; ///< Color of the light squares on the GUI

        /**
         * @brief Draw the chess board on the window.
         * @param board The board to display.
        */
        void DrawChessBoard(const std::unique_ptr<Board> &board);
        /**
         * @brief Draw a singular chess piece on the board at the specified rank and file.
         * @param piece The type of piece to draw.
         * @param color The colour of the piece to draw.
         * @param rank The integer rank in the range [1,8].
         * @param file The integer file in the range [1,8].
        */
        void DrawChessPiece(const Piece piece, const Color color, const int rank, const int file);
        /**
         * @brief Wrapper function to make calls to DrawChessPiece more efficiently.
         * @param board Board from which piece positions should be drawn.
         * @param piece The type of piece to draw.
        */
        void DrawPieces(const std::unique_ptr<Board> &board, Piece piece);
};

#endif