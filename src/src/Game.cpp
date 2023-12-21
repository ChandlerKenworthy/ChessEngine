#include "Game.hpp"

Game::Game(bool useGUI) {
    fBoard = Board();
    fEngine = std::make_unique<Engine>(true);
    fEngine->SetMaxDepth(10);
    fUseGUI = useGUI;
    fGUI = std::make_unique<Renderer>();
}

void Game::PrintEngineMove(Move move) {
    std::cout << "Engine made the move file = ";
    std::cout << get_file_number(move.origin) << " rank = ";
    std::cout << get_rank_number(move.origin) << " which was a ";
    std::cout << GetPieceString(move.piece) << " to square file = ";
    std::cout << get_file_number(move.target) << " rank = ";
    std::cout << get_rank_number(move.target) << "\n";
}

std::string Game::GetPieceString(Piece piece) {
    switch(piece) {
    case Piece::Pawn:
        return "Pawn";
        break;
    case Piece::Bishop:
        return "Bishop";
        break;
    case Piece::Knight:
        return "Knight";
        break;
    case Piece::Rook:
        return "Rook";
        break;
    case Piece::Queen:
        return "Queen";
        break;
    case Piece::King:
        return "King";
        break;
    default:
        return "Error piece does not exist";
        break;
    }
}

void Game::Play(Color playerColor) {
    if(fUseGUI) {
        while(fGUI->GetWindowIsOpen()) {
            sf::Event event;
            while(fGUI->PollEvent(event)) {
                if(event.type == sf::Event::Closed) {
                    fGUI->CloseWindow();
                }
            }

            while(!fBoard.GetGameIsOver()) {
                Move thisMove;
                fBoard.GenerateLegalMoves(); // Updates legal moves internally
                fGUI->Update(&fBoard); // Clear, draws and updates the board
                if(fBoard.GetColorToMove() == playerColor) { // Player makes a move
                    while(!fBoard.GetMoveIsLegal(&thisMove)) {
                        thisMove = GetUserMove();
                    }
                } else { // Engine makes a move
                    thisMove = fEngine->GetBestMove(fBoard);
                    PrintEngineMove(thisMove);
                }
                fBoard.MakeMove(thisMove);
            }
        }
    } else {
        while(!fBoard.GetGameIsOver()) {
            Move thisMove;
            fBoard.GenerateLegalMoves(); // Updates legal moves internally
            PrintBitset(fBoard.GetBoard(Color::White) | fBoard.GetBoard(Color::Black));
            if(fBoard.GetColorToMove() == playerColor) { // Player makes a move
                while(!fBoard.GetMoveIsLegal(&thisMove)) {
                    thisMove = GetUserMove();
                }
            } else { // Engine makes a move
                thisMove = fEngine->GetBestMove(fBoard);
                PrintEngineMove(thisMove);
            }
            fBoard.MakeMove(thisMove);
        }
    }


    while(!fBoard.GetGameIsOver()) {
        Move thisMove;
        fBoard.GenerateLegalMoves(); // Updates legal moves internally
        PrintBitset(fBoard.GetBoard(Color::White) | fBoard.GetBoard(Color::Black));

        if(fBoard.GetColorToMove() == playerColor) { // Player makes a move
            while(!fBoard.GetMoveIsLegal(&thisMove)) {
                thisMove = GetUserMove();
            }
        } else { // Engine makes a move
            thisMove = fEngine->GetBestMove(fBoard);
            PrintEngineMove(thisMove);
        }
        fBoard.MakeMove(thisMove);
    }
}

Move Game::GetUserMove() {
    // Returns true when read successfully and false otherwise
    bool isValidOrigin = false;
    bool isValidTarget = false;
    bool gettingOrigin = true;
    const std::regex pattern("^[A-Ha-h][1-8]$");
    Move userMove;
    std::string userText;

    while(!(isValidOrigin && isValidTarget)) {
        std::string label = gettingOrigin ? "origin" : "target";
        std::cout << "Enter " << label << " tile (e.g. A1): ";
        getline(std::cin, userText);
        // Shift the user text to uppercase only
        std::transform(userText.begin(), userText.end(), userText.begin(), ::toupper);
        if(!userText.compare("EXIT"))
            fGUI->CloseWindow();
        if(std::regex_match(userText, pattern)) {
            if(gettingOrigin) {
                isValidOrigin = true;
                userMove.origin = get_rank_from_number(userText.back() - '0') & get_file_from_number((userText.at(0) - 'A') + 1);
                gettingOrigin = false;
            } else {
                isValidTarget = true;
                userMove.target = get_rank_from_number(userText.back() - '0' ) & get_file_from_number((userText.at(0) - 'A') + 1);
            }
        }
            
    } // No information about taking, castling en-passant, filled in via GetMoveIsLegal
    return userMove;
}