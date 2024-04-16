#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "Constants.hpp"
#include "Board.hpp"
#include "Generator.hpp"
#include "Engine.hpp"

namespace py = pybind11;

void PrintBoard(Board *board) {
    std::cout << "  ---------------------------------\n";
    for(short iRank = 8; iRank > 0; --iRank) {
        std::cout << iRank << " | ";
        for(short iFile = 1; iFile < 9; ++iFile) {
            std::pair<Color, Piece> result = board->GetIsOccupied(RANKS[iRank-1] & FILES[iFile-1]);
            if(result.second != Piece::Null) {
                char pieceChar = GetPieceChar(result.second);
                if(result.first == Color::White) {
                    std::cout << pieceChar;
                } else {
                    std::cout << (char)std::tolower(pieceChar);
                }
            } else {
                std::cout << " ";
            }
            std::cout << " | ";
        }
        std::cout << "\n";
    }
    std::cout << "  ---------------------------------\n";
    std::cout << "    A   B   C   D   E   F   G   H  \n";
}

U64 MakeBitBoard(int rank, int file) {
    if(rank > 8 || rank < 1 || file > 8 || file < 1) {
        std::cout << "Warning: invalid rank or file specified. Rank and file should be in the range [1,8].\n";
        return 0;
    }
    return RANKS[rank - 1] & FILES[file - 1];
}

namespace pybind11::detail {
    template <typename T>
    struct type_caster<std::unique_ptr<T>> : public type_caster_base<std::unique_ptr<T>> {
    public:
        PYBIND11_TYPE_CASTER(std::unique_ptr<T>, _("std::unique_ptr<") + type_caster<T>::name + _(">"));

        bool load(handle src, bool convert) {
            if (!src)
                return false;
            // Use the existing type caster for T to convert src to a raw pointer
            value.reset(src.cast<T*>());
            return static_cast<bool>(value);
        }

        static handle cast(const std::unique_ptr<T> &src, return_value_policy policy, handle parent) {
            if (!src)
                return none().release();
            // Use the existing type caster for T to convert src to a Python object
            return type_caster<T>::cast(*src, policy, parent);
        }
    };
} // namespace pybind11::detail

PYBIND11_MODULE(chess_engine, m) {
    m.def("print_board", &PrintBoard, "Display a console-like version of the current chess board");
    m.def("make_bit_board", &MakeBitBoard, py::arg("rank"), py::arg("file"));
    m.def("print_bitset", &PrintBitset, py::arg("bitboard"));

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

    py::class_<Board, std::shared_ptr<Board>>(m, "Board")
        .def(py::init<>())
        .def("reset", &Board::Reset)
        .def("get_fen", &Board::GetFEN)
        .def("get_hash", &Board::GetHash)
        .def("get_state", &Board::GetState)
        .def("get_color_to_move", &Board::GetColorToMove)
        .def("load_fen", &Board::LoadFEN)
        .def("get_n_moves", &Board::GetNMoves)
        .def("reset", &Board::Reset)
        .def("print_fen", &Board::PrintFEN)
        .def("make_move", &Board::MakeMove)
        .def("undo_move", &Board::UndoMove)
        .def("get_board_color_piece", py::overload_cast<const Color, const Piece>(&Board::GetBoard), "Get the bitboard for a particular piece type and colour.", py::arg("color"), py::arg("piece"));
    
    py::class_<Generator>(m, "Generator")
        .def(py::init<>())
        .def("generate_legal_moves", &Generator::GenerateLegalMoves, "Generates the set of legal moves for the current board", py::arg("board"))
        .def("get_legal_moves", &Generator::GetLegalMoves)
        .def("get_n_legal_moves", &Generator::GetNLegalMoves)
        .def("is_under_attack", &Generator::IsUnderAttack);
}
