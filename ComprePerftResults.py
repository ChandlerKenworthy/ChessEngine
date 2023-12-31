import pandas as pd
import numpy as np

if __name__ == "__main__":
    stockfish_perft_result_path = "stockfish_perft_8.txt"
    engine_perft_result_path = "engine_perft_8.txt"

    stockfish_perft = pd.read_csv(stockfish_perft_result_path, sep=":", header=None)
    engine_perft = pd.read_csv(engine_perft_result_path, sep=":", header=None)
    stockfish_perft.columns = ["Move", "Nodes"]
    engine_perft.columns = ["Move", "Nodes"]
    stockfish_perft.set_index("Move", drop=True, inplace=True)
    engine_perft.set_index("Move", drop=True, inplace=True)

    stockfish_perft.sort_index(inplace=True)
    engine_perft.sort_index(inplace=True)

    stockfish_perft["NodeEngine"] = engine_perft["Nodes"].to_numpy().astype(np.int64)
    stockfish_perft["Delta"] = stockfish_perft["Nodes"] - stockfish_perft["NodeEngine"]

    print(stockfish_perft)