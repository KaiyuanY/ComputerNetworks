#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <cmath>
#include <iomanip>
#include <climits>

using namespace std;

struct Node {
    string name;
    double score;
    vector<Node*> neighbours;
};

double compute_edge_weight(Node* node1, Node* node2) {
    return abs(node1->score - node2->score) / (node1->score + node2->score);
}

void print_graph(unordered_map<string, Node*> graph) {
    cout << "Graph: | Node = Score: - (Adjacent Node 1) - (Adjacent Node 2) - ... |" << endl;
    for (auto n : graph) {
        string name = n.first;
        Node* node = n.second;
        cout << name << " = " << node->score << ":";
        for (Node* neighbour : node->neighbours) {
            cout << " - " << neighbour->name;
        }
        cout << endl;
    }
}

void dfs(Node* source, Node* dest, int max_hop, double distance, unordered_set<string> & visited, vector<double> & distance_list, vector<string> & cur_path, unordered_map<double, string> & path_list) {
    //base case
    if (max_hop < 0) {
        return;
    }
    if (source->name == dest->name) {
        distance_list.push_back(distance);
        string path = "";
        for (string node : cur_path) {
            path.append(node + " ");
        }
        path.pop_back();
        path_list[distance] = path;
        //cout << "adding [" << distance << ", " << path << "]" << endl;
        return;
    }
    //recursive steps
    vector<Node*> neighbours = source->neighbours;
    for (Node* neighbour : neighbours) {
        if (visited.find(neighbour->name) == visited.end()) {
            double gap = compute_edge_weight(source, neighbour);
            distance += gap;
            visited.insert(neighbour->name);
            cur_path.push_back(neighbour->name);
            dfs(neighbour, dest, max_hop - 1, distance, visited, distance_list, cur_path, path_list);
            cur_path.pop_back();
            visited.erase(neighbour->name);
            distance -= gap;
        }
    }
}

int main(int argc, char* argv[])
{
    string source = argv[1];
    string dest = argv[2];
    int max_hop = stoi(argv[3]);
    //cout << "source node = " << source << ", dest node = " << dest << ", hops = " << max_hop << endl;

    unordered_map<string, Node*> graph;

    //read graph file
    ifstream graph_file("graph.txt");
    string line;
    if (graph_file.is_open()) {
        while (getline(graph_file, line)) {
            //cout << line << ";" << endl;
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

    //read score file
    ifstream score_file("scores.txt");
    if (score_file.is_open()) {
        while (getline(score_file, line)) {
            //cout << line << ";" << endl;
            stringstream ss(line);
            string node;
            double score;
            ss >> node;
            ss >> score;
            //cout << "node = " << node << ", score = " << score << endl;
            if (graph[node] == NULL) {
                Node* n = new Node();
                graph[node] = n;
            }
            graph[node]->score = score;
        }
        score_file.close();
    }
    else{
        cout << "failed when opening scores.txt" << endl;
        return -2;
    }

    //print_graph(graph);
    //cout << endl << "Shortest Path Result:" << endl;
    //find shortest path
    if (graph[source] == NULL || graph[dest] == NULL) {
        cout << "No path exists between " << source << " and " << dest << " within " << max_hop << " hops." << endl;
        return 1;
    }
    if (source == dest) {
        cout << 0.00 << endl;
    }

    unordered_set<string> visited;
    vector<double> distance_list;
    vector<string> cur_path;
    unordered_map<double, string> path_list;

    visited.insert(source);
    cur_path.push_back(source);

    dfs(graph[source], graph[dest], max_hop, 0, visited, distance_list, cur_path, path_list);

    //for (auto path : path_list) {
        //cout << "[" << path.first << ", " << path.second << "]" << endl;
    //}
    //post processing
    if (distance_list.size() == 0) {
        cout << "No path exists between " << source << " and " << dest << " within " << max_hop << " hops." << endl;
        return 2;
    }
    
    double min = INT_MAX;
    for (double distance : distance_list) {
        if (distance < min) {
            min = distance;
        }
    }
    string path = path_list[min];

    cout << path << endl;
    cout << fixed << setprecision(2) << min << endl;

    return 0;
}
