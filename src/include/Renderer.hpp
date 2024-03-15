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
#include "Constants.hpp"
#include "Board.hpp"

/**
 * @class Renderer
 * @brief Class to handle displaying the chess board and handling user interactions with the GUI.
 * 
 * The Renderer class provides an interface to easily display, update and handle GUI based events. It should be used only for handling GUI elements and not be involved in updating e.g. bitboards based on player input.
 */ 

class Renderer : public QWidget {
    public:
        Renderer(QWidget *parent = nullptr);
        void DrawPieces(const std::unique_ptr<Board> &board);
    private slots:
        void startSearch();
    private:
        const int fTileWidth;
        int fPieceHeight;

        QSpinBox *depthSpinBox;
        QDoubleSpinBox *timeSpinBox;
        QComboBox *versionComboBox;
        QGraphicsView *view;

        QColor fLightSquare;
        QColor fDarkSquare;

        // Create graphics view and scene
        QGraphicsScene *fScene;

        void DrawChessBoard();
};

#endif