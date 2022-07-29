#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <stack>
#include <algorithm>
#include <vector.hpp>
#include <point_parser.hpp>

using namespace std;

stack<Vector> hull;
Vector start;

vector<Vector>::iterator getStart(vector<Vector>::iterator begin, vector<Vector>::iterator end) {
    vector<Vector>::iterator result = begin;
    vector<Vector>::iterator it = begin;
    while(it != end) {
        if(it->x > result->x) result = it;
        if(it->x == result->x && it->y > result->y) result = it;
        it++;
    }
    return result;
}

double getAngle(Vector a, Vector b) {
    return (a.y - b.y) / (a.x - b.x);
}

double getDistance(Vector a, Vector b) {
    return sqrt(pow((a.y - b.y),2)+pow((a.x - b.x),2));
}

bool compareAngles(Vector a, Vector b) {
    return getAngle(start,a) < getAngle(start,b);
}

bool compareDistance(Vector a, Vector b) {
    return getDistance(start,a) < getDistance(start,b);
}

bool compareDistanceReverse(Vector a, Vector b) {
    return getDistance(start,a) > getDistance(start,b);
}

vector<Vector> grahamScan(vector<Vector> points) {

    if (points.size() == 0) {
        return points;
    }

    vector<Vector>::iterator startIt = getStart(points.begin(), points.end());
    start = *startIt;
    points.erase(startIt);

    // sort points by polar angle with start
    sort(points.begin(), points.end(), compareAngles);

    uint32_t i = 0;
    //TODO base case points.size() == 0
    double startAngle = getAngle(start,points[0]);
    double endAngle = getAngle(start,points[points.size()-1]);
    double prev = getAngle(start,points[0]);
    int startSize = 0;
    int endSize = 0;
    
    while(i < points.size()) {
        double angle = getAngle(start,points[i]);

        if (angle == startAngle) {
            startSize++;
            i++;
            continue;
        }
        if (angle == endAngle) {
            endSize++;
            i++;
            continue;
        }


        if(angle == prev) {
            if (getDistance(points[i], start) > getDistance(points[i-1], start)) {
                points.erase(points.begin()+i-1);
            } else {
                points.erase(points.begin()+i);
            }
        } else {
            i++;
        }
        prev = angle;
    }

    sort(points.begin(), points.begin()+startSize, compareDistance);
    sort(points.end()-endSize, points.end(), compareDistanceReverse);

    // if we have less than 3 the hull consists of all these points
    if (points.size() < 3) {
        points.push_back(start);
        return points;
    }

    hull.push(start);
    hull.push(points[0]);
    hull.push(points[1]);

    for (uint32_t i=2; i<points.size(); i++) {
        Vector top = hull.top();
        if (top == points[i]) continue;
        hull.pop();
        Vector nextToTop = hull.top();
        hull.push(top);
        
        // while we go right remove the last node until we go left 
        while (orientation(nextToTop, top, points[i]) < 0) {
            hull.pop();
            top = hull.top();
            hull.pop();
            nextToTop = hull.top();
            hull.push(top);
        }
        hull.push(points[i]);
    }
    vector<Vector> result;
    while (!hull.empty()) {
        result.push_back(hull.top());
        hull.pop();
    }

    return result;
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
    vector<Vector> result = grahamScan(points);


    cout << "---" << endl;
    cout << "size: " << result.size() << endl;
    cout << "Hull found: " << endl;

    for (Vector v : result) {
        cout << "(" << v.x << ", " << v.y << ")  ";
    }
    cout << endl << "---" << endl;
    

    return 0;
}
