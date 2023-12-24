#include "Renderer.hpp"

Renderer::Renderer() : 
fWindowWidth{800},
fLightColor{sf::Color(255, 206, 158)}, 
fDarkColor{sf::Color(209, 139, 71)}
//fYellowLightColor{sf::Color(245, 235, 138)},
//fYellowDarkColor{sf::Color(216, 196, 100)},
//fHighlightedSquares{} 
{
    fWindow = new sf::RenderWindow(sf::VideoMode(fWindowWidth, fWindowWidth), "Chess Board");
    if (!fFont.loadFromFile("../assets/Roboto-Bold.ttf")) {
        // Handle the case where the font cannot be loaded
    }
    fSquareWidth = (float)fWindowWidth / 8.;
}

Renderer::~Renderer() {
    delete fWindow;
}

void Renderer::Update(const std::unique_ptr<Board> &board) {
    fWindow->clear();
    DrawChessBoard(board);
    fWindow->display();
};

void Renderer::HandlePress(int rank, int file) { // 0--7 range for both
    // Find the rank/file of the pressed square
    // TODO: Highlight attack rays of the piece pressed
    /*for(int n = 0; n < fHighlightedSquares.size(); n++) {
        std::pair<int, int> *square = &fHighlightedSquares[n];
        if(square->first == rank && square->second == file) {
            fHighlightedSquares.erase(fHighlightedSquares.begin() + n);
            return; // Clicking on already highlighted square to de-select it
        }
    }
    fHighlightedSquares.push_back(std::make_pair(rank, file));*/
}

void Renderer::DrawChessBoard(const std::unique_ptr<Board> &board) {
    for (int iFile = 0; iFile < BITS_PER_FILE; ++iFile) {
        for (int iRank = 0; iRank < BITS_PER_FILE; ++iRank) {
            sf::RectangleShape square(sf::Vector2f(fSquareWidth, fSquareWidth));
            square.setPosition(iFile * fSquareWidth, iRank * fSquareWidth);
            square.setFillColor((iRank + iFile) % 2 == 0 ? fLightColor : fDarkColor);

            //for(int n = 0; n < fHighlightedSquares.size(); n++) {
            //    std::pair<int, int> *thisSquare = &fHighlightedSquares[n];
            //    if((7 - j) == thisSquare->first && i == thisSquare->second)
            //        square.setFillColor((i + j) % 2 == 0 ? fYellowLightColor : fYellowDarkColor);
            //}
            fWindow->draw(square);

            if(iFile == 7) {
                // Draw rank numbers on the left side of the board
                sf::Text fileText(std::to_string(8 - iRank), fFont, 18);
                fileText.setPosition(0.1 * fSquareWidth, (iRank + 0.05) * fSquareWidth);
                fileText.setFillColor((iRank + iFile) % 2 == 0 ? fLightColor : fDarkColor);
                fWindow->draw(fileText);
            }

            if(iRank == 7) {
                // Draw file letters at the bottom of the board
                char fileLetter;
                if(iFile == 0) {
                    fileLetter = 'A';
                } else if(iFile == 1) {
                    fileLetter = 'B';
                } else if(iFile == 2) {
                    fileLetter = 'C';
                } else if(iFile == 3) {
                    fileLetter = 'D';
                } else if(iFile== 4) {
                    fileLetter = 'E';
                } else if(iFile == 5) {
                    fileLetter = 'F';
                } else if(iFile == 6) {
                    fileLetter = 'G';
                } else {
                    fileLetter = 'H';
                }
                sf::Text rankText(fileLetter, fFont, 18);
                rankText.setPosition((iFile + 0.8) * fSquareWidth, (iRank + 0.75) * fSquareWidth);
                rankText.setFillColor((iFile + iRank) % 2 == 0 ? fDarkColor : fLightColor);
                fWindow->draw(rankText);
            }
        }
    }

    DrawPieces(board, Piece::Queen);
    DrawPieces(board, Piece::King);
    DrawPieces(board, Piece::Rook);
    DrawPieces(board, Piece::Rook);
    DrawPieces(board, Piece::Bishop);
    DrawPieces(board, Piece::Knight);
    DrawPieces(board, Piece::Pawn);
};

void Renderer::DrawPieces(const std::unique_ptr<Board> &board, Piece piece) {
    // Rooks, bishops, knights, pawns (or queens due to possible promotion)
    U64 white = board->GetBoard(Color::White, piece);
    U64 black = board->GetBoard(Color::Black, piece);

    while(white) {
        U64 pos = 0;
        set_bit(pos, pop_LSB(white));
        DrawChessPiece(piece, Color::White, get_rank_number(pos), get_file_number(pos));
    }
    while(black) {
        U64 pos = 0;
        set_bit(pos, pop_LSB(black));
        DrawChessPiece(piece, Color::Black,  get_rank_number(pos), get_file_number(pos));
    }
}

void Renderer::DrawChessPiece(const Piece piece, const Color color, const int rank, const int file) {
    sf::Texture pieceTexture;
    std::string filename;

    // Determine the filename based on piece type and color
    switch(piece) {
        case Piece::Pawn:
            filename = (color == Color::White) ? "pawn_white.png" : "pawn_black.png";
            break;
        case Piece::Bishop:
            filename = (color == Color::White) ? "bishop_white.png" : "bishop_black.png";
            break;
        case Piece::Knight:
            filename = (color == Color::White) ? "knight_white.png" : "knight_black.png";
            break;
        case Piece::Rook:
            filename = (color == Color::White) ? "rook_white.png" : "rook_black.png";
            break;
        case Piece::Queen:
            filename = (color == Color::White) ? "queen_white.png" : "queen_black.png";
            break;
        case Piece::King:
            filename = (color == Color::White) ? "king_white.png" : "king_black.png";
            break;
        default:
            // Handle an unknown piece type
            std::cout << "Unknown piece type\n";
            return;
    }

    // Load the texture from file
    if (!pieceTexture.loadFromFile("../assets/" + filename)) {
        // Handle the case where the texture cannot be loadeds
        return;
    }

    // Create a sprite and set its position
    sf::Sprite pieceSprite(pieceTexture);
    const float squareSize = static_cast<float>(fWindowWidth) / 8.;
    int posy = 8 - rank;
    int posx = file - 1;

    // Scale the sprite to fit the square, with some additional padding
    // Via the max width/height 
    float maxLength = std::max(pieceTexture.getSize().x, pieceTexture.getSize().y);
    float scale = squareSize * 0.68 / maxLength;
    float xmargin = (squareSize - (pieceTexture.getSize().x * scale)) / 2;
    float ymargin = (squareSize - (pieceTexture.getSize().y * scale)) / 2;

    pieceSprite.setPosition((posx * squareSize) + xmargin, (posy * squareSize) + ymargin);
    pieceSprite.setScale(scale, scale);

    // Draw the piece on the window
    fWindow->draw(pieceSprite);
};
