#pragma once
#include <vector>

namespace ewd
{
    template <typename T>
    using Matrix = std::vector<std::vector<T>>;

    using vecDouble = std::vector<double>;
    using vecIndex = std::vector<size_t>;
    using vecInt = std::vector<int>;

    using matDouble = Matrix<double>;
    using matIndex = Matrix<size_t>;
    using matInt = Matrix<int>;
}