# How to compile and run
* make
* ./lab1 AB AT 5

# Implementation
* Parse graph.txt and scores.txt and construct graph.
* Run DFS on graph to find a list of all paths from the 2 requested nodes with number of hops less or equal to k.
* Find the path with smallest distance | scoreA - scoreB |/(scoreA + scoreB).
* Print the path and its corresponding distance.