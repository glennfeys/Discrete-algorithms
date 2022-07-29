#include <iostream>
#include <graph.hpp>
#include <algorithm>
#include <clq_weighted_parser.hpp>
#include <unordered_set>
#include <map>
#include <math.h>
#include <vector>
#include <utility>
#include <functional>
#include <cstring>

using namespace std;

typedef uint32_t Vertex;
typedef unordered_set<Vertex> VertexCover;
typedef pair<Vertex, Vertex> Edge;

uint32_t get_max_degree(const EdgeSetGraph &g, Edge e)
{
    return max(g.getDegree(e.first), g.getDegree(e.second));
}

uint32_t get_min_degree(const EdgeSetGraph &g, Edge e)
{
    return min(g.getDegree(e.first), g.getDegree(e.second));
}

uint32_t get_total_degree(const EdgeSetGraph &g, Edge e)
{
    return g.getDegree(e.first) + g.getDegree(e.second);
}

uint32_t get_max_weight(const EdgeSetGraph &g, Edge e)
{
    return max(g.getWeight(e.first), g.getWeight(e.second));
}

uint32_t get_min_weight(const EdgeSetGraph &g, Edge e)
{
    return min(g.getWeight(e.first), g.getWeight(e.second));
}

uint32_t get_first_node_weight(const EdgeSetGraph &g, Edge e)
{
    return e.first;
}

vector<Edge> sortedEdges(EdgeSetGraph &g, function<uint32_t(const EdgeSetGraph &g, Edge e)> sortProperty)
{
    vector<Edge> result;
    for (Edge edge : g.getEdges())
    {
        result.push_back(edge);
    }

    sort(result.begin(), result.end(), [&](const Edge &a, const Edge &b) -> bool {
        return sortProperty(g, a) > sortProperty(g, b);
    });

    return result;
}

pair<VertexCover, uint32_t> pricing_method(EdgeSetGraph &g, map<Vertex, uint32_t> &local_weights, function<uint32_t(const EdgeSetGraph &g, Edge e)> sortProperty)
{
    VertexCover vertexCover;
    uint32_t total_weight = 0;

    auto edges = sortedEdges(g, sortProperty);

    for (Edge edge : edges)
    {
        int from = edge.first;
        int to = edge.second;

        if (vertexCover.count(from) != 0 || vertexCover.count(to) != 0)
        { // Edge is paid for
            continue;
        }

        uint32_t min_vertex;
        uint32_t max_vertex;

        if (local_weights[from] < local_weights[to])
        {
            min_vertex = from;
            max_vertex = to;
        }
        else
        {
            min_vertex = to;
            max_vertex = from;
        }

        uint32_t minimum_weight = local_weights[min_vertex];
        local_weights[min_vertex] = 0;
        local_weights[max_vertex] -= minimum_weight;
        total_weight += g.getWeight(min_vertex);
        vertexCover.insert(min_vertex);
    }

    return make_pair(vertexCover, total_weight);
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    if (argc < 2 || argc > 3)
    {
        cerr << "Usage: " << argv[0] << " [filename]" << endl;
        return 1;
    }

    auto ordering_function = get_first_node_weight;

    if (argc == 3)
    {
        if (strcmp(argv[2], "max_degree") == 0)
        {
            ordering_function = get_max_degree;
        }
        else if (strcmp(argv[2], "min_degree") == 0)
        {
            ordering_function = get_min_degree;
        }
        else if (strcmp(argv[2], "total_degree") == 0)
        {
            ordering_function = get_total_degree;
        }
        else if (strcmp(argv[2], "max_weight") == 0)
        {
            ordering_function = get_max_weight;
        }
        else if (strcmp(argv[2], "min_weight") == 0)
        {
            ordering_function = get_min_weight;
        }
    }

    EdgeSetGraph graph;
    try
    {
        ClqWeightedParser parser;
        parser.parse(*argv[1], graph);
    }
    catch (exception &e)
    {
        cerr << "Parsing failure: " << e.what() << endl;
        return 2;
    }

    const uint32_t N = graph.numberOfVertices;

    map<Vertex, uint32_t> weights;

    for (Vertex v = 1; v <= N; v++)
    {
        weights[v] = graph.getWeight(v);
    }

    cout << "Graph of " << N << " vertices, " << graph.numberOfEdges << " edges, density " << graph.density() << endl;
    pair<VertexCover, uint32_t> result = pricing_method(graph, weights, ordering_function);
    cout << "Minimum weight of vertex cover: " << result.second << endl;
    for (auto vertex : result.first)
    {
        cout << vertex << " ";
    }
    cout << endl;
    return 0;
}