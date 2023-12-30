/**
 * @file Test.hpp
 * @brief Definition of the Board class.
 */


#ifndef TEST_HPP
#define TEST_HPP

#include <vector>

#include "Constants.hpp"
#include "Move.hpp"
#include "Renderer.hpp"
#include "Engine.hpp"
#include "Board.hpp"

/**
 * @class Test
 * @brief Class used to run tests on the engine.
 * 
 */ 
class Test {
    public:
        explicit Test();
        /**
         * @brief Generate the number of moves possible up to the specified depth. Can compare with literature (fExpectedGeneration).
         *
         * If operating with useGUI on then each move will be made the GUI updated and the loop paused until the user enters N in the console ("next") thereby checking each moves validity slowly.
        */
        unsigned long int MoveGeneration(int depth, bool useGUI);
        unsigned long int GetExpectedGeneration(int depth) { return depth < fExpectedGeneration.size() ? fExpectedGeneration[depth] : 0; };
        const std::unique_ptr<Board>& GetBoard() { return fBoard; };
    private:
        std::unique_ptr<Board> fBoard;
        std::unique_ptr<Engine> fEngine;
        std::unique_ptr<Renderer> fGUI;
        std::vector<unsigned long int> fExpectedGeneration; ///< Total number of possible moves after each depth level
};

#endif