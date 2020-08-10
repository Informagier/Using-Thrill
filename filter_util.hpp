#pragma once

#include <vector>

#include "thrill/api/dia.hpp"

template <typename ValueType>
using FilterPair = std::pair<ValueType, bool>;

template <typename ValueType, typename Stack>
auto FilterBySecond(const thrill::api::DIA<FilterPair<ValueType>, Stack> &dia)
{
    using Pair = FilterPair<ValueType>;
    return dia.template FlatMap<ValueType>(
        [](const Pair &elemPair, auto emit) {
            if (elemPair.second)
            {
                emit(elemPair.first);
            }
        });
}