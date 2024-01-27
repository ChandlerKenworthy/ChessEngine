#include <memory>

#include "Move.hpp"
#include "Board.hpp"
#include "Renderer.hpp"
#include "Engine.hpp"
#include "Test.hpp"

const std::string VERSION = "v1.6.1";

bool ProcessCommandLineArgs(const std::vector<std::string>& args,
                            bool &useGUI,
                            bool &doGame,
                            bool &helpRequested,
                            int &perftDepth,
                            Color &userColor,
                            std::string &fenString) {
    for(int i = 0; i < args.size(); i++) {
        std::string arg = args[i];
        if(!arg.compare("--no-gui")) {
            useGUI = false;
        } else if(!arg.compare("--perft")) {
            perftDepth = std::stoi(args[i+1]); // TODO: Catch if this is not a valid digit
        } else if(!arg.compare("--help")) {
            helpRequested = true;
        } else if(!arg.compare("--fen")) {
            fenString = args[i+1];
        } else if(!arg.compare("--play")) {
            doGame = true;
        } else if(!arg.compare("--color")) {
            userColor = !args[i+1].compare("black") ? Color::Black : Color::White; // TODO: Catch if this is not a valid
        }
    }

    return true;
}

void DisplayHelp() {
    std::cout << "Usage: ChessEngine [options]\n\n"
              << "Description:\n"
              << "  A static evaluation based chess engine. You can play against the computer or get the best moves from given positions.\n\n"
              << "Options:\n"
              << "  --no-gui            Run the program without a graphical user interface.\n"
              << "  --perft <depth>     Perform a perft test up to the specified depth. Depths >= 7 can take a significant time to compute depending on the positions complexity.\n"
              << "  --fen <fen>         Specify an initial position for the engine to perform perft tests or play against using the standard FEN notation.\n"
              << "  --play              Play a game of user versus the computer. The engine will play the best move.\n"
              << "  --color <colour>    Specify the colour of the human player e.g. \"white\" or \"black\". If not provided will default to white.\n\n"
              << "Examples:\n"
              << "  ChessEngine --perft 5 --fen \"rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1\" --no-gui\n"
              << "  ChessEngine --play\n";
}

void Play(const std::string &fen, Color userColor) {
    const std::unique_ptr<Board> board = std::make_unique<Board>(); // Initalise the main game board (game state handling)
    const std::unique_ptr<Renderer> gui = std::make_unique<Renderer>(); // For handling the GUI
    const std::unique_ptr<Engine> engine = std::make_unique<Engine>(true); // For handling the chess engine

    // If the FEN exists load the board with the FEN
    if(fen.size() > 0)
        board->LoadFEN(fen);

    // For now, don't use GUI do it all in the command line - later use a GUI
    while(board->GetState() == State::Play) {
        engine->GenerateLegalMoves(board);
        U32 move{0};
        if(board->GetColorToMove() == userColor) {
            // Human player chooses a move
            move = gui->ReadUserMove(); // Reads the users console input and translates into a move
            while(!engine->GetMoveIsLegal(&move)) { // Protection against illegal user moves
                std::cout << "[Warning] Illegal move entered. Please enter a valid move.\n";
                move = gui->ReadUserMove();
            }
        } else {
            // Computer chooses a move
            move = engine->GetRandomMove();
            // Print out the move to the console
            board->PrintDetailedMove(move);
        }
        
        // Make the move
        board->MakeMove(move);
    }

    /*
    std::pair<Color, Piece> selectedPiece = std::make_pair(Color::White, Piece::Null);
    U64 pieceTile = 0;
    U64 selectedTile = 0;

    bool makeLegalMove = false;
    U32 userMove;
    
    gui->Update(b); // Draw board initially

    while(gui->GetWindowIsOpen()) {
        sf::Event event;
        while(gui->PollEvent(event)) {
            if(event.type == sf::Event::Closed) {
                gui->CloseWindow();
            } else if(event.type == sf::Event::MouseButtonPressed) {
                selectedTile = gui->GetClickedSquare(event);
                // User clicked on a piece (not an empty square)
                std::pair<Color, Piece> clickedPosition = b->GetIsOccupied(selectedTile);
                if(clickedPosition.second != Piece::Null && clickedPosition.first == b->GetColorToMove()) { // set-bit on the bitboard of occupation
                    selectedPiece = clickedPosition;
                    pieceTile = selectedTile;
                } else { // User clicked on an empty square/enemy piece
                    if(selectedPiece.second != Piece::Null) {
                        engine->GenerateLegalMoves(b);
                        if(engine->GetNLegalMoves() == 0) { // Game is either a stalemate or checkmate
                            std::cout << "Game ending condition met\n";
                        } else {
                            SetMove(userMove, pieceTile, selectedTile, selectedPiece.second, Piece::Null);
                            if(engine->GetMoveIsLegal(&userMove)) {
                                makeLegalMove = true;
                            } else {
                                std::cout << "Move was found to be illegal!\n";
                            }
                        }
                    }
                    selectedPiece = std::make_pair(Color::White, Piece::Null);
                    pieceTile = 0;
                }
                // Find which piece (if any) the user clicked on
                // or if already clicked a piece drop this piece at the tile clicked on
            } else if(event.type == sf::Event::KeyPressed) {
                if(event.key.code == sf::Keyboard::B) {
                    SetMovePromotionPiece(userMove, Piece::Bishop);
                } else if(event.key.code == sf::Keyboard::R) {
                    SetMovePromotionPiece(userMove, Piece::Rook);
                } else if(event.key.code == sf::Keyboard::N) {
                    SetMovePromotionPiece(userMove, Piece::Knight);
                } else { // Default to queen promotion
                    SetMovePromotionPiece(userMove, Piece::Queen);
                }
            } // TODO: When pressing a key after a legal move is ready it tries to make a null move then swithces possession. 
            // Still getting the "We got a keyboard without any keys." issue as well currently...

            if(makeLegalMove) {
                b->MakeMove(userMove);
                gui->Update(b); // Only update GUI when a move was actually made

                // Clear variables
                makeLegalMove = false;
                userMove = 0;
            }
        }
    }*/
}

int main(int argc, char* argv[]) {

    bool useGUI = true; 
    bool doGame = false;
    bool helpRequested = false;
    int perftDepth = 0;
    Color userColor = Color::White;
    std::string fenString = "";

    std::vector<std::string> args(argv, argv + argc);
    bool success = ProcessCommandLineArgs(args, useGUI, doGame, helpRequested, perftDepth, userColor, fenString);

    if(helpRequested) {
        DisplayHelp();
    } else if(perftDepth > 0) {
        Test myTest = Test(useGUI);
        unsigned long int result = myTest.GetNodes(perftDepth, fenString);
        std::cout << "\nNodes searched: " << result << "\n";
    } else if(doGame) {
        Play(fenString, userColor);
    }

    return 0;
}
