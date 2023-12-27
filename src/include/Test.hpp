/**
 * @file Test.hpp
 * @brief Definition of the Board class.
 */


#ifndef TEST_HPP
#define TEST_HPP

#include <vector>

#include "Constants.hpp"
#include "Move.hpp"
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
        int MoveGeneration(int depth);
        unsigned long int GetExpectedGeneration(int depth) { return depth < fExpectedGeneration.size() ? fExpectedGeneration[depth] : 0; };
    private:
        std::unique_ptr<Board> fBoard;
        std::unique_ptr<Engine> fEngine;
        std::vector<unsigned long int> fExpectedGeneration; ///< Total number of possible moves after each depth level
};

#endif