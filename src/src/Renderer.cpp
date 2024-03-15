#include "Renderer.hpp"

Renderer::Renderer(QWidget *parent) : fTileWidth(70), fLightSquare(255, 206, 158), fDarkSquare(209, 139, 71), fScene(new QGraphicsScene) {
    fPieceHeight = fTileWidth * 0.8;

    QVBoxLayout *layout = new QVBoxLayout(this);

    view = new QGraphicsView(fScene);
    view->setRenderHint(QPainter::Antialiasing);
    layout->addWidget(view);
    
    // Max Depth
    QHBoxLayout *depthLayout = new QHBoxLayout;
    QLabel *depthLabel = new QLabel("Max Depth:", this);
    depthSpinBox = new QSpinBox(this);
    depthLayout->addWidget(depthLabel);
    depthLayout->addWidget(depthSpinBox);
    
    // Max Time
    QHBoxLayout *timeLayout = new QHBoxLayout;
    QLabel *timeLabel = new QLabel("Max Time (seconds):", this);
    timeSpinBox = new QDoubleSpinBox(this);
    timeSpinBox->setDecimals(1); // Adjust as needed
    timeLayout->addWidget(timeLabel);
    timeLayout->addWidget(timeSpinBox);
    
    // Engine Version
    QHBoxLayout *versionLayout = new QHBoxLayout;
    QLabel *versionLabel = new QLabel("Engine Version:", this);
    versionComboBox = new QComboBox(this);
    versionComboBox->addItem("Version 1");
    versionComboBox->addItem("Version 2");
    versionLayout->addWidget(versionLabel);
    versionLayout->addWidget(versionComboBox);
    
    // Add layouts to main layout
    layout->addLayout(depthLayout);
    layout->addLayout(timeLayout);
    layout->addLayout(versionLayout);
    
    // Add buttons
    QPushButton *startButton = new QPushButton("Start", this);
    connect(startButton, &QPushButton::clicked, this, &Renderer::startSearch);
    layout->addWidget(startButton);
    
    // Set up chessboard
    DrawChessBoard();
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

void Renderer::DrawPieces(const std::unique_ptr<Board> &board) {
    for(Piece p : PIECES) {
        U64 whitePieces = board->GetBoard(Color::White, p);
        U64 blackPieces = board->GetBoard(Color::Black, p);

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

            int posX = ((file - 1) * fTileWidth) + ((fTileWidth - scaledWidth) / 2);
            int posY = ((8 - rank) * fTileWidth) + ((fTileWidth - fPieceHeight) / 2);
            chessPiece->setPos(posX, posY);
            blackPieces &= blackPieces - 1;
        }
    }
}