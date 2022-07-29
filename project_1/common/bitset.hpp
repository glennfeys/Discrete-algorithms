#pragma once

#include <cstdint>
#include <bit>
#include <cassert>
#include <utility>
#include <cstring>

class DynamicBitSet;

class DynamicBitSetIterator {
public:
    DynamicBitSetIterator(const DynamicBitSet& bitSet);

    DynamicBitSetIterator(const DynamicBitSet& bitSet, size_t offset) : currentArrayItem(0), currentArrayOffset(offset), bitSet(&bitSet) {}

    DynamicBitSetIterator& operator++();

    uint32_t operator*() const {
        return __builtin_ctzl(currentArrayItem) + (currentArrayOffset << 6U);
    }

    bool operator==(const DynamicBitSetIterator& y) const {
        return currentArrayItem == y.currentArrayItem && currentArrayOffset == y.currentArrayOffset;
    }

private:
    uint64_t currentArrayItem;
    size_t currentArrayOffset;
    const DynamicBitSet* bitSet;
};

// Because the builtin std::bitset does not allow variable size...
class DynamicBitSet {
public:
    DynamicBitSet() : storage(nullptr), arrayLength(0) {}

    DynamicBitSet(size_t amountOfBits) {
        arrayLength = (amountOfBits + 64 - 1) / 64;
        storage = new uint64_t[arrayLength]();
    }

    DynamicBitSet(DynamicBitSet&& other) : storage(std::exchange(other.storage, nullptr)), arrayLength(other.arrayLength) {}

    DynamicBitSet(const DynamicBitSet& other) : arrayLength(other.arrayLength) {
        storage = new uint64_t[arrayLength];
        memcpy(storage, other.storage, sizeof(uint64_t) * arrayLength);
    }

    DynamicBitSet& operator=(const DynamicBitSet& other) {
        delete[] storage;
        arrayLength = other.arrayLength;
        storage = new uint64_t[arrayLength];
        memcpy(storage, other.storage, sizeof(uint64_t) * arrayLength);
        return *this;
    }

    ~DynamicBitSet() {
        delete[] storage;
    }

    void clear() {
        memset(storage, 0, sizeof(uint64_t) * arrayLength);
    }

    bool empty() const {
        for(size_t i = 0; i < arrayLength; ++i) {
            if(storage[i] != 0) return false;
        }
        return true;
    }

    void fill() {
        for(size_t i = 0; i < arrayLength; ++i) {
            storage[i] = -1UL;
        }
    }

    size_t size() const {
        size_t sum = 0;
        for(size_t i = 0; i < arrayLength; ++i) {
            sum += std::popcount(storage[i]);
        }
        return sum;
    }

    DynamicBitSet intersectionWith(const DynamicBitSet& other) const {
        assert(other.arrayLength == arrayLength);

        DynamicBitSet ret {};
        ret.arrayLength = arrayLength;
        ret.storage = new uint64_t[arrayLength];
        for(size_t i = 0; i < arrayLength; ++i) {
            ret.storage[i] = storage[i] & other.storage[i];
        }

        return ret;
    }

    uint32_t ffs() const {
        for(size_t i = 0; i < arrayLength; ++i) {
            if(storage[i] != 0) {
                return __builtin_ffsl(storage[i]) - 1 + (i << 6U);
            }
        }
        return -1;
    }

    uint32_t ffus() const {
        for(size_t i = 0; i < arrayLength; ++i) {
            if(storage[i] != -1UL) {
                return __builtin_ffsl(~storage[i]) - 1 + (i << 6U);
            }
        }
        return -1;
    }

    void insert(uint32_t bit) {
        storage[bit >> 6UL] |= 1UL << (bit & 63UL);
    }

    void erase(uint32_t bit) {
        storage[bit >> 6UL] &= ~(1UL << (bit & 63UL));
    }

    bool contains(uint32_t bit) const {
        return !!(storage[bit >> 6UL] & (1UL << (bit & 63UL)));
    }

    DynamicBitSetIterator begin() const {
        return { *this };
    }

    DynamicBitSetIterator end() const {
        return { *this, arrayLength - 1 };
    }

    friend class DynamicBitSetIterator;

private:
    uint64_t* storage;
    size_t arrayLength;
};

bool operator!=(const DynamicBitSetIterator& x, const DynamicBitSetIterator& y) {
    return !(x == y);
}

DynamicBitSetIterator::DynamicBitSetIterator(const DynamicBitSet& bitSet) : currentArrayOffset(0), bitSet(&bitSet) {
    currentArrayItem = bitSet.storage[0];
    while(currentArrayItem == 0 && currentArrayOffset + 1 < bitSet.arrayLength) {
        currentArrayItem = bitSet.storage[++currentArrayOffset];
    }
}

DynamicBitSetIterator& DynamicBitSetIterator::operator++() {
    currentArrayItem &= currentArrayItem - 1U;

    while(currentArrayItem == 0 && currentArrayOffset + 1 < bitSet->arrayLength) {
        currentArrayItem = bitSet->storage[++currentArrayOffset];
    }

    return *this;
}
