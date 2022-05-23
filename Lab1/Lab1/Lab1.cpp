#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <cmath>

using namespace std;

struct Node {
    string name;
    double score;
    vector<Node*> neighbours;
};

double compute_edge_weight(Node* node1, Node* node2) {
    return abs(node1->score - node2->score) / (node1->score + node2->score);
}

int main(int argc, char* argv[])
{
    string source = argv[1];
    string dest = argv[2];
    int max_hops = stoi(argv[3]);
    cout << "source node = " << source << ", dest node = " << dest << ", hops = " << max_hops << endl;

    unordered_map<string, Node*> graph;

    //read graph file
    ifstream graph_file("graph.txt");
    string line;
    if (graph_file.is_open()) {
        cout << "success" << endl;
        while (getline(graph_file, line)) {
            stringstream ss(line);
            string node1, node2;
            ss >> node1;
            ss >> node2;
            //construct adjacency list
            if (graph.find(node1) == graph.end()) {
                Node* node = new Node();
                node->name = node1;
                graph[node1] = node;
            }
            if (graph.find(node2) == graph.end()) {
                Node* node = new Node();
                node->name = node2;
                graph[node2] = node;
            }
            graph[node1]->neighbours.push_back(graph[node2]);
            graph[node2]->neighbours.push_back(graph[node1]);
        }
        graph_file.close();
    }
    else {
        cout << "failed when opening graph.txt" << endl;
        return -1;
    }

    return 0;
}
