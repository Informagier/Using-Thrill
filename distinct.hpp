#pragma once

#include <vector>
#include <utility>
#include <iterator>
#include "thrill/common/math.hpp"
#include "thrill/api/dia.hpp"
#include "thrill/api/window.hpp"
#include "thrill/api/sort.hpp"
#include "thrill/api/union.hpp"
#include "thrill/api/concat.hpp"
#include "thrill/api/zip.hpp"
#include "range_util.hpp"
#include "thrill/api/group_by_key.hpp"
#include "thrill/api/rebalance.hpp"
#include "thrill/api/collapse.hpp"

template <typename ValueType, typename Stack, typename KeyExtractor>
auto Distinct(
    thrill::api::DIA<ValueType, Stack> &dia, const KeyExtractor &keyFun)
{
    auto sorted = dia.Sort([&keyFun](const ValueType &a, const ValueType &b) -> bool { return keyFun(a) < keyFun(b); });

    auto ret = sorted.template FlatWindow<ValueType>(
        2,
        [&keyFun](size_t, auto &values, auto emit) {
            if (keyFun(values[0]) != keyFun(values[1]))
                emit(values[0]);
        },
        [](size_t, auto &values, auto emit) { emit(values[0]); }).Rebalance();
    return ret;

    // auto head = Head(sorted);
    // auto tail = Tail(sorted);
    // auto cleaned = sorted.Zip(
    //     thrill::api::CutTag,
    //     tail,
    //     [](const ValueType &first, const ValueType &second) -> FilterPair<ValueType> { return FilterPair<ValueType>{first, first != second}; });
    // auto filtered = FilterBySecond(cleaned);
    // return thrill::api::Union(head, filtered);

    // return sorted.ReduceByKey(
    //           [](const ValueType &key) -> ValueType { return key; },
    //           [](const ValueType &val, const ValueType &) -> ValueType { return val; });

    // return dia.template GroupByKey<ValueType>(
    //     thrill::api::NoLocationDetectionTag,
    //     [](const ValueType val) -> ValueType { return val; },
    //     [](auto &iter, const ValueType) -> ValueType { return iter.Next(); });
}

template <typename ValueType, typename Stack>
auto Distinct(
    thrill::api::DIA<ValueType, Stack> &dia)
{
    return Distinct(dia, [](const ValueType &v) -> ValueType { return v; });
}