import pandas as pd
import numpy as np

stockfish = pd.read_csv("../perft/fish3.txt", header=None, sep=":")
stockfish.columns = ["InitialMove", "Nodes"]
stockfish.set_index("InitialMove", inplace=True)

generator = pd.read_csv("../perft/gen3.txt", header=None, sep=":")
generator.columns = ["InitialMove", "Nodes"]
generator.set_index("InitialMove", inplace=True)

isDiff = False

# Search for differences in the index
missing_from_gen = [i for i in stockfish.index if i not in generator.index]
excess_from_gen = [i for i in generator.index if i not in stockfish.index]

if len(missing_from_gen) > 0:
    print(f"Generator failed to find {len(missing_from_gen)} initial nodes: ")
    for nodeName in missing_from_gen:
        isDiff = True
        print(f"Missing a node: {nodeName}")
    
if len(excess_from_gen) > 0:
    print(f"Generator found {len(missing_from_gen)} excess initial nodes: ")
    for nodeName in excess_from_gen:
        isDiff = True
        print(f"Excess node: {nodeName}")

if len(missing_from_gen) > 0 or len(excess_from_gen) > 0:
    exit()

# Search for differences in number of nodes after each initial move
stockfish.sort_values('InitialMove', inplace=True)
generator.sort_values('InitialMove', inplace=True)
stockfish["NGen"] = generator["Nodes"]
stockfish["Diff"] = stockfish["Nodes"] - stockfish["NGen"]

for i, diff in enumerate(stockfish["Diff"].to_list()):
    if diff != 0:
        isDiff = True
        if diff > 0:
            print(f"Node {stockfish.index.to_list()[i]} has {abs(diff)} too few moves")
        if diff < 0:
            print(f"Node {stockfish.index.to_list()[i]} has {abs(diff)} too many moves")

if not isDiff:
    print("Results match")