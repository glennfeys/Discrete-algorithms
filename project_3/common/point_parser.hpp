#pragma once

#include <iostream>
#include <fstream>
#include <vector>

#include <vector.hpp>

class PointParser final {
public:
    PointParser() {}
    PointParser(const PointParser&) = delete;
    ~PointParser() {}

    void parse(const char& fileName, std::vector<Vector>& points) const {
        std::ifstream inputFile(&fileName);
        if(!inputFile) {
            throw std::runtime_error("Could not open source file");
        }

	double x, y;
        while((inputFile >> x) && (inputFile >> y)) {
            points.emplace_back(x, y);
        }
    }
};
