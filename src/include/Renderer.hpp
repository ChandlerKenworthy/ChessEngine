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
        Renderer(const std::unique_ptr<Board> &board, const std::unique_ptr<Generator> &generator, const std::unique_ptr<Engine> &engine, QWidget *parent = nullptr);
        void DrawPieces();
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
        const int fTileWidth;
        int fPieceHeight;
        Color fUserColor;

        const int fBoardWidth;
        const int fBoardHeight;

        QSpinBox *depthSpinBox;
        QDoubleSpinBox *timeSpinBox;
        QComboBox *versionComboBox;
        QGraphicsView *view;

        QColor fLightSquare;
        QColor fDarkSquare;

        // For handling piece movement (drag and drop style)
        bool fIsDragging;
        U64 fStartSquare;
        U64 fEndSquare;
        QGraphicsPixmapItem *fSelectedPiece;
        std::vector<std::pair<U8, QGraphicsPixmapItem*>> fPieces;
        QGraphicsPixmapItem *fDraggedPiece;

        // Create graphics view and scene
        QGraphicsScene *fScene;

        void DrawChessBoard();
        void DoMoveUpdate();
};

#endif