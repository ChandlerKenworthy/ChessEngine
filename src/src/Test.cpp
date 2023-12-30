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

unsigned long int Test::MoveGeneration(int depth, bool useGUI) {
    if(useGUI) {
        while(fGUI->GetWindowIsOpen()) {

            unsigned long int numPositions = 0;
            if(depth == 0)
                return 1;

            fEngine->GenerateLegalMoves(fBoard);
            std::string uIn = "";
            std::vector<U32> moves = fEngine->GetLegalMoves();

            for(int iMove = 0; iMove < moves.size(); iMove++) {
                U32 move = moves.at(iMove);

                if(depth == 1) {
                    while(uIn.compare("n")) {
                        std::cout << "Awaiting 'n' command to continue: ";
                        std::getline(std::cin, uIn);
                    }
                }

                if(depth == 2) {

                }

                fBoard->MakeMove(move);

                //std::cout << "Depth = " << 4 - depth << " Nodes = " << numPositions << " ";
                //PrintMove(move);
                fGUI->Update(fBoard);

                

                numPositions += MoveGeneration(depth - 1, true);
                fBoard->UndoMove();
            }
            return numPositions;
        }
    } else {
        if(depth == 0)
            return 1;

        fEngine->GenerateLegalMoves(fBoard);
        unsigned long int numPositions = 0;
        unsigned long int subPositions = 0;

        const int printDepth = 7;

        std::vector<U32> moves = fEngine->GetLegalMoves();
        if(depth == printDepth)
            std::cout << "Parent nodes to search = " << moves.size() << "\n";

        for(int iMove = 0; iMove < moves.size(); iMove++) {
            U32 move = moves.at(iMove);
            if(depth == printDepth) {
                PrintMove(move); 
                subPositions = numPositions;
            }

            //if(depth == 1) {
            //    std::cout << "\n depth 1 move = ";
            //    PrintMove(move);
            //}
            fBoard->MakeMove(move);
            numPositions += MoveGeneration(depth - 1, false);
            if(depth == printDepth) {
                std::cout << ": " << numPositions - subPositions << "\n";
            }
            fBoard->UndoMove();
        }

        return numPositions;
    }

    return -1;
}