#pragma once

#include <utility>

#include "thrill/common/math.hpp"
#include "thrill/api/dia.hpp"
#include "thrill/api/zip_with_index.hpp"

#include "filter_util.hpp"

template <typename ValueType, typename Stack>
auto Head(const thrill::api::DIA<ValueType, Stack> &dia)
{
    using Pair = FilterPair<ValueType>;
    auto filterable = dia.ZipWithIndex(
        [](const ValueType &elem, const size_t &index) {
            return Pair{elem, index == 0};
        });
    return FilterBySecond(filterable);
}

template <typename ValueType, typename Stack>
auto Tail(const thrill::api::DIA<ValueType, Stack> &dia)
{
    using Pair = FilterPair<ValueType>;
    auto filterable = dia.ZipWithIndex(
        [](const ValueType &elem, const size_t &index) {
            return Pair{elem, index != 0};
        });
    return FilterBySecond(filterable);
}
