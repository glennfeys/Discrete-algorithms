#pragma once

#include <cstdint>

template<typename T>
class RowMajorMatrix {
public:
    RowMajorMatrix(uint32_t w, uint32_t h) : w{w} {
        storage = new T[(size_t)w * (size_t)h];
    }

    RowMajorMatrix(const RowMajorMatrix&) = delete;

    ~RowMajorMatrix() {
        delete[] storage;
    }

    T& ref(uint32_t x, uint32_t y) {
        return storage[y * w + x];
    }

private:
    uint32_t w;
    T* storage;
};

template<typename T>
class ColumnMajorMatrix {
public:
    ColumnMajorMatrix(uint32_t w, uint32_t h) : h{h} {
        storage = new T[(size_t)w * (size_t)h];
    }

    ColumnMajorMatrix(const ColumnMajorMatrix&) = delete;

    ~ColumnMajorMatrix() {
        delete[] storage;
    }

    T& ref(uint32_t x, uint32_t y) {
        return storage[x * h + y];
    }

private:
    uint32_t h;
    T* storage;
};
