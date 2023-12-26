// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/Signable.hpp"  // IWYU pragma: associated

#include <memory>
#include <stdexcept>
#include <utility>

#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::contract::implementation
{
using namespace std::literals;

template <typename IDType>
Signable<IDType>::Signable(
    const api::Session& api,
    const Nym_p& nym,
    const VersionNumber version,
    std::string_view conditions,
    std::string_view alias,
    Signatures&& signatures) noexcept
    : api_(api)
    , signer_(nym)
    , version_(version)
    , conditions_(conditions)
    , alias_(alias)
    , signatures_()
    , set_once_()
{
    if (false == signatures.empty()) {
        signatures_.set_value(std::move(signatures));
    }
}

template <typename IDType>
Signable<IDType>::Signable(
    const api::Session& api,
    const Nym_p& nym,
    const VersionNumber version,
    std::string_view conditions,
    std::string_view alias) noexcept
    : Signable(api, nym, version, conditions, alias, Signatures{})
{
}

template <typename IDType>
Signable<IDType>::Signable(
    const api::Session& api,
    const Nym_p& nym,
    const VersionNumber version,
    std::string_view conditions,
    std::string_view alias,
    std::string_view name,
    IDType id,
    Signatures&& signatures) noexcept
    : Signable(api, nym, version, conditions, alias, std::move(signatures))
{
    set_once_.set_value(Calculated{std::move(id), CString{name}});
}

template <typename IDType>
Signable<IDType>::Signable(
    const api::Session& api,
    const Nym_p& nym,
    const VersionNumber version,
    std::string_view conditions,
    std::string_view alias,
    IDType id,
    Signatures&& signatures) noexcept
    : Signable(
          api,
          nym,
          version,
          conditions,
          alias,
          id.asBase58(api.Crypto()),
          id,
          std::move(signatures))
{
}

template <typename IDType>
Signable<IDType>::Signable(const Signable& rhs) noexcept
    : api_(rhs.api_)
    , signer_(rhs.signer_)
    , version_(rhs.version_)
    , conditions_(rhs.conditions_)
    , alias_(*rhs.alias_.lock_shared())
    , signatures_(rhs.signatures_)
    , set_once_(rhs.set_once_)
{
}

template <typename IDType>
auto Signable<IDType>::Alias() const noexcept -> UnallocatedCString
{
    return alias_.lock_shared()->c_str();
}

template <typename IDType>
auto Signable<IDType>::Alias(alloc::Strategy alloc) const noexcept -> CString
{
    return {*alias_.lock_shared(), alloc.result_};
}

template <typename IDType>
auto Signable<IDType>::add_signatures(Signatures signatures) noexcept -> void
{
    assert_false(signatures_.ready());

    signatures_.set_value(std::move(signatures));
}

template <typename IDType>
auto Signable<IDType>::check_id() const noexcept -> bool
{
    return calculate_id() == ID();
}

template <typename IDType>
auto Signable<IDType>::first_time_init() noexcept(false) -> void
{
    first_time_init([](const auto&) { return CString{}; });
}

template <typename IDType>
auto Signable<IDType>::first_time_init(SetNameFromID_t) noexcept(false) -> void
{
    first_time_init(
        [this](const auto& id) { return id.asBase58(api_.Crypto(), {}); });
}

template <typename IDType>
auto Signable<IDType>::first_time_init(GetName cb) noexcept(false) -> void
{
    auto id = [&] {
        auto out = calculate_id();

        if (out.empty()) { throw std::runtime_error("Failed to calculate id"); }

        return out;
    }();
    auto name = [&] {
        if (false == cb.operator bool()) {
            throw std::runtime_error("invalid name callback");
        }

        return std::invoke(cb, id);
    }();

    try {
        set_once_.set_value(Calculated{std::move(id), std::move(name)});
    } catch (const std::exception& e) {
        LogAbort()()(e.what()).Abort();
    }
}

template <typename IDType>
auto Signable<IDType>::ID() const noexcept -> const IDType&
{
    assert_true(set_once_.ready());

    return set_once_.get().id_;
}

template <typename IDType>
auto Signable<IDType>::init_serialized() noexcept(false) -> void
{
    const auto& required = ID();
    const auto calculated = calculate_id();

    if (required != calculated) {

        throw std::runtime_error(
            "Calculated id ("s + calculated.asBase58(api_.Crypto()) +
            ") does not match serialized id (" +
            required.asBase58(api_.Crypto()) + ")");
    }
}

template <typename IDType>
auto Signable<IDType>::Name() const noexcept -> std::string_view
{
    assert_true(set_once_.ready());

    return set_once_.get().name_;
}

template <typename IDType>
auto Signable<IDType>::serialize(const ProtobufType& in, Writer&& out)
    const noexcept -> bool
{
    return proto::write(in, std::move(out));
}

template <typename IDType>
auto Signable<IDType>::SetAlias(std::string_view alias) noexcept -> bool
{
    alias_.lock()->assign(alias);

    return true;
}

template <typename IDType>
auto Signable<IDType>::signatures() const noexcept -> std::span<const Signature>
{
    assert_true(signatures_.ready());

    return signatures_.get();
}

template <typename IDType>
auto Signable<IDType>::Signer() const noexcept -> Nym_p
{
    return signer_;
}

template <typename IDType>
auto Signable<IDType>::Terms() const noexcept -> std::string_view
{
    return conditions_;
}

template <typename IDType>
auto Signable<IDType>::update_signature(const PasswordPrompt& reason) -> bool
{
    if (!Signer()) {
        LogError()()("Missing nym.").Flush();

        return false;
    }

    return true;
}

template <typename IDType>
auto Signable<IDType>::Validate() const noexcept -> bool
{
    return validate();
}

template <typename IDType>
auto Signable<IDType>::verify_signature(const proto::Signature&) const -> bool
{
    if (!Signer()) {
        LogError()()("Missing nym.").Flush();

        return false;
    }

    return true;
}

template <typename IDType>
auto Signable<IDType>::Version() const noexcept -> VersionNumber
{
    return version_;
}
}  // namespace opentxs::contract::implementation

namespace opentxs::contract::implementation
{
template class Signable<identifier::Generic>;
template class Signable<identifier::Notary>;
template class Signable<identifier::Nym>;
template class Signable<identifier::UnitDefinition>;
}  // namespace opentxs::contract::implementation
