#pragma once

#ifndef GRAPH_STARTS_AT_ZERO
#define GRAPH_OFFSET 1
#else
#define GRAPH_OFFSET 0
#endif
#include <cstdint>
#include <unordered_set>
#include <utility>

template<typename T, size_t O>
class OffsetArray;
class DynamicBitSet;

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
    OffsetArray<uint32_t, GRAPH_OFFSET> sortByEdgeDensity(float densityThreshold, uint32_t* (*finder)(uint32_t*, uint32_t*));
    void printAdjacencyMatrix() const;
};

class Graph2D : public Graph {
protected:
    Graph::StorageOffset ref(uint32_t from, uint32_t to) const override;

    uint64_t** storage {nullptr};
    uint32_t* vertexWeights {nullptr};
    uint32_t* degrees {nullptr};

public:
    Graph2D() {}
    Graph2D(const Graph2D&);
    ~Graph2D() { 
        for(uint32_t i = 0; i < numberOfVertices; ++i) {
            delete[] storage[i];
        }

        delete[] storage;
        delete[] vertexWeights;
        delete[] degrees;
    }

    void setParameters(uint32_t numberOfVertices, uint32_t numberOfEdges) override;
    void addEdge(uint32_t from, uint32_t to) override;
    void removeEdge(uint32_t from, uint32_t to) override;
    void removeVertex(uint32_t vertex);
    uint32_t getDegree(uint32_t vertex) const;

    inline uint32_t amountOfBitVectors() const {
        return (numberOfVertices + 64 - 1) / 64;
    }

    inline const uint64_t* neighbours(uint32_t from) const {
        return storage[from - GRAPH_OFFSET];
    }
    DynamicBitSet neighboursSet(uint32_t from);

    inline void setWeight(uint32_t vertex, uint32_t weight) {
        vertexWeights[vertex - GRAPH_OFFSET] = weight;
    }

    inline uint32_t getWeight(uint32_t vertex) const {
        return vertexWeights[vertex - GRAPH_OFFSET];
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

class ListGraph final: public Graph {
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

struct pair_hash
{
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2> &pair) const
    {
        return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
};

class EdgeSetGraph : public Graph2D {
protected:
    std::unordered_set<std::pair<uint32_t, uint32_t>, pair_hash> edgeSet;

public:
    EdgeSetGraph() {}
    ~EdgeSetGraph() {}

    void addEdge(uint32_t from, uint32_t to) override;
    std::unordered_set<std::pair<uint32_t, uint32_t>, pair_hash>& getEdges();
};
