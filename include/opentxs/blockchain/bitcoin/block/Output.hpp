// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/Amount.hpp"
// IWYU pragma: no_include "opentxs/core/Data.hpp"
// IWYU pragma: no_include "opentxs/core/identifier/Generic.hpp"
// IWYU pragma: no_include "opentxs/util/Writer.hpp"

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
namespace internal
{
class Output;
}  // namespace internal

class OutputPrivate;
class Script;
}  // namespace block
}  // namespace bitcoin

namespace token
{
namespace cashtoken
{
struct View;
}  // namespace cashtoken
}  // namespace token
}  // namespace blockchain

class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block
{
class OPENTXS_EXPORT Output : public opentxs::Allocated
{
public:
    using ContactID = identifier::Generic;

    OPENTXS_NO_EXPORT static auto Blank() noexcept -> Output&;

    operator bool() const noexcept { return IsValid(); }

    auto Cashtoken() const noexcept -> const token::cashtoken::View*;
    auto get_allocator() const noexcept -> allocator_type final;
    auto HasCashtoken() const noexcept -> bool
    {
        return nullptr != Cashtoken();
    }
    OPENTXS_NO_EXPORT auto Internal() const noexcept -> const internal::Output&;
    auto IsValid() const noexcept -> bool;
    auto Note(const api::crypto::Blockchain& crypto) const noexcept
        -> UnallocatedCString;
    auto Note(const api::crypto::Blockchain& crypto, allocator_type alloc)
        const noexcept -> CString;
    auto Keys(allocator_type alloc) const noexcept -> Set<crypto::Key>;
    auto Keys(Set<crypto::Key>& out) const noexcept -> void;
    auto Payee() const noexcept -> ContactID;
    auto Payer() const noexcept -> ContactID;
    auto Print() const noexcept -> UnallocatedCString;
    auto Print(allocator_type alloc) const noexcept -> CString;
    auto Script() const noexcept -> const block::Script&;
    auto Value() const noexcept -> Amount;

    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Output&;
    auto swap(Output& rhs) noexcept -> void;

    OPENTXS_NO_EXPORT Output(OutputPrivate* imp) noexcept;
    Output(allocator_type alloc = {}) noexcept;
    Output(const Output& rhs, allocator_type alloc = {}) noexcept;
    Output(Output&& rhs) noexcept;
    Output(Output&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Output& rhs) noexcept -> Output&;
    auto operator=(Output&& rhs) noexcept -> Output&;

    ~Output() override;

protected:
    friend OutputPrivate;

    OutputPrivate* imp_;
};
}  // namespace opentxs::blockchain::bitcoin::block
