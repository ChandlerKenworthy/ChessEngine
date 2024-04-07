#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "Constants.hpp"
#include "Board.hpp"
//#include "Generator.hpp" // Include your existing C++ header file

namespace py = pybind11;

PYBIND11_MODULE(chess_engine, m) {
    py::enum_<Color>(m, "Color")
        .value("WHITE", Color::White)
        .value("BLACK", Color::Black)
        .export_values();

    py::enum_<Piece>(m, "Piece")
        .value("NULL", Piece::Null)
        .value("PAWN", Piece::Pawn)
        .value("KNIGHT", Piece::Knight)
        .value("BISHOP", Piece::Bishop)
        .value("ROOK", Piece::Rook)
        .value("QUEEN", Piece::Queen)
        .value("KING", Piece::King);

    py::enum_<State>(m, "State")
        .value("PLAY", State::Play)
        .value("STALEMATE", State::Stalemate)
        .value("INSUFFICIENTMATERIAL", State::InSufficientMaterial)
        .value("MOVEREPITITION", State::MoveRepetition)
        .value("FIFTYMOVERULE", State::FiftyMoveRule)
        .value("CHECKMATE", State::Checkmate);

    py::class_<Board>(m, "Board")
        .def(py::init<>())
        .def("get_hash", &Board::GetHash)
        .def("get_color_to_move", &Board::GetColorToMove)
        .def("load_fen", &Board::LoadFEN)
        .def("get_n_moves", &Board::GetNMoves)
        .def("reset", &Board::Reset)
        .def("make_move", &Board::MakeMove)
        .def("undo_move", &Board::UndoMove)
        .def("get_board_color_piece", py::overload_cast<const Color, const Piece>(&Board::GetBoard), "Get the bitboard for a particular piece type and colour.", py::arg("color"), py::arg("piece"));
}
