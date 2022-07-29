#include <iostream>
#include <cstdint>
#include <vector>
#include <math.h>

#include <vector.hpp>
#include <point_parser.hpp>
#include <unordered_set>

double distance(const Vector &v1, const Vector &v2, const Vector &v)
{
    return (v.y - v1.y) * (v2.x - v1.x) -
           (v2.y - v1.y) * (v.x - v1.x);
}

int determine_side(const Vector &v1, const Vector &v2, const Vector &v)
{
    auto dist = distance(v1, v2, v);
    if (dist < 0.0)
    {
        return -1;
    }
    else if (dist > 0.0)
    {
        return 1;
    }
    return 0;
}

void quickHull(std::unordered_set<Vector, vector_hash> &points, const Vector &v1, const Vector &v2, std::unordered_set<Vector, vector_hash> &convexHull)
{

    if (points.size() == 0)
        return;
    if (points.size() == 1)
    {
        Vector v = *points.begin();
        convexHull.insert(v);
        return;
    }

    Vector point = *points.begin();
    double max_dist = 0.0;

    for (Vector v : points)
    {
        double dist = abs(distance(v1, v2, v));
        if (dist > max_dist)
        {
            point = v;
            max_dist = dist;
        }
    }
    convexHull.insert(point);

    std::unordered_set<Vector, vector_hash> left_set;
    std::unordered_set<Vector, vector_hash> right_set;
    for (Vector v : points)
    {
        if (determine_side(point, v1, v) == -1)
            left_set.insert(v);
        else if (determine_side(point, v2, v) == 1)
            right_set.insert(v);
    }

    quickHull(left_set, v1, point, convexHull);
    quickHull(right_set, point, v2, convexHull);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " [filename]" << std::endl;
        return 1;
    }

    std::vector<Vector> input_points;
    try
    {
        PointParser parser;
        parser.parse(*argv[1], input_points);
    }
    catch (std::exception &e)
    {
        std::cerr << "Parsing failure: " << e.what() << std::endl;
        return 2;
    }
    std::unordered_set<Vector, vector_hash> points(input_points.begin(), input_points.end());

    if (points.size() < 3)
    {
        std::cout << "At least three points are needed for determining a convex hull." << std::endl;
    }

    std::unordered_set<Vector, vector_hash> result;

    Vector max_x = *points.begin();
    Vector min_x = *points.begin();

    for (Vector v : points)
    {
        if (v.x > max_x.x)
        {
            max_x = v;
        }
        else if (v.x < min_x.x)
        {
            min_x = v;
        }
    }
    points.erase(min_x);
    points.erase(max_x);
    
    std::unordered_set<Vector, vector_hash> left_set;
    std::unordered_set<Vector, vector_hash> right_set;
    for (Vector v : points)
    {
        if (determine_side(min_x, max_x, v) == -1)
            left_set.insert(v);
        else if (determine_side(min_x, max_x, v) == 1)
            right_set.insert(v);
    }
    result.insert(min_x);
    result.insert(max_x);

    quickHull(right_set, min_x, max_x, result);
    quickHull(left_set, max_x, min_x, result);

    std::cout << "Convex hull: [ ";
    for (Vector point : result)
    {
        std::cout << point << " ";
    }
    std::cout << "]" << std::endl;
    std::cout << "Size: " << result.size() << std::endl;

    return 0;
}