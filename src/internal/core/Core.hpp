// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/core/AddressType.hpp"

#pragma once

#include <ContactEnums.pb.h>
#include <ContractEnums.pb.h>
#include <PeerEnums.pb.h>
#include <cstddef>
#include <iosfwd>

#include "internal/otx/common/NymFile.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Generic;
class Notary;
class Nym;
class UnitDefinition;
}  // namespace identifier

class PasswordPrompt;
class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::internal
{
struct NymFile : virtual public opentxs::NymFile {
    virtual auto LoadSignedNymFile(const opentxs::PasswordPrompt& reason)
        -> bool = 0;
    virtual auto SaveSignedNymFile(const opentxs::PasswordPrompt& reason)
        -> bool = 0;
};
}  // namespace opentxs::internal

namespace opentxs::blockchain
{
auto AccountName(const Type chain) noexcept -> UnallocatedCString;
auto Chain(const api::Session& api, const identifier::Nym& id) noexcept -> Type;
auto Chain(const api::Session& api, const identifier::Notary& id) noexcept
    -> Type;
auto Chain(
    const api::Session& api,
    const identifier::UnitDefinition& id) noexcept -> Type;
auto IssuerID(const api::Session& api, const Type chain) noexcept
    -> const identifier::Nym&;
auto NotaryID(const api::Session& api, const Type chain) noexcept
    -> const identifier::Notary&;
auto UnitID(const api::Session& api, const Type chain) noexcept
    -> const identifier::UnitDefinition&;
}  // namespace opentxs::blockchain

namespace opentxs::factory
{
auto Secret(const std::size_t bytes) noexcept -> opentxs::Secret;
auto Secret(const ReadView bytes, const bool mode) noexcept -> opentxs::Secret;
}  // namespace opentxs::factory

namespace opentxs
{
auto check_subset(
    const std::size_t available,
    const std::size_t requested,
    const std::size_t position) noexcept -> bool;
auto translate(const AddressType in) noexcept -> proto::AddressType;
auto translate(const proto::AddressType in) noexcept -> AddressType;
}  // namespace opentxs
