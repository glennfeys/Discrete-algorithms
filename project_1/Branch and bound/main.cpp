#include <iostream>
#include <cstdint>
#include <cstring>
#include <algorithm>

#include <matrix.hpp>
#include <offset_array.hpp>
#include <graph.hpp>
#include <clq_parser.hpp>
#include <bitset.hpp>

typedef LowerTriangularGraph GraphType;

//#define DO_BENCHMARK

#ifdef DO_BENCHMARK
#include <benchmark/benchmark.h>
#endif

struct Result {
    OffsetArray<uint32_t, 1> best;
    OffsetArray<uint32_t, 1> actNode;
    uint32_t maxClique;
};

/**
 * @brief   Finds an upper bound greedily for the chromatic number in the complementary graph.
 * @param g The graph
 * @retval  Upper bound for chromatic number in complementary graph
 */
uint32_t upperBoundChromaticNumberComplementaryGraph(const GraphType& graph) {
    OffsetArray<uint32_t, 1> colors { graph.numberOfVertices };
    colors[1] = 1;
    for(uint32_t i = 2; i <= graph.numberOfVertices; ++i) {
        colors[i] = 0;
    }

    DynamicBitSet unavailable { graph.numberOfVertices + 1 };

    for(uint32_t i = 2; i <= graph.numberOfVertices; ++i) {
        unavailable.insert(0); // Sentinel

        for(uint32_t j = 1; j <= graph.numberOfVertices; ++j) {
            // Note the '!': this makes the graph complementary!
            if(!graph.hasEdge(i, j) && colors[j] != 0) {
                unavailable.insert(colors[j]);
            }
        }

        colors[i] = unavailable.ffus();

        unavailable.clear();
    }

    uint32_t colorsUsed = 1;
    for(uint32_t i = 1; i <= graph.numberOfVertices; ++i) {
        colorsUsed = std::max(colorsUsed, colors[i]);
    }
    return colorsUsed;
}

/**
 * @brief   Search for the maximum clique using branch & bound
 * @param g The graph
 * @retval  A result structure containing the best clique, its size, and the mapping from sorted -> original vertices.
 */
Result search(GraphType& graph) {
    const uint32_t N = graph.numberOfVertices;

    OffsetArray<uint32_t, 1> actNode = graph.sortByEdgeDensity(0.40f, std::min_element);

    // The independent set number (alpha) and chromatic number (X) are related as follows:
    //     X >= |V| / alpha(G)
    // <=> alpha(G) >= |V| / X
    // However, the independent set number in the complementary graph is the clique number in the normal graph.
    // Therefore, if we find an upper bound for X in the complementary graph, we can calculate a lower bound as |V|/X.
    uint32_t greedyColorsInComplement = upperBoundChromaticNumberComplementaryGraph(graph);
    // Size of the largest currently known clique.
    // Can be set initially to zero for the initial guess, but we try to guess better.
    uint32_t maxClique = graph.numberOfVertices / greedyColorsInComplement;
    // Check if the heuristic of Turan is better.
    if(graph.numberOfVertices * graph.numberOfVertices > 2 * graph.numberOfEdges) {
        maxClique = std::max(maxClique, graph.numberOfVertices * graph.numberOfVertices / (graph.numberOfVertices * graph.numberOfVertices - 2 * graph.numberOfEdges));
    }

    std::cout << "Initial lower bound for max clique size: " << maxClique << std::endl;

    uint32_t currentDepth = 1;

    OffsetArray<uint32_t, 1> start { N };
    OffsetArray<uint32_t, 1> last { N };
    start[1] = 0;
    last[1] = N;
    OffsetArray<uint32_t, 1> best { N };
    bool storedBest = false;

    ColumnMajorMatrix<uint32_t> adj(N, N);
    for(uint32_t col = 1; col <= N; ++col) {
        adj.ref(0, col - 1) = col;
    }
    
    //DynamicBitSet colorsUsed { N + 1 };

    while(currentDepth > 0) {
        //std::cout << currentDepth << " " << maxClique<<std::endl;
        ++start[currentDepth];

        if(currentDepth + last[currentDepth] - start[currentDepth] > maxClique || !storedBest) {
            uint32_t previousDepth = currentDepth;
            uint32_t from = adj.ref(previousDepth - 1, start[previousDepth] - 1);
            //colorsUsed.insert(colorMap[from]);
            ++currentDepth;
            start[currentDepth] = 0;
            last[currentDepth] = 0;

            // Determine node for next depth
            for(uint32_t col = start[previousDepth] + 1, end = last[previousDepth]; col <= end; ++col) {
                uint32_t to = adj.ref(previousDepth - 1, col - 1);

                if(graph.hasEdge(from, to)/* && !colorsUsed.contains(colorMap[to])*/) {
                    adj.ref(currentDepth - 1, last[currentDepth]) = to;
                    ++last[currentDepth];
                }
            }

            // If the next depth doesn't contain any nodes, see if a new maxClique has been found and return to previous depth
            if(last[currentDepth] == 0) {
                //colorsUsed.erase(colorMap[from]);
                --currentDepth;
                if(currentDepth > maxClique || !storedBest) {
                    maxClique = currentDepth;
                    for(uint32_t col = 1; col <= currentDepth; ++col) {
                        best[col] = adj.ref(col - 1, start[col] - 1);
                    }
                    storedBest = true;
                    std::cout << "Found new best " << maxClique << std::endl;
                }
            }
        } else {
            // Prune, further expansion would not find a better incumbent
            --currentDepth;
            if(currentDepth > 0) {
                //colorsUsed.erase(colorMap[adj.ref(currentDepth - 1, start[currentDepth] - 1)]);
            }
        }
    }

    return {
        .best = std::move(best),
        .actNode = std::move(actNode),
        .maxClique = maxClique,
    };
}

#ifdef DO_BENCHMARK

static const char* benchFileNames[] = {
    "test",
    "/home/niels/Downloads/DSJC500_5.clq",
};

void BM_main(benchmark::State& state) {
    //std::cout << state.range(0) << std::endl;
    GraphType graph;
    ClqParser parser;
    parser.parse(*benchFileNames[state.range(0)], graph);
    for(auto _ : state) {
        benchmark::DoNotOptimize(search(graph));
    }
}
BENCHMARK(BM_main)->Threads(2)->DenseRange(0, sizeof(benchFileNames) / sizeof(benchFileNames[0]), 1);

BENCHMARK_MAIN();

#else

int main(int argc, char* argv[]) {
    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " [filename]" << std::endl;
        return 1;
    }

    GraphType graph;
    try {
        ClqParser parser;
        parser.parse(*argv[1], graph);
    } catch(std::exception& e) {
        std::cerr << "Parsing failure: " << e.what() << std::endl;
        return 2;
    }

    std::cout << "Graph of " << graph.numberOfVertices << " vertices, " << graph.numberOfEdges << " edges, density " << graph.density() << std::endl;

    auto result = search(graph);

    std::cout << "Size of max clique: " << result.maxClique << std::endl;
    for(uint32_t i = 1; i <= result.maxClique; ++i) {
        std::cout << result.actNode[result.best[i]] << " ";
    }
    std::cout << std::endl;
    
    return 0;
}

#endif
