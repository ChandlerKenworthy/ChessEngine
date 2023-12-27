#include "Test.hpp"

Test::Test() {
    fBoard = std::make_unique<Board>();
    fEngine = std::make_unique<Engine>(true);
    fGUI = std::make_unique<Renderer>();

    fExpectedGeneration = {
        1,
        20,
        400,
        8902,
        197281,
        4865609,
        119060324,
        3195901860,
        84998978956,
        2439530234167,
    };
}

int Test::MoveGeneration(int depth, bool useGUI) {
    if(useGUI) {
        int maxDepth = depth;
        while(fGUI->GetWindowIsOpen()) {
            sf::Event event;
            while(fGUI->PollEvent(event)) {
                if(event.type == sf::Event::Closed) {
                    fGUI->CloseWindow();
                }
            }


            if(depth == 0)
                return 1;

            fEngine->GenerateLegalMoves(fBoard);
            int numPositions = 0;
            std::string uIn = "";

            std::vector<U32> *moves = fEngine->GetLegalMoves();

            for(int iMove = 0; iMove < moves->size(); iMove++) {
                U32 move = moves->at(iMove);
                fBoard->MakeMove(move);
                fGUI->Update(fBoard);
                if(depth == 1) {
                    while(uIn.compare("next")) {
                        std::cout << "Awaiting 'next' command to continue: ";
                        std::getline(std::cin, uIn);
                    }
                }

                numPositions += MoveGeneration(depth - 1, true);
                fBoard->UndoMove();
            }
            return numPositions;
        }
    } else {
        if(depth == 0)
            return 1;

        fEngine->GenerateLegalMoves(fBoard);
        int numPositions = 0;

        std::vector<U32> *moves = fEngine->GetLegalMoves();

        for(int iMove = 0; iMove < moves->size(); iMove++) {
            U32 move = moves->at(iMove);
            fBoard->MakeMove(move);
            numPositions += MoveGeneration(depth - 1, false);
            fBoard->UndoMove();
        }

        return numPositions;
    }

    return -1;
}