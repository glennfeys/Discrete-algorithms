#pragma once

#include <vector>
#include <cstdint>

template<typename T>
class SliceIterator {
public:
    SliceIterator(T* pointer) : pointer(pointer) {}

    const SliceIterator<T>& operator ++() {
        ++pointer;
        return *this;
    }

    bool operator ==(const SliceIterator<T>& other) const {
        return pointer == other.pointer;
    }

    T operator*() const {
        return *pointer;
    }

private:
    T* pointer;
};

template<typename T>
bool operator !=(const SliceIterator<T>& self, const SliceIterator<T>& other) {
    return !(self == other);
}

template<typename T>
class Slice {
public:
    Slice(T* elements, size_t length) : elements(elements), _length(length) {}
    Slice(std::vector<T>& elements) : elements(elements.data()), _length(elements.size) {}
    ~Slice() {}

    T& operator [](size_t index) {
        return elements[index];
    }

    const T& operator [](size_t index) const {
        return elements[index];
    }

    Slice slice(size_t offset) const {
        if(offset < _length)
            return { elements + offset, _length - offset };
        else
            return { elements + _length, 0 };
    }

    Slice slice(size_t offset, size_t length) const {
        return { elements + offset, _length };
    }

    size_t length() const {
        return _length;
    }

    const SliceIterator<T> begin() const {
        return { elements };
    }

    const SliceIterator<T> end() const {
        return { elements + _length };
    }

private:
    T* elements;
    size_t _length;
};
