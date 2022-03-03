// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/core/AddressType.hpp"

#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>

#include "internal/otx/common/NymFile.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/ContactEnums.pb.h"
#include "serialization/protobuf/ContractEnums.pb.h"
#include "serialization/protobuf/PeerEnums.pb.h"
#include "util/Blank.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Notary;
class Nym;
class UnitDefinition;
}  // namespace identifier

class Identifier;
class PasswordPrompt;
class Secret;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
template <typename T>
struct make_blank;

template <>
struct make_blank<OTData> {
    static auto value(const api::Session&) -> OTData { return Data::Factory(); }
};
}  // namespace opentxs

namespace opentxs::internal
{
struct NymFile : virtual public opentxs::NymFile {
    virtual auto LoadSignedNymFile(const PasswordPrompt& reason) -> bool = 0;
    virtual auto SaveSignedNymFile(const PasswordPrompt& reason) -> bool = 0;
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
auto Secret(const std::size_t bytes) noexcept
    -> std::unique_ptr<opentxs::Secret>;
auto Secret(const ReadView bytes, const bool mode) noexcept
    -> std::unique_ptr<opentxs::Secret>;
}  // namespace opentxs::factory

namespace opentxs
{
auto translate(const AddressType in) noexcept -> proto::AddressType;
auto translate(const proto::AddressType in) noexcept -> AddressType;
}  // namespace opentxs
