#include <iostream>
#include <cstdint>

#include <graph.hpp>
#include <clq_weighted_parser.hpp>
#include <mtx_parser.hpp>
#include <unordered_set>
#include <set>
#include <map>
#include <queue>
#include <bitset.hpp>

//#define MTX

bool degree0(Graph2D &graph, DynamicBitSet &activeVertices) {
    // Nodes with a degree of 0 are always nodes separated from the other nodes
    // Removing such nodes will not have an influence on other nodes, thus looping
    // over those nodes is sufficient.

    bool edited = false;
    for(uint32_t vertex : DynamicBitSet {activeVertices}) {
        if(graph.getDegree(vertex) == 0) {
            // We only edit the activeNodes array since there is no edges to another vertix anyway
            // Calling graph.removeVertix() won't do anything as a result
            activeVertices.erase(vertex);
            edited = true;
        }
    }

    return edited;
}

bool adjacent(Graph2D &graph, DynamicBitSet &activeVertices, DynamicBitSet &partialVertexCover) {
    bool edited = false;
    for(uint32_t vertex : DynamicBitSet {activeVertices}) {
        uint32_t weight = 0;
        DynamicBitSet neighbours = graph.neighboursSet(vertex);
        for(uint32_t neighbour : neighbours) {
            weight += graph.getWeight(neighbour);
        }

        if (graph.getWeight(vertex) >= weight) {
            for(uint32_t neighbour : neighbours) {
                activeVertices.erase(neighbour);
                graph.removeVertex(neighbour);

                // Move the vertices of N(v) from V to S
                partialVertexCover.insert(neighbour);
            }
            activeVertices.erase(vertex);
            graph.removeVertex(vertex);
            edited = true;
        }
    }

    return edited;
}

bool degree1(Graph2D &graph, DynamicBitSet &activeVertices, DynamicBitSet &partialVertexCover) {
    bool edited = false;
    for(uint32_t vertex : DynamicBitSet {activeVertices}) {
        // Get the weight of degree-1 neighbours
        uint32_t weight = 0;
        std::unordered_set<uint32_t> degree1Neighbours;
        for(uint32_t neighbour : graph.neighboursSet(vertex)) {
            if(graph.getDegree(neighbour) == 1) {
                weight += graph.getWeight(neighbour);
                degree1Neighbours.insert(neighbour);
            }
        }

        if(graph.getWeight(vertex) <= weight) {
            for(uint32_t neighbour : degree1Neighbours) {
                activeVertices.erase(neighbour);
                graph.removeVertex(neighbour);
            }
            activeVertices.erase(vertex);
            graph.removeVertex(vertex);

            // Move v from V to S
            partialVertexCover.insert(vertex);

            edited = true;
        }
    }

    return edited;
}

bool degree2(Graph2D &graph, DynamicBitSet &activeVertices, DynamicBitSet &partialVertexCover) {
    bool edited = false;
    for(uint32_t vertex1 : DynamicBitSet {activeVertices}) {        
        for(uint32_t vertex2 : DynamicBitSet {activeVertices}) {
            if(vertex1 == vertex2) {
                continue;
            }

            // Get intersection between the two neighbour sets
            // And calculate the eventua weights
            // TODO: Now 0 is an illegal weight. 0 is smaller than all weights.
            uint32_t weight = 0;
            DynamicBitSet degree2Neighbours {graph.numberOfVertices};
            DynamicBitSet neighbours1 = graph.neighboursSet(vertex1);
            for(uint32_t neighbour2 : graph.neighboursSet(vertex2)) {
                // Adjacent to both vertices and degree is 2
                if(neighbours1.contains(neighbour2) && graph.getDegree(neighbour2) == 2) {
                    weight += graph.getWeight(neighbour2);
                    degree2Neighbours.insert(neighbour2);
                }
            }

            if(graph.getWeight(vertex1) + graph.getWeight(vertex2) <= weight) {
                for(uint32_t neighbour : degree2Neighbours) {
                    activeVertices.erase(neighbour);
                    graph.removeVertex(neighbour);
                }
                activeVertices.erase(vertex1);
                activeVertices.erase(vertex2);
                graph.removeVertex(vertex1);
                graph.removeVertex(vertex2);

                // Move vi and vj from V to S
                partialVertexCover.insert(vertex1);
                partialVertexCover.insert(vertex2);

                edited = true;
            }
        }
    }

    return edited;
}

void reduce(Graph2D &graph, DynamicBitSet &activeVertices, DynamicBitSet &partialVertexCover) {
    uint32_t size;
    do {
        size = activeVertices.size();

        while(degree0(graph, activeVertices)) {};

        while(adjacent(graph, activeVertices, partialVertexCover)) {};

        while(degree1(graph, activeVertices, partialVertexCover)) {};

        while(degree2(graph, activeVertices, partialVertexCover)) {};
    } while(size != activeVertices.size());
}

uint32_t greatestDegreeHeuristic(Graph2D &graph, DynamicBitSet &activeVertices) {
    uint32_t v = *activeVertices.begin();
    for(uint32_t vertex : activeVertices) {
        if(graph.getDegree(vertex) > graph.getDegree(v)) {
            v = vertex;
        }
    }
    return v;
}

uint32_t randomHeuristic(DynamicBitSet& activeVertices) {
    uint32_t n = rand() % activeVertices.size();
    uint32_t i = 0;
    for(uint32_t vertex : activeVertices) {
        if(i == n) {
            return vertex;
        }
        i++;
    }
    return *activeVertices.begin();
}

uint32_t smallestWeightHeuristic(Graph2D &graph, DynamicBitSet &activeVertices) {
    uint32_t v = *activeVertices.begin();
    for(uint32_t vertex : activeVertices) {
        if(graph.getWeight(vertex) < graph.getWeight(v)) {
            v = vertex;
        }
    }
    return v;
}

uint32_t greatestDegreeWeightRatioHeuristic(Graph2D &graph, DynamicBitSet &activeVertices) {
    uint32_t v = *activeVertices.begin();
    for(uint32_t vertex : activeVertices) {
        if(graph.getDegree(vertex) / graph.getWeight(vertex) > graph.getDegree(v) / graph.getWeight(v)) {
            v = vertex;
        }
    }
    return v;
}

uint32_t lowerBound(Graph2D &graph, DynamicBitSet activeVertices, std::map<uint32_t, DynamicBitSet> &neighbours) {
    uint32_t result = 0;
    while(!activeVertices.empty()) {
        // Select a node from the active nodes
        uint32_t start = *activeVertices.begin();
        activeVertices.erase(start);

        // This set initially contains all neighbours of the start vertex, because they are all potention clique candidates
        // Through iterations this set will schrink so it contains the neighbours of all added vertices
        DynamicBitSet tmpNeighbours {graph.numberOfVertices};
        for(uint32_t n : neighbours[start]) {
            if(activeVertices.contains(n)) {
                tmpNeighbours.insert(n);
            }
        }

        uint32_t cliqueWeight = graph.getWeight(start);
        uint32_t max = graph.getWeight(start);

        // While there still is a vertex to add to the clique
        while(!tmpNeighbours.empty()) {
            uint32_t selected = *tmpNeighbours.begin();
            activeVertices.erase(selected);

            cliqueWeight += graph.getWeight(selected);
            if(graph.getWeight(selected) > max) {
                max = graph.getWeight(selected);
            }

            // Take the intersection of the current neighbours and the neighbours of the selected vertex
            DynamicBitSet tmp = tmpNeighbours;
            tmpNeighbours.clear();
            for(uint32_t n : neighbours[selected]) {
                if(tmp.contains(n)) {
                    tmpNeighbours.insert(n);
                }
            }
        }

        result += (cliqueWeight - max);
    }
    return result;
}

DynamicBitSet search(Graph2D &graph, DynamicBitSet &activeVertices, std::map<uint32_t, DynamicBitSet> &neighbours, DynamicBitSet &vertexCover, uint32_t vertexCoverWeight, DynamicBitSet &vertexCoverBest) {
    uint32_t vertexCoverBestWeight = 0;
    for(uint32_t vector : vertexCoverBest) {
        vertexCoverBestWeight += graph.getWeight(vector);
    }
    
    if(activeVertices.empty()) {
        if (vertexCoverBestWeight > vertexCoverWeight) {
            return vertexCover;
        } else {
            return vertexCoverBest;
        }
    }

    if(lowerBound(graph, activeVertices, neighbours) + vertexCoverWeight >= vertexCoverBestWeight) {
        return vertexCoverBest;
    }

    // Select a vertex v using a heuristic (H1 for now)
    // Doing it this way is stupid, but at least it works
    uint32_t v = greatestDegreeHeuristic(graph, activeVertices);

    activeVertices.erase(v);
    vertexCover.insert(v);
    vertexCoverBest = search(graph, activeVertices, neighbours, vertexCover, vertexCoverWeight + graph.getWeight(v), vertexCoverBest);
    vertexCover.erase(v);

    DynamicBitSet tmpNeighbours {graph.numberOfVertices};
    for(uint32_t neighbour : neighbours[v]) {
        if(activeVertices.contains(neighbour)) {
            tmpNeighbours.insert(neighbour);
            activeVertices.erase(neighbour);
            vertexCover.insert(neighbour);
            vertexCoverWeight += graph.getWeight(neighbour);
        }
    }

    auto result = search(graph, activeVertices, neighbours, vertexCover, vertexCoverWeight, vertexCoverBest);

    activeVertices.insert(v);
    for(uint32_t neighbour : tmpNeighbours) {
        activeVertices.insert(neighbour);
        vertexCover.erase(neighbour);
        vertexCoverWeight -= graph.getWeight(neighbour);
    }

    return result;
}

// branch and bound minimum weight vertex cover algo
DynamicBitSet bmwvc(Graph2D &graph) {
    // Current active vertices in the graph
    DynamicBitSet activeVertices {graph.numberOfVertices};
    for(uint32_t i = 0; i < graph.numberOfVertices; ++i) {
        activeVertices.insert(i);
    }

    // vertex cover that will be initiated by reduce(...)
    DynamicBitSet vertexCover {graph.numberOfVertices};

    reduce(graph, activeVertices, vertexCover);

    // construct all neighbour DynamicBitSets once
    std::map<uint32_t, DynamicBitSet> neighbours;
    for(uint32_t vertex : activeVertices) {
        neighbours[vertex] = std::move(graph.neighboursSet(vertex));
    }

    // Breadth first flood fill to get all disjoint subgraphs
    while(!activeVertices.empty()) {
        std::queue<uint32_t> queue;
        DynamicBitSet subGraph {graph.numberOfVertices}; // TODO: Maybe this bfs flood can also perform the sort using the heuristic
        queue.push(*activeVertices.begin());
        while(!queue.empty()) {
            uint32_t vertex = queue.front();
            subGraph.insert(vertex);

            for(uint32_t neighbour : neighbours[vertex]) {
                if(!subGraph.contains(neighbour)) {
                    queue.push(neighbour);
                }
            }

            activeVertices.erase(vertex);
            queue.pop();
        }
        
        DynamicBitSet tmpVertexCover {graph.numberOfVertices};
        DynamicBitSet tmpVertexCoverBest {subGraph};
        vertexCover.insertAll(search(graph, subGraph, neighbours, tmpVertexCover, 0, tmpVertexCoverBest));
    }

    return vertexCover;
}

int main(int argc, char* argv[]) {
    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " [filename]" << std::endl;
        return 1;
    }

    Graph2D graph;

#ifdef MTX  
    try {
        MtxParser parser;
        parser.parse(*argv[1], graph);
    } catch(std::exception& e) {
        std::cerr << "Parsing failure: " << e.what() << std::endl;
        return 2;
    }

    const uint32_t N = graph.numberOfVertices;

    for(uint32_t i = 1; i <= N; ++i) {
        graph.setWeight(i - GRAPH_CORRECTION, (i + 1) % 200);
    }
#else
    try {
        ClqWeightedParser parser;
        parser.parse(*argv[1], graph);
    } catch(std::exception& e) {
        std::cerr << "Parsing failure: " << e.what() << std::endl;
        return 2;
    }

    const uint32_t N = graph.numberOfVertices;
#endif

    std::cout << "Graph of " << N << " vertices, " << graph.numberOfEdges << " edges, density " << graph.density() << std::endl;
    DynamicBitSet result = bmwvc(graph);

    std::cout << "Minimum weighted vertex cover: [ ";
    uint32_t weight = 0;
    for(uint32_t vertex : result) {
        std::cout << vertex + 1 << " ";
        weight += graph.getWeight(vertex);
    }
    std::cout << "]";
    std::cout << ", total weight: " << weight << std::endl;
    
    return 0;
}
