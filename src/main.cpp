#include <memory>

#include "Move.hpp"
#include "Board.hpp"
#include "Renderer.hpp"
#include "Engine.hpp"
#include "Test.hpp"

bool ProcessCommandLineArgs(const std::vector<std::string>& args,
                            bool &useGUI,
                            bool &doGame,
                            bool &helpRequested,
                            bool &doFinePrint,
                            int &perftDepth,
                            int &playSelf,
                            Color &userColor,
                            std::string &fenString,
                            int &maxDepth) {
    for(uint i = 0; i < args.size(); i++) {
        std::string arg = args[i];
        if(!arg.compare("--no-gui")) {
            useGUI = false;
        } else if(!arg.compare("--perft")) {
            perftDepth = std::stoi(args[i+1]); // TODO: Catch if this is not a valid digit
        } else if(!arg.compare("--verbose")) {
            doFinePrint = true; ///< Print all the moves generated when perft testing, useful for debugging
        } else if(!arg.compare("--help")) {
            helpRequested = true;
        } else if(!arg.compare("--fen")) {
            fenString = args[i+1];
        } else if(!arg.compare("--play")) {
            doGame = true;
        } else if(!arg.compare("--play-self")) {
            playSelf = std::stoi(args[i+i]);
        } else if(!arg.compare("--color")) {
            userColor = !args[i+1].compare("black") ? Color::Black : Color::White; // TODO: Catch if this is not a valid
        } else if(!arg.compare("--depth")) {
            maxDepth = std::stoi(args[i+1]);
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
              << "  --play-self <n>     Make the computer play against itself n times and print the outcomes.\n"
              << "  --depth <n>         The maximum depth the computer should search to, exponentially increases runtime.\n"
              << "  --verbose           Prints every move generated at the highest search depth when performing a perft test.\n"
              << "  --color <colour>    Specify the colour of the human player e.g. \"white\" or \"black\". If not provided will default to white.\n\n"
              << "Examples:\n"
              << "  ChessEngine --perft 5 --fen \"rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1\" --no-gui\n"
              << "  ChessEngine --play\n";
}

void PlaySelf(int nGames, int depth) {
    int whiteWins = 0;
    int blackWins = 0;
    int stalemates = 0;
    int materialDraw = 0;
    int fiftyMoveDraw = 0;

    const std::unique_ptr<Board> board = std::make_unique<Board>();
    const std::unique_ptr<Generator> generator = std::make_unique<Generator>();
    const std::unique_ptr<Engine> engine = std::make_unique<Engine>(generator, board, depth);

    for(int iGame = 0; iGame < nGames; ++iGame) {
        board->Reset();

        float percentage = 100. * ((float)iGame + 1.) / (float)nGames;
        std::cout << "Percentage completed " << percentage << "% [" << iGame + 1 << "/" << nGames << "]\n";

        while(board->GetState() == State::Play) {
            generator->GenerateLegalMoves(board);
            if(generator->GetNLegalMoves() == 0)
                break;

            U16 move{0};
            if(board->GetColorToMove() == Color::White) {
                move = engine->GetBestMove(false); 
            } else {
                move = engine->GetRandomMove(); // For now black is a random agent
            }
            
            board->MakeMove(move);
        }

        if(board->GetState() == State::Checkmate) {
            std::string winningColour = board->GetColorToMove() == Color::White ? "Black" : "White";
            Color wonColour = board->GetColorToMove() == Color::White ? Color::Black : Color::White;
            if(wonColour == Color::White) {
                whiteWins++;
            } else {
                blackWins++;
            }
        } else if(board->GetState() == State::Stalemate) {
            stalemates++;
        } else if(board->GetState() == State::FiftyMoveRule) {
            fiftyMoveDraw++;
        } else if(board->GetState() == State::InSufficientMaterial) {
            materialDraw++;
        }
    }

    std::cout << "\n========== Summary ==========\n";
    std::cout << "Checkmates by white:            " << whiteWins << "\n";
    std::cout << "Checkmates by black:            " << blackWins << "\n";
    std::cout << "Draws by stalemate:             " << stalemates << "\n";
    std::cout << "Draws by 50-move rule:          " << fiftyMoveDraw << "\n";
    std::cout << "Draws by insufficient material: " << materialDraw << "\n";
}

void Play(const std::string &/*fen*/, Color /*userColor*/, int /*depth*/) {
    //const std::unique_ptr<Board> board = std::make_unique<Board>(); // Initalise the main game board (game state handling)
    //const std::unique_ptr<Renderer> gui = std::make_unique<Renderer>(); // For handling the GUI
    //const std::unique_ptr<Generator> generator = std::make_unique<Generator>();
    //const std::unique_ptr<Engine> engine = std::make_unique<Engine>(generator, board, depth);

    // If the FEN exists load the board with the FEN
    //if(fen.size() > 0)
    //    board->LoadFEN(fen);

    //gui->Update(board); // Draw board initially
    /*
    while(gui->GetWindowIsOpen()) {
        sf::Event event;
        while(gui->PollEvent(event)) {
            if(event.type == sf::Event::Closed) {
                gui->CloseWindow();
            }
        }
        // For now, don't use GUI do it all in the command line - later use a GUI
        generator->GenerateLegalMoves(board);
        while(board->GetState() == State::Play) {
            U16 move{0};
            if(board->GetColorToMove() == userColor) {
                // Human player chooses a move
                move = gui->ReadUserMove(); // Reads the users console input and translates into a move
                while(!generator->GetMoveIsLegal(move)) { // Protection against illegal user moves
                    std::cout << "[Warning] Illegal move entered. Please enter a valid move.\n";
                    move = gui->ReadUserMove();
                }
            } else {
                // Compcuter chooses a move
                //move = engine->GetRandomMove();
                move = engine->GetBestMove(true);
                // Print out the move to the console
                board->PrintDetailedMove(move);
            }
            
            // Make the move
            board->MakeMove(move);

            // Update the GUI
            gui->Update(board);

            // Updates checkmate states to potentially get an early exit
            generator->GenerateLegalMoves(board);
        }

        std::cout << "Game terminated normally in state " << (int)board->GetState() << "\n";
        board->PrintFEN();
        gui->CloseWindow();
    }
    */
}

int main(int argc, char* argv[]) {

    bool useGUI = true; 
    bool doGame = false;
    bool helpRequested = false;
    bool doFinePrint = false;
    int perftDepth = 0;
    int maxDepth = 4;
    int playSelf = 0;
    Color userColor = Color::White;
    std::string fenString = "";

    std::vector<std::string> args(argv, argv + argc);
    ProcessCommandLineArgs(args, useGUI, doGame, helpRequested, doFinePrint, perftDepth, playSelf, userColor, fenString, maxDepth);

    if(helpRequested) {
        DisplayHelp();
    } else if(perftDepth > 0) {
        Test myTest = Test(useGUI);
        unsigned long int result = myTest.GetNodes(perftDepth, fenString, doFinePrint);
        std::cout << "\nNodes searched: " << result << "\n";
    } else if(doGame) {
        Play(fenString, userColor, maxDepth);
    } else if(playSelf != 0) {
        PlaySelf(playSelf, maxDepth);
    } else {
        std::unique_ptr<Board> b = std::make_unique<Board>();
        QApplication app(argc, argv);
        Renderer window(b);
        window.setWindowTitle("Chess Engine");
        window.show();

        window.DrawPieces();

        return app.exec();
    }
    return 0;
}
