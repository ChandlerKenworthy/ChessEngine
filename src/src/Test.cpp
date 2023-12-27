#include "Test.hpp"

Test::Test() {
    fBoard = std::make_unique<Board>();
    fEngine = std::make_unique<Engine>(true);

    fExpectedGeneration = {
        1,
        20,
        400,
        8902,
        197281,
        4865609,
        119060324,
        3195901860
    };
}

int Test::MoveGeneration(int depth) {
    if(depth == 0)
        return 1;

    fEngine->GenerateLegalMoves(fBoard);
    int numPositions = 0;

    std::vector<U32> *moves = fEngine->GetLegalMoves();

    for(int iMove = 0; iMove < moves->size(); iMove++) {
        U32 move = moves->at(iMove);
        fBoard->MakeMove(move);
        numPositions += MoveGeneration(depth - 1);
        fBoard->UndoMove();
    }

    return numPositions;
}