/**
 *
 *
 * @file PointerRange.h
 * @brief 
 * @author clonker
 * @date 10/10/18
 */
#pragma once

#include <algorithm>
#include <functional>

namespace crampl {

template<typename PointerType, typename Compare = std::less<std::decay_t<decltype(*std::declval<PointerType>())>>>
struct PointerRange {
    template<typename SizeType>
    PointerRange(PointerType ptr, SizeType size)
            : begin(ptr), end(ptr ? ptr + size : nullptr) {}
    PointerType begin = nullptr;
    PointerType end = nullptr;
    bool operator<(const PointerRange &rhs) const {
        Compare cmp;
        return std::lexicographical_compare(begin, end, rhs.begin, rhs.end, cmp);
    }
};

template<typename PointerType, typename SizeType>
PointerRange(PointerType c, SizeType s) -> PointerRange<PointerType>;

}
