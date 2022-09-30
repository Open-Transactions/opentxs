// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <functional>
#include <string_view>
#include <typeindex>
#include <variant>

#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace database
{
namespace wallet
{
namespace db
{
struct Pattern;
}  // namespace db
}  // namespace wallet
}  // namespace database
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct hash<opentxs::blockchain::database::wallet::db::Pattern> {
    auto operator()(const opentxs::blockchain::database::wallet::db::Pattern&
                        data) const noexcept -> std::size_t;
};
}  // namespace std

namespace opentxs::blockchain::database::wallet::db
{
auto operator==(const Pattern& lhs, const Pattern& rhs) noexcept -> bool;

struct Pattern {
    const Space data_;

    auto Data() const noexcept -> ReadView;
    auto Index() const noexcept -> Bip32Index;

    Pattern(const Bip32Index index, const ReadView data) noexcept;
    Pattern(const ReadView bytes) noexcept(false);
    Pattern() = delete;
    Pattern(const Pattern&) = delete;
    Pattern(Pattern&& rhs) noexcept;
    auto operator=(const Pattern&) -> Pattern& = delete;
    auto operator=(Pattern&&) -> Pattern& = delete;

    ~Pattern() = default;

private:
    static constexpr auto fixed_ = sizeof(Bip32Index);
};
}  // namespace opentxs::blockchain::database::wallet::db
