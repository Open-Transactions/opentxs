// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <span>

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace blockchain
{
namespace block
{
class Outpoint;
}  // namespace block

namespace protocol
{
namespace bitcoin
{
namespace base
{
namespace block
{
namespace internal
{
class Input;
}  // namespace internal

class InputPrivate;
class Script;
}  // namespace block
}  // namespace base
}  // namespace bitcoin
}  // namespace protocol
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::protocol::bitcoin::base::block
{
class OPENTXS_EXPORT Input : public opentxs::Allocated
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> Input&;

    operator bool() const noexcept { return IsValid(); }

    auto Coinbase() const noexcept -> ReadView;
    auto get_allocator() const noexcept -> allocator_type final;
    OPENTXS_NO_EXPORT auto Internal() const noexcept -> const internal::Input&;
    auto IsValid() const noexcept -> bool;
    auto Keys(allocator_type alloc) const noexcept -> Set<crypto::Key>;
    auto Keys(Set<crypto::Key>& out) const noexcept -> void;
    auto PreviousOutput() const noexcept -> const blockchain::block::Outpoint&;
    auto Print(const api::Crypto& crypto) const noexcept -> UnallocatedCString;
    auto Print(const api::Crypto& crypto, allocator_type alloc) const noexcept
        -> CString;
    auto Script() const noexcept -> const block::Script&;
    auto Sequence() const noexcept -> std::uint32_t;
    auto Witness() const noexcept -> std::span<const WitnessItem>;

    auto get_deleter() noexcept -> delete_function final;
    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Input&;
    auto swap(Input& rhs) noexcept -> void;

    OPENTXS_NO_EXPORT Input(InputPrivate* imp) noexcept;
    Input(allocator_type alloc = {}) noexcept;
    Input(const Input& rhs, allocator_type alloc = {}) noexcept;
    Input(Input&& rhs) noexcept;
    Input(Input&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Input& rhs) noexcept -> Input&;
    auto operator=(Input&& rhs) noexcept -> Input&;

    ~Input() override;

protected:
    friend InputPrivate;

    InputPrivate* imp_;
};
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block
