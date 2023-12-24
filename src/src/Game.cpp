#include "Game.hpp"
#include "Renderer.hpp"

Game::Game(bool useGUI, int maxDepth) {
    fBoard = Board();
    fEngine = std::make_unique<Engine>(true);
    fEngine->SetMaxDepth(maxDepth);
    fUseGUI = useGUI;
    fGUI = std::unique_ptr<Renderer>();
    fIsDragging = false;
}

void Game::PrintEngineMove(Move move) {
    std::cout << "Engine made the move file = ";
    std::cout << get_file_number(move.origin) << " rank = ";
    std::cout << get_rank_number(move.origin) << " which was a ";
    std::cout << GetPieceString(move.piece) << " to square file = ";
    std::cout << get_file_number(move.target) << " rank = ";
    std::cout << get_rank_number(move.target) << "\n";
}

void Game::Play(Color playerColor) {
    if(fUseGUI) {
        std::cout << "Ye mam 1 \n";
        while(fGUI->GetWindowIsOpen()) {
            sf::Event event;
            while(fGUI->PollEvent(event)) {
                if(event.type == sf::Event::Closed) {
                    fGUI->CloseWindow();
                } else if(event.type == sf::Event::MouseButtonPressed) {
                    int rank = 8 - (event.mouseButton.y / fGUI->GetSquareSize()); // 0->7 range
                    int file = event.mouseButton.x / fGUI->GetSquareSize(); // 0->7 range
                    U64 square = get_rank_from_number(rank + 1) & get_file_from_number(file + 1);
                    if(fBoard.GetIsOccupied(square).second != Piece::Null) {
                        fIsDragging = true;
                        fDraggedPiece = fBoard.GetIsOccupied(square);
                    }
                    fGUI->HandlePress(rank, file);
                } else if(event.type == sf::Event::MouseButtonReleased) {
                    //fGUI->DropPiece(fDraggedPiece);
                    fIsDragging = false;
                } else if(event.type == sf::Event::MouseMoved && fIsDragging) {
                    // Move the piece currently being dragged
                }
            }
            fGUI->Update(&fBoard);
            /*while(!fBoard.GetGameIsOver()) {
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
            }*/
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
}

Move Game::GetUserMove() {
    const std::regex pattern("^[A-Ha-h][1-8]$");

    Move userMove;
    bool wasBackOrExit = false;
    bool gettingOrigin = true;

    auto convertUserInputToCoords = [](const std::string& input) -> int {
        return get_rank_from_number(input.back() - '0') & get_file_from_number((input.at(0) - 'A') + 1);
    };

    while(!(userMove.origin && userMove.target) && !wasBackOrExit) { // Need valid origin, target and not back/exit command
        std::string userText;
        std::cout << "Enter " << (gettingOrigin ? "origin" : "target") << ": ";
        getline(std::cin, userText);
        std::transform(userText.begin(), userText.end(), userText.begin(), ::toupper);
        if(!userText.compare("EXIT")) {
            fGUI->CloseWindow();
            wasBackOrExit = true;
        } else if (userText == "BACK") {
            wasBackOrExit = HandleBackCommand(userMove);
        } else if (std::regex_match(userText, pattern)) {
            if(gettingOrigin) {
                userMove.origin = convertUserInputToCoords(userText);
                gettingOrigin = false;
            } else {
                userMove.target = convertUserInputToCoords(userText);
                gettingOrigin = true;
            }
        }
    }
    // No information about taking, castling en-passant, filled in via GetMoveIsLegal
    return userMove;
}

bool Game::HandleBackCommand(Move& userMove) {
    fBoard.UndoMove();
    fBoard.GenerateLegalMoves();
    fGUI->Update(&fBoard);
    return true;
}
