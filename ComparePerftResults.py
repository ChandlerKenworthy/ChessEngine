import sys

def compare_outputs(engine_output, stockfish_output):

    nEngineNodes = engine_output.split('\n')#[-1]
    nEngineNodes = [i for i in nEngineNodes if len(i) > 0]
    nEngineNodes = int(nEngineNodes[-1].split(":")[1])

    nStockfishNodes = stockfish_output.split('\n')#[-1]
    nStockfishNodes = [i for i in nStockfishNodes if len(i) > 0]
    nStockfishNodes = int(nStockfishNodes[-1].split(":")[1])
    
    return nStockfishNodes == nEngineNodes

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python compare_outputs.py <engine_output_file> <stockfish_output_file>")
        sys.exit(1)

    engine_output_file = sys.argv[1]
    stockfish_output_file = sys.argv[2]

    with open(engine_output_file, 'r') as f:
        engine_output = f.read()

    with open(stockfish_output_file, 'r') as f:
        stockfish_output = f.read()

    if compare_outputs(engine_output, stockfish_output):
        print("Outputs match!")
        sys.exit(0)
    else:
        print("Outputs do not match!")
        sys.exit(1)


"""
import numpy as np
import pandas as pd

if __name__ == "__main__":
    stockfish_perft_result_path = "perft/stockfish_perft_7.txt"
    engine_perft_result_path = "perft/engine_perft_7.txt"

    stockfish_perft = pd.read_csv(stockfish_perft_result_path, sep=":", header=None)
    engine_perft = pd.read_csv(engine_perft_result_path, sep=":", header=None)
    stockfish_perft.columns = ["Move", "Nodes"]
    engine_perft.columns = ["Move", "Nodes"]
    stockfish_perft.set_index("Move", drop=True, inplace=True)
    engine_perft.set_index("Move", drop=True, inplace=True)

    stockfish_perft.sort_index(inplace=True)
    engine_perft.sort_index(inplace=True)

    x = stockfish_perft.index.to_list()
    res = [i for i in x if i not in engine_perft.index.to_list()]
    print(res)

    if len(res) == 0:
        stockfish_perft["NodeEngine"] = engine_perft["Nodes"].to_numpy().astype(np.int64)
        stockfish_perft["Delta"] = stockfish_perft["Nodes"] - stockfish_perft["NodeEngine"]

        print(stockfish_perft)
"""
