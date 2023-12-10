#include "Game.hpp"

int main() {
    // Use unique/shared pointers "samrt pointers" to cleanup memory management of complex objects
    /*std::unique_ptr<Board> board = std::make_unique<Board>(); // Provide args inside () if needed
    std::unique_ptr<Engine> engine = std::make_unique<Engine>(true);

    std::string myFen = "rnb1k1nr/ppp2ppp/8/1q1pp3/3BP3/NP1P1N2/P1PQ1PPP/R3K2R w KQkq -";
    bool success = board->LoadFEN(myFen);
    if(!success)
        return 0;
    
    PrintBitset(board->GetBoard(Color::White) | board->GetBoard(Color::Black));
    
    // Make castling moves ONLY
    board->GenerateLegalMoves();
    std::vector<Move> moves = board->GetLegalMoves();
    if(moves.size() > 0) {
        std::cout << "Origin = " << get_rank_number(moves[0].origin) << " " << get_file_number(moves[0].origin) << "\n";
        std::cout << "Target = " << get_rank_number(moves[0].target) << " " << get_file_number(moves[0].target) << "\n";
        board->MakeMove(moves[0]);
    }

    PrintBitset(board->GetBoard(Color::White) | board->GetBoard(Color::Black));

    return 1;*/

    Game myGame = Game();
    myGame.Play(Color::White);
}

Game::Game() {
    fBoard = std::make_unique<Board>();
    fEngine = std::make_unique<Engine>(true);
}

void Game::Play(Color playerColor) {
    while(!fBoard->GetGameIsOver()) {
        Move thisMove;
        fBoard->GenerateLegalMoves(); // Updates legal moves internally
        if(fBoard->GetColorToMove() == playerColor) { // Player makes a move
            while(!fBoard->GetMoveIsLegal(&thisMove)) {
                thisMove = GetUserMove();
            }
        } else { // Engine makes a move
            std::cout << "Making engine move [Doesn't work yet]\n";
            thisMove = fEngine->GetBestMove();
        }
        fBoard->MakeMove(thisMove);
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
            
    }

    // Other move parameters here
    PrintBitset(userMove.origin);

    return userMove;
}