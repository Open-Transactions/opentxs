// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace crypto
{
class Envelope;
}  // namespace crypto

namespace proto
{
class Envelope;
}  // namespace proto

class Armored;
class PasswordPrompt;

using OTEnvelope = Pimpl<crypto::Envelope>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto
{
class OPENTXS_EXPORT Envelope
{
public:
    using Recipients = UnallocatedSet<Nym_p>;
    using SerializedType = proto::Envelope;

    virtual auto Armored(opentxs::Armored& ciphertext) const noexcept
        -> bool = 0;
    virtual auto Open(
        const identity::Nym& recipient,
        const AllocateOutput plaintext,
        const PasswordPrompt& reason) const noexcept -> bool = 0;
    virtual auto Serialize(AllocateOutput destination) const noexcept
        -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto Serialize(
        SerializedType& serialized) const noexcept -> bool = 0;

    virtual auto Seal(
        const Recipients& recipients,
        const ReadView plaintext,
        const PasswordPrompt& reason) noexcept -> bool = 0;
    virtual auto Seal(
        const identity::Nym& theRecipient,
        const ReadView plaintext,
        const PasswordPrompt& reason) noexcept -> bool = 0;

    Envelope(const Envelope&) = delete;
    Envelope(Envelope&&) = delete;
    auto operator=(const Envelope&) -> Envelope& = delete;
    auto operator=(Envelope&&) -> Envelope& = delete;

    virtual ~Envelope() = default;

protected:
    Envelope() = default;

private:
    virtual auto clone() const noexcept -> Envelope* = 0;
};
}  // namespace opentxs::crypto
