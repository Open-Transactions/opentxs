// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/crypto/subaccount/deterministic/Imp.hpp"  // IWYU pragma: associated

#include <HDPath.pb.h>
#include <boost/container/vector.hpp>
#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>

#include "blockchain/crypto/element/Element.hpp"
#include "blockchain/crypto/subaccount/base/Imp.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/crypto/Subaccount.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"
#include "opentxs/core/Amount.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/asymmetric/Role.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/PasswordPrompt.hpp"

namespace opentxs::blockchain::crypto
{
using namespace std::literals;

DeterministicPrivate::DeterministicPrivate(
    const api::Session& api,
    const crypto::Account& parent,
    const SubaccountType type,
    const identifier::Account& id,
    identifier::Generic source,
    std::string_view sourceName,
    std::string_view name,
    proto::HDPath path,
    ChainData&& data) noexcept
    : SubaccountPrivate(
          api,
          parent,
          type,
          id,
          std::move(source),
          sourceName,
          name)
    , path_(std::move(path))
    , seed_id_(api_.Factory().Internal().SeedID(path_.seed()))
    , data_(std::move(data))
    , generated_({{data_.internal_.type_, 0}, {data_.external_.type_, 0}})
    , used_({{data_.internal_.type_, 0}, {data_.external_.type_, 0}})
    , last_allocation_()
    , cached_key_()
{
}

DeterministicPrivate::DeterministicPrivate(
    const api::Session& api,
    const crypto::Account& parent,
    const SubaccountType type,
    const identifier::Account& id,
    const SerializedType& serialized,
    identifier::Generic source,
    std::string_view sourceName,
    std::string_view name,
    Bip32Index internal,
    Bip32Index external,
    ChainData&& data) noexcept(false)
    : SubaccountPrivate(
          api,
          parent,
          type,
          id,
          std::move(source),
          sourceName,
          name,
          serialized.common())
    , path_(serialized.path())
    , seed_id_(api_.Factory().Internal().SeedID(path_.seed()))
    , data_(std::move(data))
    , generated_(
          {{data_.internal_.type_, internal},
           {data_.external_.type_, external}})
    , used_(
          {{data_.internal_.type_, serialized.internalindex()},
           {data_.external_.type_, serialized.externalindex()}})
    , last_allocation_()
    , cached_key_()
{
}

DeterministicPrivate::ChainData::ChainData(
    const api::Session& api,
    Subchain internalType,
    bool internalContact,
    Subchain externalType,
    bool externalContact) noexcept
    : internal_(api, internalType, internalContact)
    , external_(api, externalType, externalContact)
{
}

auto DeterministicPrivate::ChainData::Get(Subchain type) noexcept(false)
    -> AddressData&
{
    if (type == internal_.type_) {

        return internal_;
    } else if (type == external_.type_) {

        return external_;
    } else {

        throw std::out_of_range("Invalid subchain");
    }
}

auto DeterministicPrivate::accept(
    const rLock& lock,
    const Subchain type,
    const identifier::Generic& contact,
    const std::string_view label,
    const Time time,
    const Bip32Index index,
    Batch& generated,
    const PasswordPrompt& reason) const noexcept -> std::optional<Bip32Index>
{
    check_lookahead(lock, type, generated, reason);
    set_metadata(lock, type, index, contact, label);
    auto& element =
        const_cast<DeterministicPrivate&>(*this).element(lock, type, index);
    element.Internal().Reserve(time);
    LogTrace()()("Accepted index ")(index).Flush();

    return index;
}

auto DeterministicPrivate::AllowedSubchains() const noexcept -> Set<Subchain>
{
    return {data_.internal_.type_, data_.external_.type_};
}

auto DeterministicPrivate::BalanceElement(
    const Subchain type,
    const Bip32Index index) const noexcept(false) -> const crypto::Element&
{
    auto lock = rLock{lock_};

    return element(lock, type, index);
}

auto DeterministicPrivate::check(
    const rLock& lock,
    const Subchain type,
    const identifier::Generic& contact,
    const std::string_view label,
    const Time time,
    const Bip32Index candidate,
    const PasswordPrompt& reason,
    Fallback& fallback,
    std::size_t& gap,
    Batch& generated) const noexcept(false) -> std::optional<Bip32Index>
{
    const auto accept = [&](Bip32Index index) {
        return this->accept(
            lock, type, contact, label, time, index, generated, reason);
    };

    if (is_generated(lock, type, candidate)) {
        LogTrace()()("Examining generated index ")(candidate).Flush();
        const auto& element = this->element(lock, type, candidate);
        const auto status = element.Internal().IsAvailable(contact, label);

        switch (status) {
            case Status::NeverUsed: {
                LogTrace()()("index ")(candidate)(" was never used").Flush();

                return accept(candidate);
            }
            case Status::Reissue: {
                LogTrace()()("Recycling unused index ")(candidate).Flush();

                return accept(candidate);
            }
            case Status::Used: {
                LogTrace()()("index ")(candidate)(" has confirmed transactions")
                    .Flush();
                gap = 0;

                throw std::runtime_error("Not acceptable");
            }
            case Status::MetadataConflict: {
                LogTrace()()("index ")(candidate)(" can not be used").Flush();
                ++gap;

                throw std::runtime_error("Not acceptable");
            }
            case Status::Reserved: {
                LogTrace()()("index ")(candidate)(" is reserved").Flush();
                ++gap;

                throw std::runtime_error("Not acceptable");
            }
            case Status::StaleUnconfirmed:
            default: {
                LogTrace()()("saving index ")(candidate)(" as a fallback")
                    .Flush();
                fallback[status].emplace(candidate);
                ++gap;

                throw std::runtime_error("Not acceptable");
            }
        }
    } else {
        LogTrace()()("Generating index ")(candidate).Flush();
        const auto newIndex = generate(lock, type, candidate, reason);

        assert_true(newIndex == candidate);

        generated.emplace_back(newIndex);

        return accept(candidate);
    }
}

void DeterministicPrivate::check_lookahead(
    const rLock& lock,
    Batch& internal,
    Batch& external,
    const PasswordPrompt& reason) const noexcept(false)
{
    check_lookahead(lock, data_.internal_.type_, internal, reason);
    check_lookahead(lock, data_.external_.type_, external, reason);
}

auto DeterministicPrivate::check_lookahead(
    const rLock& lock,
    const Subchain type,
    Batch& generated,
    const PasswordPrompt& reason) const noexcept(false) -> void
{
    auto needed = need_lookahead(lock, type);

    while (0u < needed) {
        generated.emplace_back(generate_next(lock, type, reason));
        --needed;
    }
}

auto DeterministicPrivate::confirm(
    const rLock& lock,
    const Subchain type,
    const Bip32Index index) noexcept -> void
{
    const auto checkUsed = [&] {
        try {
            auto& used = used_.at(type);

            if (index < used) { return; }

            static const auto blank = identifier::Generic{};

            for (auto i{index}; i > used; --i) {
                const auto& element = this->element(lock, type, i - 1u);

                if (Status::Used != element.Internal().IsAvailable(blank, "")) {
                    return;
                }
            }

            for (auto i{index}; i < generated_.at(type); ++i) {
                const auto& element = this->element(lock, type, i);

                if (Status::Used == element.Internal().IsAvailable(blank, "")) {
                    used = std::max(used, i + 1u);
                } else {

                    return;
                }
            }
        } catch (...) {
            LogError()()("invalid subchain or index").Flush();
        }
    };
    checkUsed();
    auto generated = Batch{};
    const auto reason = api_.Factory().PasswordPrompt(
        "Generate account keys for wallet scanning.");
    check_lookahead(lock, type, generated, reason);
}

auto DeterministicPrivate::element(
    const rLock&,
    const Subchain type,
    const Bip32Index index) noexcept(false) -> crypto::Element&
{
    const auto validType =
        (type == data_.internal_.type_) || (type == data_.external_.type_);

    if (false == validType) {
        const auto error = CString{print(type)}
                               .append(" subchain is not valid for ")
                               .append(Describe());

        throw std::out_of_range(error.c_str());
    }

    try {
        if (data_.internal_.type_ == type) {
            auto& data = data_.internal_;

            return *data.map_.at(index);
        } else {
            auto& data = data_.external_;

            return *data.map_.at(index);
        }
    } catch (...) {
        const auto error = CString{"index "}
                               .append(std::to_string(index))
                               .append(" has not been generated on ")
                               .append(Describe())
                               .append(" ")
                               .append(print(type))
                               .append(" subchain");

        throw std::out_of_range(error.c_str());
    }
}

auto DeterministicPrivate::finish_allocation(
    const rLock& lock,
    const Subchain subchain,
    const Batch& generated) const noexcept -> bool
{
    static const auto null = Batch{};

    if (subchain == data_.internal_.type_) {

        return finish_allocation(lock, generated, null);
    } else if (subchain == data_.external_.type_) {

        return finish_allocation(lock, null, generated);
    } else {
        LogAbort()().Abort();
    }
}

auto DeterministicPrivate::finish_allocation(
    const rLock& lock,
    const Batch& internal,
    const Batch& external) const noexcept -> bool
{
    if (0u < internal.size()) {
        parent_.Parent().Parent().Internal().KeyGenerated(
            target_, parent_.NymID(), id_, type_, data_.internal_.type_);
    }

    if (0u < external.size()) {
        parent_.Parent().Parent().Internal().KeyGenerated(
            target_, parent_.NymID(), id_, type_, data_.external_.type_);
    }

    return save(lock);
}

auto DeterministicPrivate::Floor(const Subchain type) const noexcept
    -> std::optional<Bip32Index>
{
    auto lock = rLock{lock_};

    try {

        return used_.at(type);
    } catch (...) {

        return std::nullopt;
    }
}

auto DeterministicPrivate::GenerateNext(
    const Subchain type,
    const PasswordPrompt& reason) const noexcept -> std::optional<Bip32Index>
{
    auto lock = rLock{lock_};

    if (0 == generated_.count(type)) { return {}; }

    try {
        auto generated = Batch{};
        generated.emplace_back(generate_next(lock, type, reason));

        assert_true(0u < generated.size());

        if (finish_allocation(lock, type, generated)) {

            return generated.front();
        } else {

            return std::nullopt;
        }
    } catch (...) {

        return std::nullopt;
    }
}

auto DeterministicPrivate::generate(
    const rLock& lock,
    const Subchain type,
    const Bip32Index desired,
    const PasswordPrompt& reason) const noexcept(false) -> Bip32Index
{
    auto& addressMap = data_.Get(type).map_;
    auto& index = generated_.at(type);

    assert_true(addressMap.size() == index);
    assert_true(desired == index);

    if (max_index_ <= index) { throw std::runtime_error("Account is full"); }

    const auto& key = PrivateKey(type, index, reason);

    if (false == key.IsValid()) {
        throw std::runtime_error("Failed to generate key");
    }

    const auto& blockchain = parent_.Parent().Parent();
    const auto [it, added] = addressMap.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(index),
        std::forward_as_tuple(std::make_unique<implementation::Element>(
            api_,
            blockchain,
            Self(),
            base_chain(target_),
            type,
            index,
            key,
            get_contact())));

    if (false == added) { throw std::runtime_error("Failed to add key"); }

    return index++;
}

auto DeterministicPrivate::generate_next(
    const rLock& lock,
    const Subchain type,
    const PasswordPrompt& reason) const noexcept(false) -> Bip32Index
{
    return generate(lock, type, generated_.at(type), reason);
}

auto DeterministicPrivate::get_contact() const noexcept -> identifier::Generic
{
    static const auto blank = identifier::Generic{};

    return blank;
}

auto DeterministicPrivate::init(bool existing) noexcept(false) -> void
{
    const auto& log = LogTrace();
    const auto cb = [&](const auto& data) {
        const auto type = data.type_;
        const auto nextGenerateIndex = generated_.at(type);
        const auto generatedCount = data.map_.size();
        const auto contiguous = data.check_keys();
        log()("verifying consistency of ")(Describe())(" ")(print(type))(
            " subchain")
            .Flush();
        log()("generate index: ")(nextGenerateIndex).Flush();
        log()("    used index: ")(used_.at(type)).Flush();
        log()("     key count: ")(generatedCount).Flush();

        assert_true(contiguous);

        if (generatedCount != nextGenerateIndex) {
            LogError()()("next generate index for ")(Describe())(" ")(
                print(type))(" subchain is ")(nextGenerateIndex)(" however ")(
                generatedCount)(" keys have been generated")
                .Flush();

            return true;
        }

        return false;
    };
    auto inconsistent = cb(data_.internal_);
    inconsistent |= cb(data_.external_);

    if (inconsistent) {
        LogError()()("repairing inconsistent state for ")(Describe()).Flush();
        const auto reason = api_.Factory().PasswordPrompt(
            "Generate keys to repair inconsistent blockchain account");
        init(reason);
    } else {
        SubaccountPrivate::init(existing);
    }
}

auto DeterministicPrivate::init(const PasswordPrompt& reason) noexcept(false)
    -> void
{
    {
        auto lock = rLock{lock_};

        if (account_already_exists(lock)) {
            throw std::runtime_error(
                "attempting to initialize new subaccount "s
                    .append(id_.asBase58(api_.Crypto()))
                    .append(" which has already been created"));
        }

        auto internal = Batch{};
        auto external = Batch{};
        check_lookahead(lock, internal, external, reason);

        if (false == finish_allocation(lock, internal, external)) {
            throw std::runtime_error("Failed to save new account");
        }
    }

    SubaccountPrivate::init(false);
}

auto DeterministicPrivate::Key(const Subchain type, const Bip32Index index)
    const noexcept -> const opentxs::crypto::asymmetric::key::EllipticCurve&
{
    try {

        return data_.Get(type).map_.at(index)->Key();
    } catch (...) {

        return opentxs::crypto::asymmetric::key::EllipticCurve::Blank();
    }
}

auto DeterministicPrivate::LastGenerated(const Subchain type) const noexcept
    -> std::optional<Bip32Index>
{
    auto lock = rLock{lock_};

    try {
        auto output = generated_.at(type);

        return (0 == output) ? std::optional<Bip32Index>{} : output - 1;
    } catch (...) {

        return {};
    }
}

auto DeterministicPrivate::mutable_element(
    const rLock& lock,
    const Subchain type,
    const Bip32Index index) noexcept(false) -> crypto::Element&
{
    auto& data = data_.Get(type).map_;

    try {

        return *data.at(index);
    } catch (...) {
        auto error = CString{"index "}
                         .append(std::to_string(index))
                         .append(" does not exist for ")
                         .append(print(type))
                         .append(" subchain which contains ");

        if (0u == data.size()) {
            error.append("no elements");
        } else {
            error.append("elements ")
                .append(std::to_string(data.cbegin()->first))
                .append(" through ")
                .append(std::to_string(data.crbegin()->first));
        }

        throw std::out_of_range{error.c_str()};
    }
}

auto DeterministicPrivate::need_lookahead(
    const rLock& lock,
    const Subchain type) const noexcept -> Bip32Index
{
    const auto capacity = (generated_.at(type) - used_.at(type));

    if (capacity >= window_) { return 0u; }

    const auto effective = [&] {
        auto& last = last_allocation_[type];

        if (false == last.has_value()) {
            last = window_;

            return window_;
        } else {
            last = std::min(max_allocation_, last.value() * 2u);

            return last.value();
        }
    }();

    return effective - capacity;
}

auto DeterministicPrivate::PathRoot() const noexcept
    -> const opentxs::crypto::SeedID&
{
    return seed_id_;
}

auto DeterministicPrivate::PrivateKey(
    const implementation::Element& element,
    const Subchain type,
    const Bip32Index index,
    const PasswordPrompt& reason) const noexcept
    -> const opentxs::crypto::asymmetric::key::EllipticCurve&
{
    auto key = PrivateKey(type, index, reason);

    if (false == key.IsValid()) {
        LogError()()("error deriving private key").Flush();

        return Subaccount::PrivateKey(element, type, index, reason);
    }

    if (false == key.HasPrivate()) {
        LogError()()("deriving private key is not valid").Flush();

        return Subaccount::PrivateKey(element, type, index, reason);
    }

    auto handle = element.data_.lock();
    auto& data = *handle;

    if (false == data.private_key_.has_value()) {
        data.private_key_.emplace(std::move(key));
    }

    return *data.private_key_;
}

auto DeterministicPrivate::Reserve(
    const Subchain type,
    const PasswordPrompt& reason,
    const std::string_view label,
    const Time time) const noexcept -> std::optional<Bip32Index>
{
    return Reserve(type, identifier::Generic{}, reason, label, time);
}

auto DeterministicPrivate::Reserve(
    const Subchain type,
    const identifier::Generic& contact,
    const PasswordPrompt& reason,
    const std::string_view label,
    const Time time) const noexcept -> std::optional<Bip32Index>
{
    auto batch = Reserve(type, 1_uz, contact, reason, label, time);

    if (batch.empty()) { return std::nullopt; }

    return batch.front();
}

auto DeterministicPrivate::Reserve(
    const Subchain type,
    const std::size_t batch,
    const PasswordPrompt& reason,
    const std::string_view label = {},
    const Time time = Clock::now()) const noexcept -> Batch
{
    return Reserve(type, batch, identifier::Generic{}, reason, label, time);
}

auto DeterministicPrivate::Reserve(
    const Subchain type,
    const std::size_t batch,
    const identifier::Generic& contact,
    const PasswordPrompt& reason,
    const std::string_view label,
    const Time time) const noexcept -> Batch
{
    return reserve(type, batch, contact, reason, label, time);
}

auto DeterministicPrivate::reserve(
    const Subchain type,
    const std::size_t batch,
    const identifier::Generic& contact,
    const PasswordPrompt& reason,
    const std::string_view label,
    const Time time) const noexcept -> Batch
{
    auto output = Batch{};
    output.reserve(batch);
    auto lock = rLock{lock_};
    auto gen = Batch{};

    while (output.size() < batch) {
        auto out = use_next(lock, type, reason, contact, label, time, gen);

        if (false == out.has_value()) { break; }

        output.emplace_back(out.value());
    }

    if ((0u < output.size()) && (false == finish_allocation(lock, type, gen))) {

        return {};
    }

    return output;
}

auto DeterministicPrivate::RootNode(const PasswordPrompt& reason) const noexcept
    -> const opentxs::crypto::asymmetric::key::HD&
{
    auto& [mutex, key] = cached_key_;
    auto lock = Lock{mutex};

    if (key.IsValid()) { return key; }

    const auto fingerprint = api_.Factory().Internal().SeedID(path_.seed());
    auto path = UnallocatedVector<Bip32Index>{};

    for (const auto& child : path_.child()) { path.emplace_back(child); }

    key = api_.Crypto().Seed().GetHDKey(
        fingerprint,
        opentxs::crypto::EcdsaCurve::secp256k1,
        path,
        opentxs::crypto::asymmetric::Role::Sign,
        opentxs::crypto::asymmetric::key::EllipticCurve::MaxVersion(),
        reason);

    return key;
}

auto DeterministicPrivate::ScanProgress(Subchain type) const noexcept
    -> block::Position
{
    try {
        auto lock = rLock{lock_};

        return data_.Get(type).progress_;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return Subaccount::ScanProgress(type);
    }
}

auto DeterministicPrivate::serialize_deterministic(
    const rLock& lock,
    SerializedType& out) const noexcept -> void
{
    out.set_version(BlockchainDeterministicAccountDataVersion);
    serialize_common(lock, *out.mutable_common());
    *out.mutable_path() = path_;
    out.set_internalindex(used_.at(data_.internal_.type_));
    out.set_externalindex(used_.at(data_.external_.type_));
}

auto DeterministicPrivate::set_metadata(
    const rLock& lock,
    const Subchain subchain,
    const Bip32Index index,
    const identifier::Generic& contact,
    const std::string_view label) const noexcept -> void
{
    const auto blank = identifier::Generic{};

    try {
        auto& data = data_.Get(subchain);
        const auto& id = data.set_contact_ ? contact : blank;
        data.map_.at(index)->Internal().SetMetadata(id, label);
    } catch (...) {
    }
}

auto DeterministicPrivate::SetScanProgress(
    const block::Position& progress,
    Subchain type) noexcept -> void
{
    try {
        auto lock = rLock{lock_};

        data_.Get(type).progress_ = progress;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();
    }
}

auto DeterministicPrivate::unconfirm(
    const rLock& lock,
    const Subchain type,
    const Bip32Index index) noexcept -> void
{
    try {
        auto& used = used_.at(type);
        const auto& element = this->element(lock, type, index);

        if (0 == element.Confirmed().size()) { used = std::min(used, index); }
    } catch (...) {
    }
}

auto DeterministicPrivate::use_next(
    const rLock& lock,
    const Subchain type,
    const PasswordPrompt& reason,
    const identifier::Generic& contact,
    const std::string_view label,
    const Time time,
    Batch& generated) const noexcept -> std::optional<Bip32Index>
{
    try {
        auto gap = 0_uz;
        auto candidate = used_.at(type);
        auto fallback = Fallback{};

        while (gap < window_) {
            try {

                return check(
                    lock,
                    type,
                    contact,
                    label,
                    time,
                    candidate,
                    reason,
                    fallback,
                    gap,
                    generated);
            } catch (...) {
                ++candidate;

                continue;
            }
        }

        LogTrace()()("Gap limit reached. Searching for acceptable fallback")
            .Flush();
        const auto accept = [&](Bip32Index index) {
            return this->accept(
                lock, type, contact, label, time, index, generated, reason);
        };

        if (auto& set = fallback[Status::StaleUnconfirmed]; 0 < set.size()) {
            LogTrace()()(
                "Recycling index with old never-confirmed transactions")
                .Flush();

            return accept(*set.cbegin());
        }

        LogTrace()()(
            "No acceptable fallback discovered. Generating past the gap "
            "limit")
            .Flush();

        while (candidate < max_index_) {
            try {

                return check(
                    lock,
                    type,
                    contact,
                    label,
                    time,
                    candidate,
                    reason,
                    fallback,
                    gap,
                    generated);
            } catch (...) {
                ++candidate;

                continue;
            }
        }

        return {};
    } catch (...) {

        return {};
    }
}
}  // namespace opentxs::blockchain::crypto
