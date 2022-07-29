#include <iostream>
#include <algorithm>

#include <graph.hpp>
#include <clq_parser.hpp>
#include <offset_array.hpp>
#include <bitset.hpp>

void clique(const Graph2D &graph, OffsetArray<uint64_t, 1> &activeNodes, const uint64_t* neighbours, uint32_t size, DynamicBitSet &maxClique, DynamicBitSet &tempClique, uint32_t &maxCliqueSize, bool &found, OffsetArray<uint32_t, 1> &c) {
    uint32_t amountOfBitVectors = (graph.numberOfVertices + 64 - 1) / 64;
    
    // calculate intersection between activeNodes and neighbours
    OffsetArray<uint64_t, 1> nextActiveNodes { amountOfBitVectors };
    for(uint32_t j = 1; j <= amountOfBitVectors; ++j) {
        nextActiveNodes[j] = activeNodes[j] & neighbours[j - 1];
    }
    
    // Calculate size of current graph
    uint32_t graphSize = 0;
    for(uint32_t j = 1; j <= amountOfBitVectors; ++j) {
        graphSize += std::__popcount(nextActiveNodes[j]);
    }
    
    if(graphSize == 0) {
        if(size > maxCliqueSize) {
            maxCliqueSize = size;
            maxClique = tempClique; // Copies
            found = true;
        }
        return;
    }

    // While the graph is not empty
    while(graphSize != 0) {
        if(size + graphSize <= maxCliqueSize) 
            return;

        // Find the position of the lowest node (in our case the last set bit)
        uint32_t i = 1;
        for(uint32_t j = amountOfBitVectors; j > 0; --j) {
            if(nextActiveNodes[j] != 0) {
                i = ((j - 1) * 64) + 64 - std::__countl_zero(nextActiveNodes[j]);
                break;
            }
        }

        if(size + c[i] <= maxCliqueSize)
            return;

        tempClique.insert(i);

        // Remove node
        nextActiveNodes[((i - 1) / 64) + 1] &= ~(1UL << ((i - 1) & 63UL));

        auto newNeighbours = graph.neighbours(i);
        clique(graph, nextActiveNodes, newNeighbours, size + 1, maxClique, tempClique, maxCliqueSize, found, c);

        if(found)
            return;

        tempClique.erase(i);

        graphSize--;
    }
}

int main(int argc, char* argv[]) {
    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " [filename]" << std::endl;
        return 1;
    }

    Graph2D graph;
    try {
        ClqParser parser;
        parser.parse(*argv[1], graph);
    } catch(std::exception& e) {
        std::cerr << "Parsing failure: " << e.what() << std::endl;
        return 2;
    }

    std::cout << "Graph of " << graph.numberOfVertices << " vertices, " << graph.numberOfEdges << " edges, density " << graph.density() << std::endl;

    // Sort the graph
    OffsetArray<uint32_t, 1> actNode = graph.sortByEdgeDensity(0.40f, std::max_element);

    uint32_t amountOfBitVectors = (graph.numberOfVertices + 64 - 1) / 64;
    uint32_t maxCliqueSize = 0;
    DynamicBitSet maxClique { graph.numberOfVertices + 1 };
    bool found = false;
    OffsetArray<uint32_t, 1> c { graph.numberOfVertices };
    for(uint32_t i = 1; i <= graph.numberOfVertices; ++i)
        c[i] = graph.numberOfVertices;

    // A bitvector of used vertices in an iteration
    OffsetArray<uint64_t, 1> activeNodes { amountOfBitVectors };

    for(uint32_t i = 1; i <= graph.numberOfVertices; ++i) {
        found = false;

        // Array to keep track of nodes along the way
        DynamicBitSet tempClique { graph.numberOfVertices + 1 };
        tempClique.insert(i);

        auto neighbours = graph.neighbours(i);

        // Initialize active nodes array
        for(uint32_t j = 1; j <= amountOfBitVectors; ++j) {
            // All nodes in a vector are smaller than i
            if(i > j * 64) {
                activeNodes[j] = UINT64_MAX;
            }

            // i is somewhere in this vector
            else if((j - 1) * 64 < i && i <= j * 64) {
                activeNodes[j] = UINT64_MAX >> (i & 63UL);
            }

            // All zeroes, we already handled i
            else {
                activeNodes[j] = 0UL;
            }
        }

        clique(graph, activeNodes, neighbours, 1, maxClique, tempClique, maxCliqueSize, found, c);
        c[i] = maxCliqueSize;
    }

    std::cout << "Max clique has size " << maxCliqueSize << std::endl;
    std::cout << "Max clique: { ";
    for(uint32_t i : maxClique) {
        std::cout << actNode[i] << " ";
    }
    std::cout << "}" << std::endl;

    return 0;
}