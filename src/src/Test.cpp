#include "Test.hpp"

Test::Test() {
    fBoard = std::make_unique<Board>();
    fEngine = std::make_unique<Engine>(true);
    fGUI = std::make_unique<Renderer>();
    fPrintDepth = 999;

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

unsigned long int Test::MoveGeneration(int depth) {
    if(depth == 0)
        return 1;

    fEngine->GenerateLegalMoves(fBoard);
    unsigned long int numPositions = 0;
    unsigned long int subPositions = 0;

    std::vector<U32> moves = fEngine->GetLegalMoves();
    if(depth == fPrintDepth)
        std::cout << "Parent nodes to search = " << moves.size() << "\n";

    for(int iMove = 0; iMove < moves.size(); iMove++) {
        U32 move = moves.at(iMove);
        if(depth == fPrintDepth) {
            PrintMove(move); 
            subPositions = numPositions;
        }

        //if(depth == 1) {
        //    std::cout << "\n depth 1 move = ";
        //    PrintMove(move);
        //}
        fBoard->MakeMove(move);
        numPositions += MoveGeneration(depth - 1);
        if(depth == fPrintDepth) {
            std::cout << ": " << numPositions - subPositions << "\n";
        }
        fBoard->UndoMove();
    }

    return numPositions;
}