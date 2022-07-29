#include <iostream>
#include <cstdint>
#include <vector>
#include <unordered_set>

#include <vector.hpp>
#include <point_parser.hpp>

void jarvisMarch(const std::vector<Vector> &points, std::unordered_set<Vector, vector_hash> &convexHull) {
    // Zero edge case
    if(points.size() == 0) {
        return;
    }
    
    // The starting Vector (leftmost)
    int start = 0;
    for(size_t i = 0; i < points.size(); ++i) {
        if(points[i].x < points[start].x) {
            start = i;
        }
    }

    int current = start;
    do {
        convexHull.insert(points[current]);

        // select a vector that is different from current
        int selected = (current + 1) % points.size();

        // Check for all Vectors whether it is orientated above the line current -> selected
        for(size_t i = 0; i < points.size(); ++i) {
            if(i == (size_t) current) {
                continue;
            }
            
            // If we found a point that is more counterclock wise
            double position = orientation(points[current], points[i], points[selected]);
            if(position < 0.0) {
                selected = i;
            } else if(position == 0.0) {
                // Only select the point with the smallest distance
                Vector vector1 = points[i] - points[current];
                Vector vector2 = points[selected] - points[current];
                if(vector1.length() > vector2.length()) {
                    selected = i;
                }
            }
        }

        current = selected;
    } while(current != start);
}

int main(int argc, char* argv[]) {
    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " [filename]" << std::endl;
        return 1;
    }

    std::vector<Vector> points;
    try {
        PointParser parser;
        parser.parse(*argv[1], points);
    } catch(std::exception& e) {
        std::cerr << "Parsing failure: " << e.what() << std::endl;
        return 2;
    }

    std::unordered_set<Vector, vector_hash> result;

    jarvisMarch(points, result);

    std::cout << "Convex hull: [ ";
    for (Vector point : result) {
        std::cout << point << " ";
    }
    std::cout << "]" << std::endl;
    std::cout << "Size: " << result.size() << std::endl;

    return 0;
}
