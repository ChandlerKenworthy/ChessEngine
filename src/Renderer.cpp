#include "Renderer.hpp"

Renderer::Renderer(const std::shared_ptr<Board> &board, const std::shared_ptr<Generator> &generator, const std::shared_ptr<Engine> &engine, QWidget *parent) : QGraphicsView(parent), fBoard(board), fGenerator(generator), fEngine(engine), fTileWidth(70), fBoardWidth(560), fBoardHeight(560), fLightSquare(255, 206, 158), fDarkSquare(209, 139, 71), fLightYellow(255, 255, 204), fDarkYellow(255, 255, 0), fIsDragging(false), fStartSquare(0), fEndSquare(0), fSelectedPiece(nullptr), fScene(new QGraphicsScene) {
    fPieceHeight = fTileWidth * 0.75;
    fPieces.reserve(32);
    fDraggedPiece = nullptr;

    // Resize the window on opening
    resize(fBoardWidth, fBoardHeight);

    // Set the scene
    setScene(fScene);
    fScene->setSceneRect(0, 0, fBoardWidth, fBoardHeight);

    // Set render hints
    setRenderHint(QPainter::Antialiasing);
    setMouseTracking(true);

    // Set up chessboard
    DrawChessBoard();
}

Renderer::~Renderer() {
    delete fScene;
    delete fDraggedPiece;
    delete fSelectedPiece;
    fHighlighted.clear();
    fPieces.clear();
}

void Renderer::gameLoopSlot() {
    while (fBoard->GetState() == State::Play) {
        if(fBoard->GetColorToMove() != fUserColor) { // The engine makes a move
            // Re-generate possible moves
            fGenerator->GenerateLegalMoves(fBoard); // Also updates State of board
            if(fGenerator->GetNLegalMoves() != 0) {
                // Find the engine's best move
                U16 move = fEngine->GetBestMove(true);

                // Make the move
                fBoard->MakeMove(move);
                fBoard->AddCurrentHistory();

                // Update the GUI accordingly
                DrawPieces();
            }
        } else {
            fGenerator->GenerateLegalMoves(fBoard); // Also updates State of board
        } // User has to make a move, handled inside of mousePress/Release event
        // Ensure to call QApplication::processEvents() periodically to keep the GUI responsive
        QApplication::processEvents();

         // Check for checkmate condition and emit signal if true
        if(fBoard->GetState() != State::Play) {
            std::cout << "Game terminating due to " << get_string_state(fBoard->GetState()) << "\n";
            emit gameEndSignal();
        }
    }
}

void Renderer::DrawChessBoard() {
    // Add chessboard squares
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            QColor color = (i + j) % 2 == 0 ? fLightSquare : fDarkSquare;
            QGraphicsRectItem *square = fScene->addRect(i * fTileWidth, j * fTileWidth, fTileWidth, fTileWidth, Qt::NoPen, QBrush(color));
            square->setZValue((int)ZLevel::RegularTile);
        }
    }

    // Add rank labels
    for(int i = 0; i < 8; ++i) {
        QString label = QString::number(8 - i);
        QGraphicsTextItem *rankLabel = fScene->addText(label);
        rankLabel->setZValue((int)ZLevel::TileText);
        rankLabel->setPos(0, i * fTileWidth);
        rankLabel->setDefaultTextColor(i % 2 == 0 ? fDarkSquare : fLightSquare);
    }

    // Add file labels
    QString files = "abcdefgh";
    for(int i = 0; i < 8; ++i) {
        QString label = files.at(i);
        QGraphicsTextItem *fileLabel = fScene->addText(label);
        fileLabel->setZValue((int)ZLevel::TileText);
        fileLabel->setPos(i * fTileWidth, (8 * fTileWidth) - (fileLabel->boundingRect().height()));
        fileLabel->setDefaultTextColor(i % 2 == 0 ? fLightSquare : fDarkSquare);
    }
}

void Renderer::DrawPieces() {
    // Empty all the old pieces off of the board
    for(auto currentPiece : fPieces) {
        fScene->removeItem(currentPiece.second);
        delete currentPiece.second;
    }
    fPieces.clear();

    // Draw the current board pieces
    for(Piece p : PIECES) {
        U64 whitePieces = fBoard->GetBoard(Color::White, p);
        U64 blackPieces = fBoard->GetBoard(Color::Black, p);

        std::string assetPathWhite = "/Users/chandler/Documents/Coding/ChessEngine/assets/" + GetPieceString(p);
        assetPathWhite += "_white.png";
        std::string assetPathBlack = "/Users/chandler/Documents/Coding/ChessEngine/assets/" + GetPieceString(p);
        assetPathBlack += "_black.png";

        while(whitePieces) {
            U64 thisPiece = 1ULL << __builtin_ctzll(whitePieces);
            const U8 rank = get_rank_number(thisPiece);
            const U8 file = get_file_number(thisPiece);

            QPixmap pixmap(assetPathWhite.c_str());
            int scaledWidth = fPieceHeight * pixmap.width() / pixmap.height();
            QPixmap scaledPixmap = pixmap.scaled(scaledWidth, fPieceHeight);
            QGraphicsPixmapItem *chessPiece = fScene->addPixmap(scaledPixmap);
            chessPiece->setZValue((int)ZLevel::Piece);
            fPieces.push_back(std::make_pair(__builtin_ctzll(thisPiece), chessPiece));

            int posX = ((file - 1) * fTileWidth) + ((fTileWidth - scaledWidth) / 2);
            int posY = ((8 - rank) * fTileWidth) + ((fTileWidth - fPieceHeight) / 2);
            chessPiece->setPos(posX, posY);
            whitePieces &= whitePieces - 1;
        }

        while(blackPieces) {
            U64 thisPiece = 1ULL << __builtin_ctzll(blackPieces);
            const U8 rank = get_rank_number(thisPiece);
            const U8 file = get_file_number(thisPiece);

            QPixmap pixmap(assetPathBlack.c_str());
            int scaledWidth = fPieceHeight * pixmap.width() / pixmap.height();
            QPixmap scaledPixmap = pixmap.scaled(scaledWidth, fPieceHeight);
            QGraphicsPixmapItem *chessPiece = fScene->addPixmap(scaledPixmap);
            chessPiece->setZValue((int)ZLevel::Piece);
            fPieces.push_back(std::make_pair(__builtin_ctzll(thisPiece), chessPiece));

            int posX = ((file - 1) * fTileWidth) + ((fTileWidth - scaledWidth) / 2);
            int posY = ((8 - rank) * fTileWidth) + ((fTileWidth - fPieceHeight) / 2);
            chessPiece->setPos(posX, posY);
            blackPieces &= blackPieces - 1;
        }
    }
}

void Renderer::mousePressEvent(QMouseEvent *event) {
    fDraggedPiece = nullptr;
    QPointF scenePos = mapToScene(event->pos());

    // Convert the scene position to integer coordinates
    int x = static_cast<int>(scenePos.x());
    int y = static_cast<int>(scenePos.y());

    // Calculate the row and column of the clicked square
    U64 rank = get_rank_from_number(8 - (y / fTileWidth));
    U64 file = get_file_from_number((x / fTileWidth) + 1);
    fStartSquare = rank & file;
    const U8 dragPieceLSB = __builtin_ctzll(fStartSquare);
    fIsDragging = true; // True whilst mouse is clicked

    // Highlight the squares which represent legal moves
    HighlightLegalMoves();

    // Get the piece being dragged 
    auto it = std::find_if(fPieces.begin(), fPieces.end(), [dragPieceLSB](const std::pair<U8, QGraphicsPixmapItem*>& element) {
        return element.first == dragPieceLSB;
    });

    if(it != fPieces.end()) {
        fDraggedPiece = it->second;
    }

    // Call the base class implementation to handle the event further
    QGraphicsView::mousePressEvent(event);
}

void Renderer::HighlightLegalMoves() {
    // Search the legal moves vector for all moves starting on fStartSquare
    fHighlighted.clear();
    std::vector<U64> legalEndTiles{};

    std::vector<U16> legalMoves = fGenerator->GetLegalMoves();
    for(std::size_t iMove = 0; iMove < legalMoves.size(); ++iMove) {
        if(GetMoveOrigin(legalMoves[iMove]) & fStartSquare) {
            legalEndTiles.push_back(GetMoveTarget(legalMoves[iMove]));
        }
    }

    // this is actually just drawing another rectangle over the board but under the pieces & labels
    for(U64 endTile : legalEndTiles) {
        const int rank = get_rank_number(endTile) - 1;
        const int file = get_file_number(endTile) - 1;
        QColor color = (rank + file) % 2 == 0 ? fLightYellow : fDarkYellow;
        QGraphicsRectItem *square = fScene->addRect(file * fTileWidth, (8 - (rank + 1)) * fTileWidth, fTileWidth, fTileWidth, Qt::NoPen, QBrush(color));
        square->setZValue((int)ZLevel::HighlightTile);
        fHighlighted.push_back(square);
    }
}

void Renderer::mouseReleaseEvent(QMouseEvent *event) {
    QPointF scenePos = mapToScene(event->pos());

    // Clear all highlighted tiles
    for(auto tile : fHighlighted) {
        fScene->removeItem(tile);
        delete tile;
    }
    fHighlighted.clear();

    // Convert the scene position to integer coordinates
    int x = static_cast<int>(scenePos.x());
    int y = static_cast<int>(scenePos.y());

    // Calculate the row and column of the clicked square
    U64 rank = get_rank_from_number(8 - (y / fTileWidth));
    U64 file = get_file_from_number((x / fTileWidth) + 1);
    fEndSquare = rank & file;

    // also update the GUI based on the new board updates
    U16 move = 0;
    SetMove(move, fStartSquare, fEndSquare);
    // Before making any move check it is legal, if it is not send the clicked piece back to the start square
    bool isLegal = fGenerator->GetMoveIsLegal(move);
    if(isLegal) {
        fBoard->MakeMove(move);
        fBoard->AddCurrentHistory();
    }
    
    DrawPieces();
    fIsDragging = false;
    fStartSquare = 0;
    QGraphicsView::mouseReleaseEvent(event);
}

void Renderer::mouseMoveEvent(QMouseEvent *event) {
    // If a piece is being dragged, update its position
    if(!fIsDragging || fDraggedPiece == nullptr)
        return;

    QPointF scenePos = mapToScene(event->pos());

    // Adjust the scenePos to center of the piece
    qreal offsetX = -fDraggedPiece->boundingRect().width() / 2.0;
    qreal offsetY = -fDraggedPiece->boundingRect().height() / 2.0;
    QPointF adjustedScenePos = scenePos + QPointF(offsetX, offsetY);

    fDraggedPiece->setPos(adjustedScenePos);

    // Call the base class implementation to handle the event further
    QGraphicsView::mouseMoveEvent(event);
}