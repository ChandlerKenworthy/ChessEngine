import chess.engine
import chess.pgn
import pandas as pd

n_games_to_process = 1
fens = []
evals = []

pgn = open("../games/master/master_games-2.pgn")
for i in range(n_games_to_process):
    game = chess.pgn.read_game(pgn)
    moves = game.mainline_moves()
    moves_list = [move for move in moves]
    moves_str = ' '.join(map(str, moves_list))

    board = game.board()

    for move in moves:
        board.push(move)
        fen = board.fen()
        fens.append(fen)

# Initialize Stockfish engine
engine = chess.engine.SimpleEngine.popen_uci("/opt/homebrew/bin/stockfish")
print(f"Number of FENs: {len(fens)}")

for fen in fens:
    board = chess.Board(fen.strip())
    analysis = engine.analyse(board, chess.engine.Limit(time=0.1))
    evals.append(analysis["score"].white().score(mate_score=10000))

# Close the engine
engine.quit()

df = pd.DataFrame({"position": fens, "score": evals})
df.to_csv("../games/master/master_games-2_1.txt")