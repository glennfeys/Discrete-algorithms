#pragma once

#include <cstdint>

template<typename T, size_t O>
class OffsetArray;

class Graph {
protected:
    struct StorageOffset {
        uint64_t& ref;
        uint64_t bit;
    };

public:
    uint32_t numberOfVertices { 0 };
    uint32_t numberOfEdges { 0 };

    Graph() {}
    Graph(const Graph&) = delete;
    virtual ~Graph() {}

    virtual StorageOffset ref(uint32_t, uint32_t) const = 0;

    virtual void setParameters(uint32_t numberOfVertices, uint32_t numberOfEdges) {
        this->numberOfVertices = numberOfVertices;
        this->numberOfEdges = numberOfEdges;
    }

    virtual void addEdge(uint32_t from, uint32_t to);
    virtual void removeEdge(uint32_t from, uint32_t to);
    bool hasEdge(uint32_t from, uint32_t to) const;
    void swapEdge(uint32_t from1, uint32_t to1, uint32_t from2, uint32_t to2);
    void swapVertex(uint32_t a, uint32_t b);
    float density() const;
    void sortByEdgeDensity(uint32_t* actNode, uint32_t* (*finder)(uint32_t*, uint32_t*));
    OffsetArray<uint32_t, 1> sortByEdgeDensity(float densityThreshold, uint32_t* (*finder)(uint32_t*, uint32_t*));
    virtual void printAdjacencyMatrix() const;
};

class Graph2D final : public Graph {
protected:
    Graph::StorageOffset ref(uint32_t from, uint32_t to) const override;

    uint64_t** storage {nullptr};

public:
    Graph2D() {}
    ~Graph2D() { 
        for(uint32_t i = 0; i < numberOfVertices; ++i) {
            delete[] storage[i];
        }

        delete[] storage;
    }
    
    void setParameters(uint32_t numberOfVertices, uint32_t numberOfEdges) override;
    void addEdge(uint32_t from, uint32_t to) override;
    void removeEdge(uint32_t from, uint32_t to) override;
    void printAdjacencyMatrix() const override;

    uint64_t degree(uint32_t node) const {
        uint32_t amountOfBitVectors = (numberOfVertices + 64 - 1) / 64;

        uint32_t degree = 0;
        for(uint32_t i = 0; i < amountOfBitVectors; ++i) {
            degree += __builtin_popcount(storage[node - 1][i]);
        }
        return degree;
    }

    const uint64_t* neighbours(uint32_t from) const {
        return storage[from - 1];
    }
};

class LowerTriangularGraph final : public Graph {
protected:
    Graph::StorageOffset ref(uint32_t from, uint32_t to) const override;

    uint64_t* storage {nullptr};

public:
    LowerTriangularGraph() {}
    ~LowerTriangularGraph() { delete[] storage; }

    void setParameters(uint32_t numberOfVertices, uint32_t numberOfEdges) override;
};

class ListGraph final : public Graph {
protected:
    Graph::StorageOffset ref(uint32_t from, uint32_t to) const override;

    uint64_t** storage {nullptr};

public:
    ListGraph() {}
    ~ListGraph() {
        for(uint32_t i = 0; i < numberOfVertices; ++i) {
            delete[] storage[i];
        }

        delete[] storage;
    }

    void setParameters(uint32_t numberOfVertices, uint32_t numberOfEdges) override;
};
