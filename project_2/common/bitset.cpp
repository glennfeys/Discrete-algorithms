#include <bitset.hpp>

uint32_t DynamicBitSetIterator::operator*() const {
    return __builtin_ctzl(currentArrayItem) + (currentArrayOffset << 6U);
}

bool DynamicBitSetIterator::operator==(const DynamicBitSetIterator& y) const {
    return currentArrayItem == y.currentArrayItem && currentArrayOffset == y.currentArrayOffset;
}

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
