#include <iostream>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <numeric>

#include <matrix.hpp>
#include <graph.hpp>
#include <clq_parser.hpp>
#include <math.h>
#include <set>
#include <random>
#include <map>


typedef uint32_t Vertex;
typedef std::set<Vertex> Clique;

Clique biggestClique;
Clique currBiggestClique;
int* resultClique;

const int KMax = 15;

std::map<Clique, std::set<Clique>> neighborhoods;
size_t iterationsSinceImprovement = 0;
std::set<Vertex> possibleNodes;

// Get degree of the vertex in the graph
int getDegree(ListGraph* graph, Vertex v) {
    int result = 0;
    for(uint32_t i=0; i<graph->numberOfVertices; i++){
        if(i != v) {
            if (graph->hasEdge(i+1, v+1)) result++;
        }
    }
    return result;
}

// gives all nodes from the graph that can be added to the clique
Clique getPossibleAdds(ListGraph* graph, Clique clique) {
    // make possible nodes empty
    possibleNodes.clear();
    // Add all nodes to possible nodes
    for (uint32_t i=0; i<graph->numberOfVertices; i++) {
        possibleNodes.insert(i);
    }
    Clique result;
    // check for all nodes if they are adjacent to all nodes from the clique, if this is the case add them to result
    for (Vertex v1 : possibleNodes) {
        // if vertex in clique continue
        if(clique.find(v1) != clique.end()) continue;
        bool canAdd = true;
        // check if v is adjacent to all Vertices of clique
        for (Vertex v2 : clique) {
            // if a vertex is not adjacent with v1 break
            if (!graph->hasEdge(v1+1, v2+1)){
                canAdd = false;
                break;
            }
        }
        // if all Vertices of the clique were adjacent with v1 add it to result
        if (canAdd) result.insert(v1);
    }
    return result;
}

// Do local search to get maximum clique that can be made from clique
// we do this by adding the best vertices one by one
Clique vnd(ListGraph* graph, Clique clique) {
    Clique currBest = clique;
    // get all vertices that can be added to the clique
    Clique poss = getPossibleAdds(graph, clique);
    // while we find nodes we can add to the clique, find the best one and add it to the clique
    while (poss.size() > 0) {
        int bestDegree = -1;
        Vertex bestVertex;
        // 80% Chance that we take the highest degree as best vertex 20% chance we take the lowest degree
        bool isBest = (rand() % 100) > 20;
        // get The degree from all poss and pick the highest/lowest as best vertex based ion isBest
        for(Vertex v : poss) {
            int currDegree = getDegree(graph, v);
            if (bestDegree == -1 || (isBest && currDegree >= bestDegree) || (!isBest && currDegree <= bestDegree)) {
                bestDegree = currDegree;
                bestVertex = v;
            }
        }
        // add the best vertex to the clique
        clique.insert(bestVertex);
        // find vertices we can add to the new clique
        poss = getPossibleAdds(graph, clique);
    }
    
    return clique;
}

// get the direct neighborhood of clique (all cliques on distance 1)
std::set<Clique> getNeighborhood(ListGraph* graph, Clique clique) {
    Clique cliqueCopy = clique;
    // get all vertices we can add to clique that will stay a clique
    Clique adds = getPossibleAdds(graph, clique);
    std::set<Clique> result;
    // add all possible neighbors by adding a vertex
    for(Vertex v: adds) {
        cliqueCopy.insert(v);
        result.insert(cliqueCopy);
        cliqueCopy.erase(v);
    }
    // add all possible neighbors by removing a vertex
    for (Vertex v : clique) {
        // if clique size is one we can not remove a vertex
        if (clique.size() == 1) break;
        cliqueCopy.erase(v);
        result.insert(cliqueCopy);
        cliqueCopy.insert(v);
    }
    return result;
}

// get clique n from s 
Clique getCliqueN(std::set<Clique> s, int n) {
  std::set<Clique>::iterator it = s.begin();
  // 'advance' the iterator n times
  std::advance(it,n);
  return *it;
}

// get a random clique from neighborhood k
Clique shake(int k, ListGraph* graph, Clique clique) {
    // Get the neighborhood 1 of the current clique and choose one randomly from that neighborhood
    // Do this k times so we will have a clique on distance k from the original clique 
    for (int i=0; i<k; i++){
        // if the neighborhood of the clique was not yet calculated before
        if(neighborhoods.find(clique) == neighborhoods.end()) {
            // calculate the neighborhood of the current clique
            neighborhoods[clique] = getNeighborhood(graph, clique);
        }
        std::set<Clique> currNeighborhood = neighborhoods[clique];
        // get a random index of currNeighborhood
        int randIndex = (rand() % static_cast<int>(currNeighborhood.size()));
        // get a random clique from currNeighborhood
        clique = getCliqueN(currNeighborhood, randIndex);
    }
    return clique;
}

void vnsStep(ListGraph* graph) {
    int k = 0;
    // Try to search new optimum in neighborhood k
    // neighborhood k has all cliques with distance k of currBiggestClique 
    // (e.g. for k=2: 2 additions, or 2 deletions or 1 addition and 1 deltion)
    while (k < KMax) {
        // choose random clique from neighborhood k
        Clique nClique = shake(k, graph, currBiggestClique);
        // perform local search on nClique here with variable neighbourhood decent
        Clique localBest = vnd(graph, nClique);
        // check if localBest is bigger than currBiggestClique
        if (localBest.size() > currBiggestClique.size()) {
            currBiggestClique = localBest;
            // start searching again from neighborhood 0 with new localBest
            k = 0;
        } else {
            k++;
        }
    }
    return;
}

// Perform variable neighborhood search on the graph to get the biggest clique
// return the size of the biggest clique
int vns(ListGraph* graph) {
    // get random seed for randomness
    srand((unsigned) time(0));
    // max iterations of vns we will do before we stop
    size_t maxItNoImprov = 50;

    //set initial solution
    uint32_t randomStart = (rand() % static_cast<int>(graph->numberOfVertices));
    biggestClique = {randomStart};
    currBiggestClique = {randomStart};

    // while we have not had maxItNoImprov iterations without improvements do a vns step
    while (iterationsSinceImprovement < maxItNoImprov) {
        vnsStep(graph);
        // If the biggestClique found is the biggest yet we store it in biggestClique
        if (currBiggestClique.size() > biggestClique.size()) {
            iterationsSinceImprovement = 0;
            biggestClique = currBiggestClique;
        }
        iterationsSinceImprovement++;
        std::cout << "it no improv: " << iterationsSinceImprovement << " biggest clique:" << biggestClique.size() << std::endl;
    }
    // print the biggest clique for debugging purposses only
    std::cout << "biggest clique: ";
    for (Vertex v : biggestClique) {
        std::cout << v << ", ";
    }
    std::cout << std::endl;
    return biggestClique.size();
}

int main(int argc, char* argv[]) {
    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " [filename]" << std::endl;
        return 1;
    }

    ListGraph graph;
    try {
        ClqParser parser;
        parser.parse(*argv[1], graph);
    } catch(std::exception& e) {
        std::cerr << "Parsing failure: " << e.what() << std::endl;
        return 2;
    }

    const uint32_t N = graph.numberOfVertices;

    std::cout << "Grah of " << N << " vertices, " << graph.numberOfEdges << " edges, density " << graph.density() << std::endl;
    int cliqueSize = vns(&graph);
    std::cout << "Size of max clique: " << cliqueSize << std::endl;

    return 0;
}
