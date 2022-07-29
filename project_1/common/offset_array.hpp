#pragma once

#include <cstdint>
#include <utility>

template<typename T, size_t O>
class OffsetArray {
public:
    inline OffsetArray(size_t size) {
        storage = (new T[size]) - O;
    }

    inline OffsetArray() : storage(nullptr) {}

    inline OffsetArray(T*& storage) noexcept : storage(std::exchange(storage, nullptr) - O) {}

    OffsetArray(OffsetArray&& other) noexcept : storage(std::exchange(other.storage, nullptr)) {}

    OffsetArray& operator=(OffsetArray&& other) {
        storage = std::exchange(other.storage, nullptr);
        return *this;
    }

    OffsetArray(const OffsetArray& other) = delete;

    inline ~OffsetArray() {
        if(storage == nullptr) return;
        delete[] (storage + O);
        storage = nullptr;
    }

    inline T& operator[](size_t index) {
        return storage[index];
    }

    inline T* ptr() {
        return (storage + O);
    }

private:
    T* storage;
};
