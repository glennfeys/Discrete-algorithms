#include <iostream>
#include <algorithm>
#include <numeric>
#include <bit>
#include <cstdint>
#include <cstring>
#include <utility>

#include <graph.hpp>
#include <offset_array.hpp>
#include <bitset.hpp>

void Graph::addEdge(uint32_t from, uint32_t to) {
    auto offset = ref(from - GRAPH_OFFSET, to - GRAPH_OFFSET);
    offset.ref |= 1UL << offset.bit;
}

void Graph::removeEdge(uint32_t from, uint32_t to) {
    auto offset = ref(from - GRAPH_OFFSET, to - GRAPH_OFFSET);
    offset.ref &= ~(1UL << offset.bit);
}

bool Graph::hasEdge(uint32_t from, uint32_t to) const {
    auto offset = ref(from - GRAPH_OFFSET, to - GRAPH_OFFSET);
    return !!(offset.ref & (1UL << offset.bit));
}

void Graph::swapEdge(uint32_t from1, uint32_t to1, uint32_t from2, uint32_t to2) {
    bool hasEdge1 = hasEdge(from1, to1);
    bool hasEdge2 = hasEdge(from2, to2);

    if(hasEdge1) {
        addEdge(from2, to2);
    } else {
        removeEdge(from2, to2);
    }

    if(hasEdge2) {
        addEdge(from1, to1);
    } else {
        removeEdge(from1, to1);
    }
}

void Graph::swapVertex(uint32_t a, uint32_t b) {
    auto N = numberOfVertices;
    for(uint32_t i = 1; i <= N; ++i) {
        // Since we're swapping two indices, the source must be corrected here.
        uint32_t actualIndex = i;
        if(actualIndex == a) actualIndex = b;
        else if(actualIndex == b) actualIndex = a;

        swapEdge(a, i, b, actualIndex);
    }
}

float Graph::density() const {
    float V = (float) numberOfVertices;
    return 2.0f * ((float) numberOfEdges) / (V * (V - 1.0f));
}

void Graph::sortByEdgeDensity(uint32_t* actNode, uint32_t* (*finder)(uint32_t*, uint32_t*)) {
    const uint32_t N = numberOfVertices;

    // edge[i] gives the amount of edges for node i.
    uint32_t* edge = new uint32_t[N];

    // Count amount of edges for each node.
    for(uint32_t row = 1; row <= N; ++row) {
        edge[row - GRAPH_OFFSET] = 0;
        for(uint32_t col = 1; col <= N; ++col) {
            if(hasEdge(row, col)) {
                ++edge[row - GRAPH_OFFSET];
            }
        }
    }

    for(uint32_t node = GRAPH_OFFSET; node <= N - 2 - (1 - GRAPH_OFFSET); ++node) {
        uint32_t minNode = finder(edge + node - GRAPH_OFFSET, edge + N) - edge;

        if(node - GRAPH_OFFSET != minNode) {
            std::swap(edge[minNode], edge[node - GRAPH_OFFSET]);
            std::swap(actNode[minNode], actNode[node - GRAPH_OFFSET]);
            swapVertex(node, minNode + GRAPH_OFFSET);
        }
    }

    //graph.printAdjacencyMatrix();

    delete[] edge;
}

OffsetArray<uint32_t, GRAPH_OFFSET> Graph::sortByEdgeDensity(float densityThreshold, uint32_t* (*finder)(uint32_t*, uint32_t*)) {
    uint32_t* actNode_ = new uint32_t[numberOfVertices];
    std::iota(actNode_, actNode_ + numberOfVertices, 1);

    // Order nodes by increasing edge density.
    if(density() >= densityThreshold) {
        sortByEdgeDensity(actNode_, finder);
    }

    return { actNode_ };
}

void Graph::printAdjacencyMatrix() const {
    for(uint32_t i = 1; i <= numberOfVertices; ++i) {
        for(uint32_t j = 1; j <= i; ++j) {
            std::cout << (hasEdge(i, j) ? 1 : 0) << " ";
        }
        std::cout << std::endl;
    }
}

Graph2D::Graph2D(const Graph2D& other) {
    numberOfVertices = other.numberOfVertices;
    numberOfEdges = other.numberOfEdges;

    storage = new uint64_t*[numberOfVertices];
    vertexWeights = new uint32_t[numberOfVertices];
    degrees = new uint32_t[numberOfVertices];
    for(uint32_t i = 0; i < numberOfVertices; ++i) {
        auto count = amountOfBitVectors();
        storage[i] = new uint64_t[count];
        memcpy(storage[i], other.storage[i], count * sizeof(uint64_t));
    }
    memcpy(vertexWeights, other.vertexWeights, numberOfVertices * sizeof(uint32_t));
    memcpy(degrees, other.degrees, numberOfVertices * sizeof(uint32_t));
}

Graph::StorageOffset Graph2D::ref(uint32_t from, uint32_t to) const {
    return {
        .ref = storage[from][to >> 6UL],
        .bit = to & 63UL
    };
}

void Graph2D::setParameters(uint32_t numberOfVertices, uint32_t numberOfEdges) {
    Graph::setParameters(numberOfVertices, numberOfEdges);

    storage = new uint64_t*[numberOfVertices];
    vertexWeights = new uint32_t[numberOfVertices]();
    degrees = new uint32_t[numberOfVertices]();
    for(uint32_t i = 0; i < numberOfVertices; ++i) {
        storage[i] = new uint64_t[amountOfBitVectors()]();
    }
}

void Graph2D::addEdge(uint32_t from, uint32_t to) {
    auto off1 = ref(from - GRAPH_OFFSET, to - GRAPH_OFFSET);
    auto off2 = ref(to - GRAPH_OFFSET, from - GRAPH_OFFSET);
    off1.ref |= 1UL << off1.bit;
    off2.ref |= 1UL << off2.bit;
    ++degrees[from - GRAPH_OFFSET];
    ++degrees[to - GRAPH_OFFSET];
}

void Graph2D::removeEdge(uint32_t from, uint32_t to) {
    auto off1 = ref(from - GRAPH_OFFSET, to - GRAPH_OFFSET);
    auto off2 = ref(to - GRAPH_OFFSET, from - GRAPH_OFFSET);
    off1.ref &= ~(1UL << off1.bit);
    off2.ref &= ~(1UL << off2.bit);
    --degrees[from - GRAPH_OFFSET];
    --degrees[to - GRAPH_OFFSET];
}

uint32_t Graph2D::getDegree(uint32_t vertex) const {
    vertex -= GRAPH_OFFSET;
    /*uint32_t total = 0;
    for(uint32_t column = 0, length = amountOfBitVectors(); column < length; ++column) {
        total += std::popcount(storage[vertex][column]);
    }
    return total;*/
    return degrees[vertex];
}

void Graph2D::removeVertex(uint32_t vertex) {
    auto degree = getDegree(vertex);
    vertex -= GRAPH_OFFSET;

    // Remove from columns, therefor calculate the offset in the row and the bit.
    uint64_t bitmask = ~(1UL << (vertex & 63UL));
    uint32_t offset = vertex >> 6UL;
    for(uint32_t row = 0; row < numberOfVertices; ++row) {
        if(storage[row][offset] & ~bitmask) {
            --degrees[row];
        }
        storage[row][offset] &= bitmask;
    }

    // Remove from rows.
    for(uint32_t column = 0, length = amountOfBitVectors(); column < length; ++column) {
        storage[vertex][column] = 0;
    }

    //--numberOfVertices;
    numberOfEdges -= degree;
    degrees[vertex] = 0;
}

DynamicBitSet Graph2D::neighboursSet(uint32_t from) {
    auto neighbours = Graph2D::neighbours(from);

    DynamicBitSet result {numberOfVertices};
    for(uint32_t i = 0, length = amountOfBitVectors(); i < length; ++i) {
        result.insertBitVector(i, neighbours[i]);
    }

    return result;
}

Graph::StorageOffset LowerTriangularGraph::ref(uint32_t from, uint32_t to) const {
    // The largest number is always the row. The smallest number is the column
    uint64_t offset = from < to ? to * (to + 1) / 2 + from : from * (from + 1) / 2 + to;

    return {
        .ref = storage[offset >> 6UL],
        .bit = offset & 63UL,
    };
}

void LowerTriangularGraph::setParameters(uint32_t numberOfVertices, uint32_t numberOfEdges) {
    Graph::setParameters(numberOfVertices, numberOfEdges);

    // Calculate amount of needed bits
    size_t amountOfBits = numberOfVertices * (numberOfVertices + 1) / 2;

    storage = new uint64_t[(amountOfBits + 64 - 1) / 64]();
}

Graph::StorageOffset ListGraph::ref(uint32_t from, uint32_t to) const {
    if(from < to) {
        return {
            .ref = storage[to][from >> 6UL],
            .bit = from & 63UL,
        };
    } else {
        return {
            .ref = storage[from][to >> 6UL],
            .bit = to & 63UL,
        };
    }
}

void ListGraph::setParameters(uint32_t numberOfVertices, uint32_t numberOfEdges) {
    Graph::setParameters(numberOfVertices, numberOfEdges);

    storage = new uint64_t*[numberOfVertices];
    for(uint32_t i = 0; i < numberOfVertices; ++i) {
        storage[i] = new uint64_t[((i + 1) + 64 - 1) / 64]();
    }
}

std::unordered_set<std::pair<uint32_t, uint32_t>, pair_hash>& EdgeSetGraph::getEdges(){
    return edgeSet;
}

void EdgeSetGraph::addEdge(uint32_t from, uint32_t to){
    Graph2D::addEdge(from, to);
    edgeSet.insert(std::make_pair(from, to));
}