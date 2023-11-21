// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace crypto
{
namespace internal
{
class Subaccount;
}  // namespace internal

class Ethereum;
}  // namespace crypto
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto
{
class OPENTXS_EXPORT Imported : public Subaccount
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> Imported&;

    auto asEthereum() const noexcept -> const crypto::Ethereum&;

    auto asEthereum() noexcept -> crypto::Ethereum&;

    OPENTXS_NO_EXPORT Imported(
        std::shared_ptr<internal::Subaccount> imp) noexcept;
    Imported() = delete;
    Imported(const Imported& rhs) noexcept;
    Imported(Imported&& rhs) noexcept;
    auto operator=(const Imported&) -> Imported& = delete;
    auto operator=(Imported&&) -> Imported& = delete;

    ~Imported() override;
};
}  // namespace opentxs::blockchain::crypto
