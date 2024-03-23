import chess.pgn
import random
import math

pgn = open("/mnt/b/Documents/Coding/lichess_db_standard_rated_2016-02.pgn")
positions = []
fen = ""
print("Parsing positions:")
N = 100000

for i in range(N):
    progress = int(math.floor(i/N * 100))
    completed_blocks = progress // 2
    remaining_blocks = 50 - completed_blocks
    progress_bar = "[" + "#" * completed_blocks + "-" * remaining_blocks + "]"
    print(f"\r{progress_bar} {progress}%", end='', flush=True)
    game = chess.pgn.read_game(pgn)
    moves = game.mainline_moves()
    board = game.board()

    plyToPlay = math.floor(16 + 20 * random.random()) & ~1
    numPlyPlayed = 0
    for move in moves:
        board.push(move)
        numPlyPlayed += 1
        if numPlyPlayed == plyToPlay:
            fen = board.fen()
    
    numPiecesInPos = sum(fen.lower().count(char) for char in "rnbq")
    if numPlyPlayed > plyToPlay + 20 * 2 and numPiecesInPos >= 10:
        positions.append(game.headers['Opening'])
        positions.append(fen)

with open("output.txt", "w") as file:
    for string in positions:
        file.write(string + "\n")