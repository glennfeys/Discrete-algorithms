#pragma once

#include <cstdint>
#include <iostream>
#include <fstream>
#include <limits>

#ifdef GRAPH_STARTS_AT_ZERO
#define GRAPH_CORRECTION 1
#else
#define GRAPH_CORRECTION 0
#endif

/**
 * Just a dummy graph used to test the parser.
 */
class DummyGraph final {
public:
    DummyGraph() {}
    ~DummyGraph() {}

    void setParameters(uint32_t numberOfVertices, uint32_t numberOfEdges) {
        std::cout << numberOfVertices << " vertices, " << numberOfEdges << " edges" << std::endl;
    }

    void addEdge(uint32_t from, uint32_t to) {
        std::cout << from << ' ' << to << std::endl;
    }
};

class ClqParser final {
public:
    ClqParser() {}
    ClqParser(const ClqParser&) = delete;
    ~ClqParser() {}

    template<typename G>
    void parse(const char& fileName, G& graph) const {
        std::ifstream inputFile(&fileName);
        if(!inputFile) {
            throw std::runtime_error("Could not open source file");
        }

        std::string part;
        while(inputFile >> part) {
            switch(part[0]) {
                // Comment
                case 'c':
                    inputFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    continue;
                
                // Parameters
                case 'p': {
                    uint32_t numberOfVertices, numberOfEdges;
                    inputFile >> part;
                    inputFile >> numberOfVertices;
                    inputFile >> numberOfEdges;
                    graph.setParameters(numberOfVertices, numberOfEdges);
                    break;
                }

                // Edges
                case 'e': {
                    uint32_t from, to;
                    inputFile >> from;
                    inputFile >> to;
                    graph.addEdge(from - GRAPH_CORRECTION, to - GRAPH_CORRECTION);
                    break;
                }

                default: {
                    throw std::runtime_error(std::string("Unexpected token ") + part[0]);
                }
            }
        }
    }
};
