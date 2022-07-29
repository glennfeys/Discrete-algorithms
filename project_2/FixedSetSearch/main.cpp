#include <iostream>
#include <cstdint>
#include <algorithm>
#include <numeric>
#include <random>
#include <cassert>
#include <set>

#include <matrix.hpp>
#include <graph.hpp>
#include <clq_weighted_parser.hpp>
#include <bitset.hpp>

std::random_device device;
std::mt19937 generator(device());

typedef Graph2D GraphType;
typedef uint32_t Count;
typedef uint32_t Vertex;

void calculateUnSetRaw(const GraphType& graph, DynamicBitSet& Un, Vertex v) {
    // Note: can't calculate score here because we might count some elements double depending on what is in Un already.
    // Un(v, S) = if v is in S, => set of vertices of all neighbours of v not in S
    auto neighbours = graph.neighbours(v);
    for(uint32_t i = 0, length = graph.amountOfBitVectors(); i < length; ++i) {
        Un.insertBitVector(i, neighbours[i]);
        //std::cout << "N[" << i<<"] = " << neighbours[i] << std::endl;
    }
}

std::pair<DynamicBitSet, float> calculateUnSet(const GraphType& graph, const DynamicBitSet& S, Vertex v) {
    DynamicBitSet Un { graph.numberOfVertices };
    calculateUnSetRaw(graph, Un, v);
    Un.eraseAll(S);
    float evSwap = graph.getWeight(v);
    for(auto u : Un) {
        evSwap -= graph.getWeight(u);
    }
    return std::make_pair(Un, evSwap);
}

std::pair<DynamicBitSet, float> calculateUnSet(const GraphType& graph, const DynamicBitSet& S, Vertex u, Vertex v) {
    DynamicBitSet Un { graph.numberOfVertices };
    calculateUnSetRaw(graph, Un, u);
    calculateUnSetRaw(graph, Un, v);
    Un.eraseAll(S);
    float evSwap = graph.getWeight(u) + graph.getWeight(v);
    for(auto u : Un) {
        evSwap -= graph.getWeight(u);
    }
    return std::make_pair(Un, evSwap);
}

bool hasCommonNeighbours(const GraphType& graph, Vertex u, Vertex v) {
    auto neighbours1 = graph.neighbours(u);
    auto neighbours2 = graph.neighbours(v);
    for(uint32_t i = 0, l = graph.amountOfBitVectors(); i < l; ++i) {
        if(neighbours1[i] & neighbours2[i]) {
            return true;
        }
    }
    return false;
}

void localSearch(const GraphType& graph, DynamicBitSet& S) {
    std::vector<DynamicBitSet> impItems;
    std::vector<DynamicBitSet> impPairItems;
    bool improved;

    do {
        // Imp swaps.
        do {
            improved = false;

            // Calculate impItems
            for(auto v : S) {
                auto [Un, evSwap] = calculateUnSet(graph, S, v);

                if(evSwap > 0.0f) {
                    //std::cout << "evSwap imp: " << evSwap << std::endl;
                    // Swap(v, S) = S | Un(v, S) \ {v}
                    DynamicBitSet swap { S };
                    swap.insertAll(Un);
                    swap.erase(v);
                    impItems.push_back(std::move(swap));
                }
            }

            if(!impItems.empty()) {
                // Select random improvement.
                std::uniform_int_distribution<uint32_t> distribution(0, impItems.size() - 1);
                S = std::move(impItems[distribution(generator)]);
                improved = true;
            }

            // Reset for next loop, no cumulative actions because they might've become invalid or no longer improving.
            impItems.clear();
        } while(improved);

        // Calculate ImpPairItems.
        for(auto u : S) {
            for(auto v : S) {
                if(u != v && !graph.hasEdge(u, v) && hasCommonNeighbours(graph, u, v)) {
                    auto [Un, evSwap] = calculateUnSet(graph, S, u, v);
                    //std::cout << "{u,v}=" << u <<","<<v<<std::endl;
                    //std::cout << "Un: ";
                    //for(auto x : Un) {
                    //    std::cout << x << " ";
                    //}
                    //std::cout << std::endl;

                    if(evSwap > 0.0f) {
                        //std::cout << "evSwap impPair: " << evSwap << std::endl;
                        // Swap(u, v, S) = S | Un(u, v, S) \ {u, v}
                        DynamicBitSet swap { S };
                        swap.insertAll(Un);
                        swap.erase(u);
                        swap.erase(v);
                        impPairItems.push_back(std::move(swap));
                    }
                }
            }
        }

        if(!impPairItems.empty()) {
            // Select random improvement.
            std::uniform_int_distribution<uint32_t> distribution(0, impPairItems.size() - 1);
            S = std::move(impPairItems[distribution(generator)]);
            impPairItems.clear();
            improved = true;
        }
    } while(improved);
}

void rgc(GraphType& graph /* will modify, please give a copy */, DynamicBitSet& partialSolution) {
    auto heuristic = [&](Vertex n) {
        //std::cout << graph.getDegree(n) << " " << graph.getWeight(n) << std::endl;
        return (float) graph.getDegree(n) / graph.getWeight(n);
    };

    while(graph.numberOfEdges > 0) {
        // Restricted Candidate List:
        // First find the vertices V \ partialSolution, and pick one randomly with
        // the largest heuristic score.
        std::vector<Vertex> candidates;
        float currentBest = -1.0f;
        for(Vertex v = 0; v < graph.numberOfVertices; ++v) {
            if(partialSolution.contains(v)) continue;
            float myHeuristic = heuristic(v);
            if(myHeuristic > currentBest) {
                currentBest = myHeuristic;
                candidates.clear();
                candidates.push_back(v);
            } else if(fabsf(myHeuristic - currentBest) < __FLT_EPSILON__) {
                candidates.push_back(v);
            }
        }

        std::uniform_int_distribution<uint32_t> distribution(0, candidates.size() - 1);
        Vertex n = candidates[distribution(generator)];

        graph.removeVertex(n);
        //std::cout << "Add " << n << " " << graph.numberOfEdges << std::endl;
        partialSolution.insert(n);
        //std::cout << "---"<<std::endl;
    }
}

float totalWeight(const GraphType& graph, const DynamicBitSet& S) {
    float w = 0.0f;
    for(auto v : S) {
        w += graph.getWeight(v);
    }
    return w;
}

void rgcAndLocalSearch(const GraphType& graph, DynamicBitSet& S) {
    // rgc uses a copy, but localSearch should use the original,
    // so the first one is passed by copy and the other by reference.
    GraphType copy = graph;
    for(auto v : S) {
        copy.removeVertex(v);
    }
    rgc(copy, S);
    localSearch(graph, S);
}

DynamicBitSet grasp(const GraphType& graph, int maxItNoImprovement) {
    DynamicBitSet best;
    int itNoImprovement = 0;
    float weight = std::numeric_limits<float>::infinity();
    while(itNoImprovement < maxItNoImprovement) {
        DynamicBitSet S { graph.numberOfVertices };
        rgcAndLocalSearch(graph, S);
        auto newWeight = totalWeight(graph, S);
        if(newWeight < weight) {
            //std::cout << newWeight << " " << weight << std::endl;
            weight = newWeight;
            best = S;
            itNoImprovement = 0;
        } else {
            ++itNoImprovement;
        }
    }
    return best;
}

size_t calculateWantedSize(size_t setSize, uint32_t i) {
    return setSize - (setSize >> i);
}

struct ScoredSolution {
    DynamicBitSet solution;
    float score;

    ScoredSolution(DynamicBitSet&& solution, float score) : solution(std::move(solution)), score(score) {};

    bool operator<(const ScoredSolution& other) const {
        return solution < other.solution;
    }
};

DynamicBitSet fix(uint32_t numberOfVertices, DynamicBitSet& B, std::vector<ScoredSolution*>& S_kn, size_t size) {
    // First, count how many times each vertex occurs in every set of S_kn
    uint32_t* counts = new uint32_t[numberOfVertices];
    for(auto& S : S_kn) {
        for(auto v : S->solution) {
            ++counts[v];
        }
    }

    DynamicBitSet S { numberOfVertices };

    // Now, find up to "size" vertices that occur the most that are also in B.
    for(size_t i = 0; i < size; ++i) {
        // Select best.
        uint32_t bestCandidate = 0;
        uint32_t bestCount = 0;
        for(auto v : B) {
            if(!S.contains(v) && counts[v] > bestCount) {
                bestCount = counts[v];
                bestCandidate = v;
            }
        }
        if(bestCount == 0) {
            break;
        } else {
            S.insert(bestCandidate);
        }
    }

    delete[] counts;

    return S;
}

DynamicBitSet fss(GraphType& graph) {
    constexpr size_t initialPopulationSize = 10;
    constexpr int maxItNoImprovement = 50;
    constexpr int k = 5;
    assert(k < initialPopulationSize);

    DynamicBitSet bestSolution { graph.numberOfVertices };

    // Generate the initial population.
    std::set<ScoredSolution*> population;
    for(size_t i = 0; i < initialPopulationSize; ++i) {
        auto solution = grasp(graph, maxItNoImprovement);
        float score = totalWeight(graph, solution);
        population.insert(new ScoredSolution(std::move(solution), score));
    }

    int itNoImprovement = 0;
    float bestWeight = std::numeric_limits<float>::infinity();
    uint32_t sizeCounter = 1;
    while(itNoImprovement < maxItNoImprovement) {
        std::cout << "itNoImprovement: " << itNoImprovement << std::endl;

        // 1. Calculate S_kn as k random elements from the n best from the population.
        std::vector<ScoredSolution*> S_kn(std::min(population.size(), initialPopulationSize));
        std::partial_sort_copy(std::begin(population), std::end(population), std::begin(S_kn), std::end(S_kn), [](const auto a, const auto b) {
            return a->score > b->score;
        });
        // 2. Yes, this is step 2, but we need to find another random solution before we remove it.
        //    Set B to a random solution in Pm.
        std::uniform_int_distribution<uint32_t> distribution(0, S_kn.size() - 1);
        uint32_t bIdx = distribution(generator);
        auto B = S_kn[bIdx];
        // 1. a) We want to keep keep k elements, so remove |S_kn| - k elements.
        while(S_kn.size() > k) {
            std::uniform_int_distribution<uint32_t> distribution(0, S_kn.size() - 1);
            uint32_t idx = distribution(generator);
            S_kn.erase(std::begin(S_kn) + idx);
        }

        // 3. Calculate Fix(...)
        auto size = calculateWantedSize(B->solution.size(), sizeCounter);
        auto S = fix(graph.numberOfVertices, B->solution, S_kn, size); // Actually F at this line, but will be evolved into S by following code.

        // 4. Calculate S and apply local search.
        rgcAndLocalSearch(graph, S);

        // 5. Update the mainloop and decide what to do next.
        float newWeight = totalWeight(graph, S);
        if(newWeight < bestWeight) {
            bestWeight = newWeight;
            bestSolution = S;
            itNoImprovement = 0;
        } else {
            ++itNoImprovement;
            ++sizeCounter;
        }
        auto newItem = new ScoredSolution(std::move(S), newWeight);
        auto [_it, didInsert] = population.insert(newItem);
        if(!didInsert) {
            delete newItem;
        }
    }

    for(auto x : population) {
        delete x;
    }

    return bestSolution;
}

int main(int argc, char* argv[]) {
    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " [filename]" << std::endl;
        return 1;
    }

    GraphType graph;
    try {
        ClqWeightedParser parser;
        parser.parse(*argv[1], graph);
    } catch(std::exception& e) {
        std::cerr << "Parsing failure: " << e.what() << std::endl;
        return 2;
    }

    const Count N = graph.numberOfVertices;
    std::cout << "Graph of " << N << " vertices, " << graph.numberOfEdges << " edges, density " << graph.density() << std::endl;

    auto solution = fss(graph);
    std::cout << "Minimum weighted vertex cover: [ ";
    for(auto vertex : solution) {
        std::cout << vertex + 1 << " ";
    }
    std::cout << "], total weight: " << totalWeight(graph, solution) << std::endl;

    return 0;
}
