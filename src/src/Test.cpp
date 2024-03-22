#include "Test.hpp"

Test::Test(bool useGUI) {
    fBoard = std::make_unique<Board>();
    fGenerator = std::make_unique<Generator>();
    fUseGUI = useGUI;
    fDoFinePrint = false;
    //if(fUseGUI)
    //    fGUI = std::make_unique<Renderer>();
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

unsigned long int Test::GetNodes(int depth, std::string fen, bool doFinePrint) {
    fDoFinePrint = doFinePrint;
    auto start = std::chrono::high_resolution_clock::now(); // Time the execution
    if(fen.length() > 0)
        fBoard->LoadFEN(fen);
    SetPrintDepth(depth);
    unsigned long moves = MoveGeneration(depth);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Searched complete node tree in " << duration.count() << " microseconds\n";
    return moves;
}

unsigned long int Test::MoveGeneration(int depth) {
    if(depth == 0)
        return 1;

    fGenerator->GenerateLegalMoves(fBoard);
    unsigned long int numPositions = 0;
    unsigned long int subPositions = 0;

    std::vector<U16> moves = fGenerator->GetLegalMoves(); // Puts moves inside this vector for you

    if(depth == fPrintDepth)
        std::cout << "Parent nodes searched: " << moves.size() << "\n";

    for(std::size_t iMove = 0; iMove < moves.size(); iMove++) {
        U16 move = moves.at(iMove);
        if(depth == fPrintDepth) {
            PrintMove(move); 
            subPositions = numPositions;
        }
        if(depth == 1 && fDoFinePrint) {
            std::cout << "\n depth 1 move = ";
            PrintMove(move);
        }
        fBoard->MakeMove(move);
        numPositions += MoveGeneration(depth - 1);
        if(depth == fPrintDepth) {
            std::cout << ": " << numPositions - subPositions << "\n";
        }
        fBoard->UndoMove();
    }

    return numPositions;
}