#pragma once

namespace eds
{
    ///<summary></summary>
    struct in_place_t { };
    static constexpr in_place_t in_place = {};

    template <typename T>
    struct in_place_type_t { };

    template <size_t I>
    struct in_place_index_t { };
}