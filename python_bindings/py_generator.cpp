#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "Board.hpp"
//#include "Generator.hpp" // Include your existing C++ header file

namespace py = pybind11;

int add(int i, int j) {
    return i + j;
}

PYBIND11_MODULE(chess_engine, m) {
    m.def("add", &add, "adds two numbers");
    py::class_<Board>(m, "Board")
        .def(py::init<>());
        //.def("make_move", &Board::MakeMove)
        //.def("undo_move", &Board::UndoMove);
}
