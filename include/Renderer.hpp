/**
 * @file Renderer.hpp
 * @brief Definition of the Renderer class.
 */

#ifndef RENDER_HPP
#define RENDER_HPP

#include <memory>
#include <vector>
#include <string>
#include <iostream>

#include <QtWidgets>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMouseEvent>
#include "Constants.hpp"
#include "Board.hpp"
#include "Engine.hpp"
#include "Generator.hpp"

/**
 * @class Renderer
 * @brief Class to handle displaying the chess board and handling user interactions with the GUI.
 * 
 * The Renderer class provides an interface to easily display, update and handle GUI based events. It should be used only for handling GUI elements and not be involved in updating e.g. bitboards based on player input.
 */ 

class Renderer : public QGraphicsView {
        Q_OBJECT
    public:
        /**
         * @brief Construct a Qt-based renderer object. This will not make the window appear. To draw the window use the show() method. 
         * @param board The board to display in the viewport. 
         * @param generator The backend move generator.
         * @param engine The instance of the computer game engine.
         * @param parent The QWidget parent.
        */
        Renderer(const std::unique_ptr<Board> &board, const std::unique_ptr<Generator> &generator, const std::unique_ptr<Engine> &engine, QWidget *parent = nullptr);
        /**
         * @brief Destructs the renderer object safely ensuring all raw pointers are deleted.
        */
        ~Renderer();
        /**
         * @brief Draw all the pieces as they are in the current board instance passed to the constructor. 
        */
        void DrawPieces();
        /**
         * @brief Set the colour of the human user. Will disable moving the opposing colours.
         * @param color The colour the user is to play as.
        */
        void setUserColor(const Color color) { fUserColor = color; };
    public slots:
        void gameLoopSlot();
    signals:
        void gameLoopSignal();
    protected:
        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
    private:
        const std::unique_ptr<Board> &fBoard;
        const std::unique_ptr<Generator> &fGenerator;
        const std::unique_ptr<Engine> &fEngine;
        const int fTileWidth; ///< The width (and height since tiles are square) of tiles on the board.
        int fPieceHeight; ///< The height of pieces (used when drawing the board, must be less than fTileWidth).
        Color fUserColor; ///< The colour of pieces the human user controls.

        const int fBoardWidth; ///< The height/width of the board (they are interchangeable).
        const int fBoardHeight; ///< The height/width of the board (they are interchangeable).

        QColor fLightSquare; ///< Colour for drawing the squares.
        QColor fDarkSquare; ///< Colour for drawing the squares.
        QColor fLightYellow; ///< Colour for drawing the squares.
        QColor fDarkYellow; ///< Colour for drawing the squares.

        // For handling piece movement (drag and drop style)
        bool fIsDragging;
        U64 fStartSquare;
        U64 fEndSquare;
        QGraphicsPixmapItem *fSelectedPiece;
        std::vector<std::pair<U8, QGraphicsPixmapItem*>> fPieces;
        std::vector<QGraphicsRectItem*> fHighlighted;
        QGraphicsPixmapItem *fDraggedPiece;

        // Create graphics view and scene
        QGraphicsScene *fScene;

        void DrawChessBoard();
        void DoMoveUpdate();
        void HighlightLegalMoves();
};

enum class ZLevel {
    RegularTile,
    HighlightTile,
    TileText,
    Piece
};

#endif