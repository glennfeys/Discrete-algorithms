#pragma once

#include <cstdint>
#include <iostream>
#include <fstream>
#include <limits>

class MtxParser final {
public:
    MtxParser() {}
    MtxParser(const MtxParser&) = delete;
    ~MtxParser() {}

    template<typename G>
    void parse(const char& fileName, G& graph) const {
        std::ifstream inputFile(&fileName);
        if(!inputFile) {
            throw std::runtime_error("Could not open source file");
        }

        std::string part;
        bool firstLine = true;
        while(inputFile >> part) {
            switch(part[0]) {
                // Comment
                case '%':
                    inputFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    continue;

                // Integer
                case '0' ... '9': {
                    if(firstLine) {
                        uint32_t numberOfVertices, numberOfEdges;
                        inputFile >> numberOfVertices;
                        inputFile >> numberOfEdges;
                        graph.setParameters(numberOfVertices, numberOfEdges);

                        firstLine = false;
                        break;
                    }

                    uint32_t to;
                    inputFile >> to;
                    graph.addEdge(stoi(part) - GRAPH_CORRECTION, to - GRAPH_CORRECTION);
                    break;
                }

                default: {
                    throw std::runtime_error(std::string("Unexpected token ") + part[0]);
                }
            }
        }
    }
};
