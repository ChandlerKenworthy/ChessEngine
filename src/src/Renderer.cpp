#include "Renderer.hpp"

Renderer::Renderer(const std::unique_ptr<Board> &board, QWidget *parent) : QGraphicsView(parent), fBoard(board), fTileWidth(70), fBoardWidth(560), fBoardHeight(560), fLightSquare(255, 206, 158), fDarkSquare(209, 139, 71), fScene(new QGraphicsScene), fIsDragging(false), fStartSquare(0), fEndSquare(0), fSelectedPiece(nullptr) {
    fPieceHeight = fTileWidth * 0.8;
    fPieces.reserve(32);
    fDraggedPiece = nullptr;

    // Resize the window on opening
    resize(750, 750);

    // Set the scene
    setScene(fScene);
    fScene->setSceneRect(0, 0, fBoardWidth, fBoardHeight);

    // Set render hints
    setRenderHint(QPainter::Antialiasing);
    setMouseTracking(true);

    // Set up chessboard
    DrawChessBoard();


    //QVBoxLayout *layout = new QVBoxLayout(this);
    
    // Max Depth
    //QHBoxLayout *depthLayout = new QHBoxLayout;
    //QLabel *depthLabel = new QLabel("Max Depth:", this);
    //depthSpinBox = new QSpinBox(this);
    //depthLayout->addWidget(depthLabel);
    //depthLayout->addWidget(depthSpinBox);
    
    // Max Time
    //QHBoxLayout *timeLayout = new QHBoxLayout;
    //QLabel *timeLabel = new QLabel("Max Time (seconds):", this);
    //timeSpinBox = new QDoubleSpinBox(this);
    //timeSpinBox->setDecimals(1); // Adjust as needed
    //timeLayout->addWidget(timeLabel);
    //timeLayout->addWidget(timeSpinBox);
    
    // Engine Version
    //QHBoxLayout *versionLayout = new QHBoxLayout;
    //QLabel *versionLabel = new QLabel("Engine Version:", this);
    //versionComboBox = new QComboBox(this);
    //versionComboBox->addItem("Version 1");
    //versionComboBox->addItem("Version 2");
    //versionLayout->addWidget(versionLabel);
    //versionLayout->addWidget(versionComboBox);
    
    // Add layouts to main layout
    //layout->addLayout(depthLayout);
    //layout->addLayout(timeLayout);
    //layout->addLayout(versionLayout);
    
    // Add buttons
    //QPushButton *startButton = new QPushButton("Start", this);
    //connect(startButton, &QPushButton::clicked, this, &Renderer::startSearch);
    //layout->addWidget(startButton);
}

void Renderer::startSearch() {
    int maxDepth = depthSpinBox->value();
    double maxTime = timeSpinBox->value();
    QString engineVersion = versionComboBox->currentText();
        
    // Call your chess engine with the selected parameters
    // YourEngineClass::search(maxDepth, maxTime, engineVersion);
}

void Renderer::DrawChessBoard() {
    // Add chessboard squares
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            QColor color = (i + j) % 2 == 0 ? fLightSquare : fDarkSquare;
            QGraphicsRectItem *square = fScene->addRect(i * fTileWidth, j * fTileWidth, fTileWidth, fTileWidth, Qt::NoPen, QBrush(color));
        }
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

void Renderer::mouseReleaseEvent(QMouseEvent *event) {
    QPointF scenePos = mapToScene(event->pos());

    // Convert the scene position to integer coordinates
    int x = static_cast<int>(scenePos.x());
    int y = static_cast<int>(scenePos.y());

    // Calculate the row and column of the clicked square
    U64 rank = get_rank_from_number(8 - (y / fTileWidth));
    U64 file = get_file_from_number((x / fTileWidth) + 1);
    fEndSquare = rank & file;

    // TODO: Call a function to call the board and make the move
    // also update the GUI based on the new board updates
    U16 move = 0;
    SetMove(move, fStartSquare, fEndSquare);
    fBoard->MakeMove(move);
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