import chess.pgn
import random
import math
import os

games = []

for file in os.listdir("/mnt/b/Documents/Coding/ChessMasterGames"):
    print("Working on file", file)
    pgn = open(f"/mnt/b/Documents/Coding/ChessMasterGames/{file}")
    positions = []
    fen = ""

    for i in range(20): # 20 is max
        game = chess.pgn.read_game(pgn)
        moves = game.mainline_moves()
        moves_list = [move for move in moves]
        moves_str = ' '.join(map(str, moves_list))
        games.append(moves_str)
        #board = game.board()

        #plyToPlay = math.floor(16 + 20 * random.random()) & ~1
        #numPlyPlayed = 0
        #for move in moves:
        #    board.push(move)
        #    numPlyPlayed += 1
        #    if numPlyPlayed == plyToPlay:
        #        fen = board.fen()
        
        #numPiecesInPos = sum(fen.lower().count(char) for char in "rnbq")
        #if numPlyPlayed > plyToPlay + 20 * 2 and numPiecesInPos >= 10:
        #    positions.append(fen)

with open("output_master.txt", "w") as file:
    for string in games:
        file.write(string + "\n")