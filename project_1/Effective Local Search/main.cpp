#include <iostream>
#include <cstdint>
#include <cstring>

#include <unordered_set>
#include <algorithm>

#include <clq_parser.hpp>
#include <graph.hpp>
#include <offset_array.hpp>
#include <bitset.hpp>
#include <vector>

#include <random>
#include <chrono> 

using namespace std::chrono; 

typedef uint32_t vertex;

typedef LowerTriangularGraph GraphType;

std::random_device device;
std::mt19937 generator(device());

// For each vertex this list contains the vertices in the Graph it is NOT connected to.
OffsetArray<std::vector<vertex>, 1> missing_connections;

/**
 * @brief  Single step update of the intermeidiate data structures after a single one-opt add or drop move
 * @note   
 * @param  g: the graph
 * @param  v: vertex being added or removed from clique
 * @param  is_add: Is the vertex being added?
 * @param  possible_additions: Set of all nodes that can be added
 * @param  one_missing: Set of all nodes missing a single other edge with the clique in order to be possible additions
 * @param  missing_list: List of sets containing for each node the edges they are missing with the clique as to be added (these edges are specified as the vertex in the clique they contain)
 * @param  degree_PA: The degree of each node in the Graph induced by Possible Additions.
 * @retval None
 */
void incremental_update(const GraphType &g, vertex v, bool is_add, DynamicBitSet &possible_additions, DynamicBitSet &one_missing, OffsetArray<std::unordered_set<vertex>, 1> &missing_list, OffsetArray<uint32_t, 1> &degree_PA)
{
    if (is_add)
    {
        possible_additions.erase(v);
        one_missing.insert(v);
        missing_list[v].insert(v);

        // Find all vertices missing a connection with v
        for (vertex j : missing_connections[v])
        {
            // Add these to the set keeping track of the vertices from CC j isn't connected to
            missing_list[j].insert(v);

            // If one edge is missing now it should be added to one_missing
            // And Also remove from possible additions
            if (missing_list[j].size() == 1)
            {
                possible_additions.erase(j);

                // Update the degrees in possible additions
                for (vertex v : possible_additions)
                {
                    if (j != v && g.hasEdge(j, v))
                    {
                        degree_PA[v]--;
                    }
                }
                one_missing.insert(j);
            }
            // If two edges are missing now, we remove it from one_missing
            else if (missing_list[j].size() == 2)
            {
                one_missing.erase(j);
            }
        }
    }
    else
    {
        possible_additions.insert(v);
        one_missing.erase(v);
        missing_list[v].erase(v);

        // Find all vertices missing a connection with v
        for (vertex j : missing_connections[v])
        {
            // Remove this vertex v from the set keeping track of vertices from CC j isn't connected to
            missing_list[j].erase(v);

            // If no vertices from CC are not connected to j, it becomes a possible addition
            if (missing_list[j].size() == 0)
            {
                one_missing.erase(j);
                possible_additions.insert(j);
                for (vertex v : possible_additions)
                {
                    if (j != v && g.hasEdge(j, v))
                    {
                        degree_PA[v]++;
                    }
                }
            }
            // If only one edge is missing, add to one_missing
            else if (missing_list[j].size() == 1)
            {
                one_missing.insert(j);
            }
        }
    }
}

/**
 * @brief  This method will recompute the provided auxiliary data structures for a given clique
 * @note   
 * @param  graph: Current graph
 * @param  current_clique: Current clique
 * @param  possible_additions: Set of all nodes that can be added
 * @param  one_missing: Set of all nodes missing a single other edge with the clique in order to be possible additions
 * @param  missing_list: List of sets containing for each node the edges they are missing with the clique as to be added (these edges are specified as the vertex in the clique they contain)
 * @param  degree_PA: The degree of each node in the Graph induced by Possible Additions.
 * @retval None
 */
void recompute_auxiliary_sets(const GraphType &graph, DynamicBitSet &current_clique, DynamicBitSet &possible_additions, DynamicBitSet &one_missing, OffsetArray<std::unordered_set<vertex>, 1> &missing_list, OffsetArray<uint32_t, 1> &degree_PA)
{
    possible_additions.clear();
    one_missing.clear();

    for (vertex v = 1; v <= graph.numberOfVertices; v++)
    {
        missing_list[v].clear();
        degree_PA[v] = 0;
    }

    // We'll now construct the missing_list:
    // For each of the vertices in the current clique we add them to the missing_list

    for (vertex v : current_clique)
    {
        missing_list[v].insert(v);
        for (vertex n : missing_connections[v])
        {
            missing_list[n].insert(v);
        }
    }

    // For each of the vertices in the missing list, check if they are missing 0 or 1 vertices with the current clique
    for (vertex v = 1; v <= graph.numberOfVertices; v++)
    {
        uint32_t size = missing_list[v].size();
        if (size == 0)
        {
            possible_additions.insert(v);
        }
        else if (size == 1)
        {
            one_missing.insert(v);
        }
    }

    // Finally we determine the degree in possible additions
    for (vertex v : possible_additions)
    {
        for (vertex n : possible_additions)
        {
            if (n != v && graph.hasEdge(n, v))
            {
                degree_PA[v]++;
            }
        }
    }
}

/**
 * @brief  Performs the actual k-opt-local search algorithm
 * @note   
 * @param  graph: Graph used during the search
 * @param  current_clique: Starting clique
 * @param  possible_additions: Possible additions to the current clique (i.e. nodes connected to all nodes in the current_clique)
 * @param  one_missing: Nodes missing exactly one edge with the current_clique
 * @param  missing_list: This is an array of N sets (N = number of vertices), a set at position j contains the nodes in current_clique vertex j+1 in the graph is not connected to
 * @param  degree_PA: This list will, for each node in the graph, hold the degree of that node in the graph induced by the edges of possible_additions
 * @retval A resulting clique form the k_opt_local_search
 */
DynamicBitSet &k_opt_local_search(const GraphType &graph, DynamicBitSet &current_clique, DynamicBitSet &possible_additions, DynamicBitSet &one_missing, OffsetArray<std::unordered_set<vertex>, 1> &missing_list, OffsetArray<uint32_t, 1> &degree_PA)
{
    std::uniform_int_distribution<int> distribution(1, graph.numberOfVertices);

    int g = 0;
    int g_max = 0;

    // p = {1, .., n}
    DynamicBitSet p{graph.numberOfVertices + 1};

    do
    {
        p = DynamicBitSet{graph.numberOfVertices + 1};
        p.fill();
        p.erase(0);
        g = 0;
        g_max = 0;

        DynamicBitSet previous_clique = current_clique;
        DynamicBitSet d = previous_clique;

        DynamicBitSet best_clique;

        while (!d.empty())
        {
            DynamicBitSet intersect = possible_additions.intersectionWith(p);
            vertex m;
            // if |PA /\ P| > 0
            if (!intersect.empty())
            { // ADD PHASE

                // Find the v in PA_i_P with maximum induced_degree(graph, PA, v)
                std::vector<vertex> max_degree;
                uint32_t maximum = 0;
                for (vertex v : intersect)
                {
                    if (degree_PA[v] == maximum)
                    {
                        max_degree.push_back(v);
                    }
                    else if (degree_PA[v] > maximum)
                    {
                        maximum = degree_PA[v];
                        max_degree.clear();
                        max_degree.push_back(v);
                    }
                }
                // Randomly choose vertex with maximum degree
                int rnd = distribution(generator) % max_degree.size();
                m = max_degree[rnd];

                current_clique.insert(m);
                g++;
                p.erase(m);

                if (g > g_max)
                {
                    g_max = g;
                    best_clique = current_clique;
                }
            }
            else
            { // DROP PHASE

                // find a v in the intersection of CC and P so that |PA| is maximum, i.e. the v that most commonly occurs in the Missing map's entries for One Missing items
                DynamicBitSet CC_i_P = current_clique.intersectionWith(p);

                uint32_t maximum = 0;
                std::vector<vertex> max_occurence;

                for (vertex v : CC_i_P)
                {
                    uint32_t frequency = 0;
                    for (vertex m : one_missing)
                    {
                        frequency += missing_list[m].count(v);
                    }
                    if (frequency == maximum)
                    {
                        max_occurence.push_back(v);
                    }
                    else if (frequency > maximum)
                    {
                        max_occurence.clear();
                        max_occurence.push_back(v);
                        maximum = frequency;
                    }
                }
                // if multiple vertices, choose one randomly
                int rnd = distribution(generator) % max_occurence.size();
                m = max_occurence[rnd];

                current_clique.erase(m);
                g--;
                p.erase(m);

                if (previous_clique.contains(m))
                {
                    d.erase(m);
                }
            }
            incremental_update(graph, m, !intersect.empty(), possible_additions, one_missing, missing_list, degree_PA);
        }
        // Note: When reusing the sets best_clique and previous_clique auxiliary sets have to be recomputed, or stored somewhere temporarily
        if (g_max > 0)
        {
            current_clique = std::move(best_clique);
        }
        else
        {
            current_clique = std::move(previous_clique);
        }
        recompute_auxiliary_sets(graph, current_clique, possible_additions, one_missing, missing_list, degree_PA);
    } while (g_max > 0);
    return current_clique;
}

/**
 * @brief  The main method implementing a multi start local search
 */
int main(int argc, char *argv[])
{
    srand((unsigned)time(0));
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " [filename]" << std::endl;
        return 1;
    }

    GraphType graph;
    try
    {
        ClqParser parser;
        parser.parse(*argv[1], graph);
    }
    catch (std::exception &e)
    {
        std::cerr << "Parsing failure: " << e.what() << std::endl;
        return 2;
    }

    const uint32_t N = graph.numberOfVertices;

    std::cout << "Graph of " << N << " vertices, " << graph.numberOfEdges << " edges, density " << graph.density() << std::endl;

    // Take a single vertex from the vertices in G
    // Construct a Set of all connected neighbours in PA
    // The other vertices as well as the chosen vertex v are then part of the set OM (one missing)
    DynamicBitSet best_clique{N + 1};
    //uint64_t sizes {0};
    //uint64_t runtimes {0};

    missing_connections = OffsetArray<std::vector<vertex>, 1>(N);

    // Determine in advance for each node what vertices it is NOT connected to
    for (vertex v = 1; v <= N; v++)
    {
        for (vertex o = 1; o < v; o++)
        {
            if (!graph.hasEdge(v, o))
            {
                missing_connections[v].push_back(o);
                missing_connections[o].push_back(v);
            }
        }
    }

    for (vertex v = 1; v <= N; v++)
    {
        //auto start = high_resolution_clock::now(); 
        DynamicBitSet current_clique{N + 1};
        current_clique.insert(v);

        DynamicBitSet possible_additions{N + 1};
        OffsetArray<std::unordered_set<vertex>, 1> missing_list{N};
        OffsetArray<uint32_t, 1> degree_PA{N};
        DynamicBitSet one_missing{N + 1};

        recompute_auxiliary_sets(graph, current_clique, possible_additions, one_missing, missing_list, degree_PA);
        DynamicBitSet clique = k_opt_local_search(graph, current_clique, possible_additions, one_missing, missing_list, degree_PA);
        //auto stop = high_resolution_clock::now(); 


        //sizes += clique.size();
        //runtimes += duration_cast<microseconds>(stop - start).count();

        if (clique.size() > best_clique.size())
        {
            best_clique = std::move(clique);
        }
    }

    // std::cout << "Average runtime of k-opt: " << ((double)runtimes/(double)N)*1E-6 << std::endl;
    // std::cout << "Size of avg clique: " << ((double)sizes/(double)N) << std::endl;
    std::cout << "Size of max clique: " << best_clique.size() << std::endl;
    for (vertex i : best_clique)
    {
        std::cout << i << " ";
    }
    std::cout << std::endl;

    return 0;
}
