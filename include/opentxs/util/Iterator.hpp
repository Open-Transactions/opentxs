// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <iterator>
#include <limits>
#include <stdexcept>

namespace opentxs::iterator
{
template <typename P, typename C>
class OPENTXS_EXPORT Bidirectional
{
public:
    using value_type = C;
    using pointer = value_type*;
    using reference = value_type&;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

    Bidirectional(P* parent, std::size_t position = 0)
        : position_{position}
        , parent_{parent}
    {
    }
    Bidirectional() = default;
    Bidirectional(const Bidirectional&) = default;
    Bidirectional(Bidirectional&&) = default;
    auto operator=(const Bidirectional&) -> Bidirectional& = default;
    auto operator=(Bidirectional&&) -> Bidirectional& = default;

    auto operator*() -> reference
    {
        if (nullptr == parent_) { throw std::out_of_range{""}; }

        return parent_->at(position_);
    }

    auto operator++() -> Bidirectional&
    {
        if (std::numeric_limits<std::size_t>::max() == position_) {
            throw std::out_of_range{""};
        }

        ++position_;

        return *this;
    }
    auto operator++(int) -> Bidirectional
    {
        Bidirectional output{*this};
        ++(*this);

        return output;
    }
    auto operator--() -> Bidirectional&
    {
        if (std::numeric_limits<std::size_t>::min() == position_) {
            throw std::out_of_range{""};
        }

        --position_;

        return *this;
    }
    auto operator--(int) -> Bidirectional
    {
        const auto output{Bidirectional(*this)};
        --(*this);

        return output;
    }
    auto operator==(const Bidirectional& rhs) const -> bool
    {
        return (parent_ == rhs.parent_) && (position_ == rhs.position_);
    }
    auto operator!=(const Bidirectional& rhs) const -> bool
    {
        return !(*this == rhs);
    }

    ~Bidirectional() = default;

private:
    std::size_t position_{0};
    P* parent_{nullptr};
};
}  // namespace opentxs::iterator
