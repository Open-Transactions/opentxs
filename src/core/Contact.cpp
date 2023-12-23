// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/unordered/detail/foa.hpp>
// IWYU pragma: no_include <boost/unordered/detail/foa/flat_map_types.hpp>
// IWYU pragma: no_include <boost/unordered/detail/foa/table.hpp>

#include "opentxs/core/Contact.hpp"  // IWYU pragma: associated

#include <Contact.pb.h>
#include <ContactItem.pb.h>
#include <boost/container/flat_set.hpp>
#include <boost/unordered/unordered_flat_map.hpp>
#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <span>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "internal/api/crypto/Encode.hpp"
#include "internal/core/String.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/ContactItem.hpp"
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/AddressStyle.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/wot/Claim.hpp"
#include "opentxs/identity/wot/claim/Attribute.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/ClaimType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/Data.hpp"
#include "opentxs/identity/wot/claim/Group.hpp"
#include "opentxs/identity/wot/claim/Item.hpp"
#include "opentxs/identity/wot/claim/Item.internal.hpp"
#include "opentxs/identity/wot/claim/Section.hpp"      // IWYU pragma: keep
#include "opentxs/identity/wot/claim/SectionType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/identity/wot/claim/Types.internal.hpp"
#include "opentxs/identity/wot/claim/internal.factory.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"

#define ID_BYTES 32

namespace opentxs
{
constexpr auto OT_CONTACT_VERSION = 3;
}  // namespace opentxs

namespace opentxs
{
using namespace std::literals;

static constexpr auto style_to_string_ = [] {
    using enum blockchain::crypto::AddressStyle;

    return frozen::
        make_unordered_map<blockchain::crypto::AddressStyle, std::string_view>({
            {p2pkh, "1"sv},
            {p2sh, "2"sv},
            {p2wpkh, "3"sv},
            {p2wsh, "4"sv},
            {p2tr, "5"sv},
            {ethereum_account, "6"sv},
        });
}();
static const auto string_to_style_ = [] {
    auto out = boost::unordered_flat_map<
        std::string_view,
        blockchain::crypto::AddressStyle>{};
    out.reserve(style_to_string_.size());

    for (const auto& [key, value] : style_to_string_) {
        out.try_emplace(value, key);
    }

    return out;
}();

static auto translate_style(std::string_view in) noexcept
    -> blockchain::crypto::AddressStyle
{
    const auto& map = string_to_style_;

    if (const auto i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return blockchain::crypto::AddressStyle::unknown_address_style;
    }
}

static auto translate_style(const blockchain::crypto::AddressStyle& in) noexcept
    -> std::string_view
{
    const auto& map = style_to_string_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "0"sv;
    }
}

struct Contact::Imp {
    const api::session::Client& api_;
    VersionNumber version_{0};
    UnallocatedCString label_{""};
    mutable std::mutex lock_{};
    const identifier::Generic id_;
    identifier::Generic parent_;
    identifier::Nym primary_nym_;
    UnallocatedMap<identifier::Nym, Nym_p> nyms_;
    UnallocatedSet<identifier::Generic> merged_children_;
    std::unique_ptr<identity::wot::claim::Data> contact_data_{};
    mutable std::shared_ptr<identity::wot::claim::Data> cached_contact_data_{};
    std::atomic<std::uint64_t> revision_{0};

    static auto check_version(
        const VersionNumber in,
        const VersionNumber targetVersion) -> VersionNumber
    {
        // Upgrade version
        if (targetVersion > in) { return targetVersion; }

        return in;
    }

    static auto generate_id(const api::Session& api) -> identifier::Generic
    {
        const auto& encode = api.Crypto().Encode().InternalEncode();
        auto random = ByteArray{};
        encode.Nonce(ID_BYTES, random);

        return api.Factory().IdentifierFromPreimage(random.Bytes());
    }

    static auto translate(
        const api::session::Client& api,
        const UnitType chain,
        std::string_view value,
        ReadView subtype) noexcept(false) -> std::
        tuple<ByteArray, blockchain::crypto::AddressStyle, blockchain::Type>
    {
        auto output = std::tuple<
            ByteArray,
            blockchain::crypto::AddressStyle,
            blockchain::Type>{
            api.Factory().DataFromHex(value),
            translate_style(subtype),
            unit_to_blockchain(chain)};
        auto& [outBytes, outStyle, outChain] = output;
        const auto bad =
            outBytes.empty() ||
            (blockchain::crypto::AddressStyle::unknown_address_style ==
             outStyle) ||
            (blockchain::Type::UnknownBlockchain == outChain);

        if (bad) { throw std::runtime_error("Invalid address"); }

        return output;
    }

    Imp(const api::session::Client& api, const proto::Contact& serialized)
        : api_(api)
        , version_(check_version(serialized.version(), OT_CONTACT_VERSION))
        , label_(serialized.label())
        , lock_()
        , id_(api_.Factory().IdentifierFromBase58(serialized.id()))
        , parent_(api_.Factory().IdentifierFromBase58(serialized.mergedto()))
        , primary_nym_()
        , nyms_()
        , merged_children_()
        , contact_data_(new identity::wot::claim::Data(
              api_,
              serialized.id(),
              identity::wot::claim::DefaultVersion(),
              identity::wot::claim::DefaultVersion(),
              identity::wot::claim::Data::SectionMap{}))
        , cached_contact_data_()
        , revision_(serialized.revision())
    {
        if (serialized.has_contactdata()) {
            contact_data_ = std::make_unique<identity::wot::claim::Data>(
                api_,
                serialized.id(),
                identity::wot::claim::DefaultVersion(),
                serialized.contactdata());
        }

        assert_false(nullptr == contact_data_);

        for (const auto& child : serialized.merged()) {
            merged_children_.emplace(
                api_.Factory().IdentifierFromBase58(child));
        }

        init_nyms();
    }

    Imp(const api::session::Client& api, std::string_view label)
        : api_(api)
        , version_(OT_CONTACT_VERSION)
        , label_(label)
        , lock_()
        , id_(generate_id(api_))
        , parent_()
        , primary_nym_()
        , nyms_()
        , merged_children_()
        , contact_data_(nullptr)
        , cached_contact_data_()
        , revision_(1)
    {
        contact_data_ = std::make_unique<identity::wot::claim::Data>(
            api_,
            String::Factory(id_, api_.Crypto())->Get(),
            identity::wot::claim::DefaultVersion(),
            identity::wot::claim::DefaultVersion(),
            identity::wot::claim::Data::SectionMap{});

        assert_false(nullptr == contact_data_);
    }

    auto add_claim(const std::shared_ptr<identity::wot::claim::Item>& item)
        -> bool
    {
        auto lock = Lock{lock_};

        return add_claim(lock, item);
    }

    auto add_claim(
        const Lock& lock,
        const std::shared_ptr<identity::wot::claim::Item>& item) -> bool
    {
        assert_true(verify_write_lock(lock));

        if (false == bool(item)) {
            LogError()()("Null claim.").Flush();

            return false;
        }

        const auto version = std::make_pair(
            item->Version(), identity::wot::claim::translate(item->Section()));
        const auto proto = [&] {
            auto out = proto::ContactItem{};
            item->Internal().Serialize(out, true);
            return out;
        }();

        if (false == proto::Validate<proto::ContactItem>(
                         proto, VERBOSE, proto::ClaimType::Indexed, version)) {
            LogError()()("Invalid claim.").Flush();

            return false;
        }

        add_verified_claim(lock, item);

        return true;
    }

    auto add_nym(const Lock& lock, const Nym_p& nym, const bool primary) -> bool
    {
        assert_true(verify_write_lock(lock));

        if (false == bool(nym)) { return false; }

        const auto contactType = type(lock);
        const auto nymType = ExtractType(*nym);
        const bool haveType =
            (identity::wot::claim::ClaimType::Error != contactType) &&
            (identity::wot::claim::ClaimType::Unknown != contactType);
        const bool typeMismatch = (contactType != nymType);

        if (haveType && typeMismatch) {
            LogError()()("Wrong nym type.").Flush();

            return false;
        }

        const auto& id = nym->ID();
        const bool needPrimary = (0 == nyms_.size());
        const bool isPrimary = needPrimary || primary;
        nyms_[id] = nym;

        if (isPrimary) { primary_nym_ = id; }

        add_nym_claim(lock, id, isPrimary);

        return true;
    }

    void add_nym_claim(
        const Lock& lock,
        const identifier::Nym& nymID,
        const bool primary)
    {
        assert_true(verify_write_lock(lock));

        auto attr = boost::container::flat_set{
            identity::wot::claim::Attribute::Local,
            identity::wot::claim::Attribute::Active};

        if (primary) { attr.emplace(identity::wot::claim::Attribute::Primary); }

        auto claim =
            std::make_shared<identity::wot::claim::Item>(factory::ContactItem(
                api_.Factory().Claim(
                    id_,
                    identity::wot::claim::SectionType::Relationship,
                    identity::wot::claim::ClaimType::Contact,
                    nymID.asBase58(api_.Crypto()),
                    attr),
                {}  // TODO allocator
                ));
        add_claim(lock, claim);
    }

    void add_verified_claim(
        const Lock& lock,
        const std::shared_ptr<identity::wot::claim::Item>& item)
    {
        assert_true(verify_write_lock(lock));
        assert_false(nullptr == contact_data_);

        // NOLINTNEXTLINE(modernize-make-unique)
        contact_data_.reset(
            new identity::wot::claim::Data(contact_data_->AddItem(item)));

        assert_false(nullptr == contact_data_);

        revision_++;
        cached_contact_data_.reset();
    }

    void init_nyms()
    {
        assert_false(nullptr == contact_data_);

        const auto nyms = contact_data_->Group(
            identity::wot::claim::SectionType::Relationship,
            identity::wot::claim::ClaimType::Contact);

        if (false == bool(nyms)) { return; }

        primary_nym_ =
            api_.Factory().Internal().NymIDConvertSafe(nyms->Primary());

        for (const auto& it : *nyms) {
            const auto& item = it.second;

            assert_false(nullptr == item);

            const auto nymID = api_.Factory().NymIDFromBase58(item->Value());
            auto& nym = nyms_[nymID];
            nym = api_.Wallet().Nym(nymID);

            if (false == bool(nym)) {
                LogVerbose()()("Failed to load nym ")(nymID, api_.Crypto())(".")
                    .Flush();
            }
        }
    }

    auto merged_data(const Lock& lock) const
        -> std::shared_ptr<identity::wot::claim::Data>
    {
        assert_false(nullptr == contact_data_);
        assert_true(verify_write_lock(lock));

        if (cached_contact_data_) { return cached_contact_data_; }

        cached_contact_data_.reset(
            new identity::wot::claim::Data(*contact_data_));
        auto& output = cached_contact_data_;

        assert_false(nullptr == output);

        if (false == primary_nym_.empty()) {
            try {
                const auto& primary = nyms_.at(primary_nym_);

                if (primary) {
                    output.reset(new identity::wot::claim::Data(
                        *output + primary->Claims()));
                }
            } catch (const std::out_of_range&) {
            }
        }

        for (const auto& it : nyms_) {
            const auto& nymID = it.first;
            const auto& nym = it.second;

            if (false == bool(nym)) { continue; }

            if (nymID == primary_nym_) { continue; }

            output.reset(
                new identity::wot::claim::Data(*output + nym->Claims()));
        }

        return output;
    }

    auto payment_codes(const Lock& lock, const UnitType currency) const
        -> std::shared_ptr<identity::wot::claim::Group>
    {
        const auto data = merged_data(lock);

        if (false == bool(data)) { return {}; }

        return data->Group(
            identity::wot::claim::SectionType::Procedure,
            UnitToClaim(currency));
    }

    auto type(const Lock& lock) const -> identity::wot::claim::ClaimType
    {
        assert_true(verify_write_lock(lock));

        const auto data = merged_data(lock);

        if (false == bool(data)) {
            return identity::wot::claim::ClaimType::Error;
        }

        return data->Type();
    }

    void update_label(const Lock& lock, const identity::Nym& nym)
    {
        assert_true(verify_write_lock(lock));

        if (false == label_.empty()) { return; }

        label_ = ExtractLabel(nym);
    }

    auto verify_write_lock(const Lock& lock) const -> bool
    {
        if (lock.mutex() != &lock_) {
            LogError()()("Incorrect mutex.").Flush();

            return false;
        }

        if (false == lock.owns_lock()) {
            LogError()()("Lock not owned.").Flush();

            return false;
        }

        return true;
    }
};

Contact::Contact(
    const api::session::Client& api,
    const proto::Contact& serialized)
    : imp_(std::make_unique<Imp>(api, serialized))
{
    assert_false(nullptr == imp_);
}

Contact::Contact(const api::session::Client& api, std::string_view label)
    : imp_(std::make_unique<Imp>(api, label))
{
    assert_false(nullptr == imp_);
}

auto Contact::operator+=(Contact& rhs) -> Contact&
{
    auto rlock = Lock{rhs.imp_->lock_, std::defer_lock};
    auto lock = Lock{imp_->lock_, std::defer_lock};
    std::lock(rlock, lock);

    if (imp_->label_.empty()) { imp_->label_ = rhs.imp_->label_; }

    rhs.imp_->parent_ = imp_->id_;

    if (imp_->primary_nym_.empty()) {
        imp_->primary_nym_ = rhs.imp_->primary_nym_;
    }

    for (const auto& it : rhs.imp_->nyms_) {
        const auto& id = it.first;
        const auto& nym = it.second;

        if (false == imp_->nyms_.contains(id)) { imp_->nyms_.emplace(id, nym); }
    }

    rhs.imp_->nyms_.clear();

    for (const auto& it : rhs.imp_->merged_children_) {
        imp_->merged_children_.insert(it);
    }

    imp_->merged_children_.insert(rhs.imp_->id_);
    rhs.imp_->merged_children_.clear();

    if (imp_->contact_data_) {
        if (rhs.imp_->contact_data_) {
            // NOLINTNEXTLINE(modernize-make-unique)
            imp_->contact_data_.reset(new identity::wot::claim::Data(
                *imp_->contact_data_ + *rhs.imp_->contact_data_));
        }
    } else {
        if (rhs.imp_->contact_data_) {
            imp_->contact_data_ = std::make_unique<identity::wot::claim::Data>(
                *rhs.imp_->contact_data_);
        }
    }

    rhs.imp_->contact_data_.reset();
    imp_->cached_contact_data_.reset();
    rhs.imp_->cached_contact_data_.reset();

    return *this;
}

auto Contact::AddBlockchainAddress(
    const UnallocatedCString& address,
    const blockchain::Type type) -> bool
{
    const auto& api = imp_->api_;
    auto [bytes, style, chains, supported] =
        api.Crypto().Blockchain().DecodeAddress(address);
    const auto bad =
        bytes.empty() ||
        (blockchain::crypto::AddressStyle::unknown_address_style == style) ||
        chains.empty();

    if (bad) {
        LogError()()("Failed to decode address").Flush();

        return false;
    }

    if (false == chains.contains(type)) {
        LogError()()("Address is not valid for specified chain").Flush();

        return false;
    }

    return AddBlockchainAddress(style, type, bytes);
}

auto Contact::AddBlockchainAddress(
    const blockchain::crypto::AddressStyle& style,
    const blockchain::Type chain,
    const opentxs::Data& bytes) -> bool
{
    auto lock = Lock{imp_->lock_};
    using enum identity::wot::claim::Attribute;
    static const auto attrib =
        Vector<identity::wot::claim::Attribute>{Active, Local};
    auto claim =
        std::make_shared<identity::wot::claim::Item>(factory::ContactItem(
            imp_->api_.Factory().Claim(
                imp_->id_,
                identity::wot::claim::SectionType::Address,
                UnitToClaim(blockchain_to_unit(chain)),
                bytes.asHex(),
                attrib,
                {},
                {},
                translate_style(style)),
            {}  // TODO allocator
            ));

    return imp_->add_claim(lock, claim);
}

auto Contact::AddEmail(
    const UnallocatedCString& value,
    const bool primary,
    const bool active) -> bool
{
    if (value.empty()) { return false; }

    auto lock = Lock{imp_->lock_};

    // NOLINTNEXTLINE(modernize-make-unique)
    imp_->contact_data_.reset(new identity::wot::claim::Data(
        imp_->contact_data_->AddEmail(value, primary, active)));

    assert_false(nullptr == imp_->contact_data_);

    imp_->revision_++;
    imp_->cached_contact_data_.reset();

    return true;
}

auto Contact::AddNym(const Nym_p& nym, const bool primary) -> bool
{
    auto lock = Lock{imp_->lock_};

    return imp_->add_nym(lock, nym, primary);
}

auto Contact::AddNym(const identifier::Nym& nymID, const bool primary) -> bool
{
    auto lock = Lock{imp_->lock_};

    const bool needPrimary = (0 == imp_->nyms_.size());
    const bool isPrimary = needPrimary || primary;

    if (isPrimary) { imp_->primary_nym_ = nymID; }

    imp_->add_nym_claim(lock, nymID, isPrimary);

    imp_->revision_++;

    return true;
}

auto Contact::AddPaymentCode(
    const opentxs::PaymentCode& code,
    const bool primary,
    const UnitType currency,
    const bool active) -> bool
{
    auto attr =
        boost::container::flat_set{identity::wot::claim::Attribute::Local};

    if (active) { attr.emplace(identity::wot::claim::Attribute::Active); }

    if (primary) { attr.emplace(identity::wot::claim::Attribute::Primary); }

    auto claim =
        std::make_shared<identity::wot::claim::Item>(factory::ContactItem(
            imp_->api_.Factory().Claim(
                imp_->id_,
                identity::wot::claim::SectionType::Procedure,
                UnitToClaim(currency),
                code.asBase58(),
                attr),
            {}  // TODO allocator
            ));
    claim->SetVersion(identity::wot::claim::DefaultVersion());

    if (false == imp_->add_claim(claim)) {
        LogError()()("Unable to add claim.").Flush();

        return false;
    }

    return true;
}

auto Contact::AddPhoneNumber(
    const UnallocatedCString& value,
    const bool primary,
    const bool active) -> bool
{
    if (value.empty()) { return false; }

    auto lock = Lock{imp_->lock_};

    // NOLINTNEXTLINE(modernize-make-unique)
    imp_->contact_data_.reset(new identity::wot::claim::Data(
        imp_->contact_data_->AddPhoneNumber(value, primary, active)));

    assert_false(nullptr == imp_->contact_data_);

    imp_->revision_++;
    imp_->cached_contact_data_.reset();

    return true;
}

auto Contact::AddSocialMediaProfile(
    const UnallocatedCString& value,
    const identity::wot::claim::ClaimType type,
    const bool primary,
    const bool active) -> bool
{
    if (value.empty()) { return false; }

    auto lock = Lock{imp_->lock_};

    // NOLINTNEXTLINE(modernize-make-unique)
    imp_->contact_data_.reset(new identity::wot::claim::Data(
        imp_->contact_data_->AddSocialMediaProfile(
            value, type, primary, active)));

    assert_false(nullptr == imp_->contact_data_);

    imp_->revision_++;
    imp_->cached_contact_data_.reset();

    return true;
}

auto Contact::Best(const identity::wot::claim::Group& group)
    -> std::shared_ptr<identity::wot::claim::Item>
{
    if (0 == group.Size()) { return {}; }

    const auto primary = group.PrimaryClaim();

    if (primary) { return primary; }

    for (const auto& it : group) {
        const auto& claim = it.second;

        if (claim->HasAttribute(identity::wot::claim::Attribute::Active)) {
            return claim;
        }
    }

    return group.begin()->second;
}

auto Contact::BestEmail() const -> UnallocatedCString
{
    auto lock = Lock{imp_->lock_};
    const auto data = imp_->merged_data(lock);
    lock.unlock();

    if (false == bool(data)) { return {}; }

    return data->BestEmail();
}

auto Contact::BestPhoneNumber() const -> UnallocatedCString
{
    auto lock = Lock{imp_->lock_};
    const auto data = imp_->merged_data(lock);
    lock.unlock();

    if (false == bool(data)) { return {}; }

    return data->BestPhoneNumber();
}

auto Contact::BestSocialMediaProfile(
    const identity::wot::claim::ClaimType type) const -> UnallocatedCString
{
    auto lock = Lock{imp_->lock_};
    const auto data = imp_->merged_data(lock);
    lock.unlock();

    if (false == bool(data)) { return {}; }

    return data->BestSocialMediaProfile(type);
}

auto Contact::BlockchainAddresses() const -> UnallocatedVector<
    std::tuple<ByteArray, blockchain::crypto::AddressStyle, blockchain::Type>>
{
    auto output = UnallocatedVector<std::tuple<
        ByteArray,
        blockchain::crypto::AddressStyle,
        blockchain::Type>>{};
    auto lock = Lock{imp_->lock_};
    auto data = imp_->merged_data(lock);
    lock.unlock();

    if (false == bool(data)) { return {}; }

    const auto& version = data->Version();
    const auto section =
        data->Section(identity::wot::claim::SectionType::Address);

    if (false == bool(section)) { return {}; }

    for (const auto& it : *section) {
        const auto& type = it.first;
        const auto& group = it.second;

        assert_false(nullptr == group);

        const bool currency = proto::ValidContactItemType(
            {version, translate(identity::wot::claim::SectionType::Contract)},
            translate(type));

        if (false == currency) { continue; }

        for (const auto& inner : *group) {
            const auto& item = inner.second;

            assert_false(nullptr == item);

            try {
                output.push_back(Imp::translate(
                    imp_->api_,
                    ClaimToUnit(type),
                    item->Value(),
                    item->Subtype()));
            } catch (...) {
                continue;
            }
        }
    }

    return output;
}

auto Contact::Data() const -> std::shared_ptr<identity::wot::claim::Data>
{
    auto lock = Lock{imp_->lock_};

    return imp_->merged_data(lock);
}

auto Contact::EmailAddresses(bool active) const -> UnallocatedCString
{
    auto lock = Lock{imp_->lock_};
    const auto data = imp_->merged_data(lock);
    lock.unlock();

    if (false == bool(data)) { return {}; }

    return data->EmailAddresses(active);
}

auto Contact::ExtractLabel(const identity::Nym& nym) -> UnallocatedCString
{
    return nym.Claims().Name();
}

auto Contact::ExtractType(const identity::Nym& nym)
    -> identity::wot::claim::ClaimType
{
    return nym.Claims().Type();
}

auto Contact::ID() const -> const identifier::Generic& { return imp_->id_; }

auto Contact::Label() const -> const UnallocatedCString&
{
    return imp_->label_;
}

auto Contact::LastUpdated() const -> std::time_t
{
    assert_false(nullptr == imp_->contact_data_);

    const auto group = imp_->contact_data_->Group(
        identity::wot::claim::SectionType::Event,
        identity::wot::claim::ClaimType::Refreshed);

    if (false == bool(group)) { return {}; }

    const auto claim = group->PrimaryClaim();

    if (false == bool(claim)) { return {}; }

    try {
        const auto stlWorkaround = UnallocatedCString{claim->Value()};

        if (sizeof(int) == sizeof(std::time_t)) {

            return std::stoi(stlWorkaround);
        } else if (sizeof(long) == sizeof(std::time_t)) {

            return std::stol(stlWorkaround);
        } else if (sizeof(long long) == sizeof(std::time_t)) {

            return std::stoll(stlWorkaround);
        } else {
            LogAbort()().Abort();
        }

    } catch (const std::out_of_range&) {

        return {};
    } catch (const std::invalid_argument&) {

        return {};
    }
}

auto Contact::Nyms(const bool includeInactive) const
    -> UnallocatedVector<opentxs::identifier::Nym>
{
    auto lock = Lock{imp_->lock_};
    const auto data = imp_->merged_data(lock);
    lock.unlock();

    if (false == bool(data)) { return {}; }

    const auto group = data->Group(
        identity::wot::claim::SectionType::Relationship,
        identity::wot::claim::ClaimType::Contact);

    if (false == bool(group)) { return {}; }

    UnallocatedVector<identifier::Nym> output{};
    const auto& primaryID = group->Primary();

    for (const auto& it : *group) {
        const auto& item = it.second;

        assert_false(nullptr == item);

        const auto& itemID = item->ID();

        if (false ==
            (includeInactive ||
             item->HasAttribute(identity::wot::claim::Attribute::Active))) {
            continue;
        }

        if (primaryID == itemID) {
            output.emplace(
                output.begin(),
                imp_->api_.Factory().NymIDFromBase58(item->Value()));
        } else {
            output.emplace(
                output.end(),
                imp_->api_.Factory().NymIDFromBase58(item->Value()));
        }
    }

    return output;
}

auto Contact::PaymentCode(
    const identity::wot::claim::Data& data,
    const UnitType currency) -> UnallocatedCString
{
    auto group = data.Group(
        identity::wot::claim::SectionType::Procedure, UnitToClaim(currency));

    if (false == bool(group)) { return {}; }

    const auto item = Best(*group);

    if (false == bool(item)) { return {}; }

    return UnallocatedCString{item->Value()};
}

auto Contact::PaymentCode(const UnitType currency) const -> UnallocatedCString
{
    auto lock = Lock{imp_->lock_};
    const auto data = imp_->merged_data(lock);
    lock.unlock();

    if (false == bool(data)) { return {}; }

    return PaymentCode(*data, currency);
}

auto Contact::PaymentCodes(const UnitType currency) const
    -> UnallocatedVector<UnallocatedCString>
{
    auto lock = Lock{imp_->lock_};
    const auto group = imp_->payment_codes(lock, currency);
    lock.unlock();

    if (false == bool(group)) { return {}; }

    UnallocatedVector<UnallocatedCString> output{};

    for (const auto& it : *group) {
        assert_false(nullptr == it.second);

        const auto& item = *it.second;
        output.emplace_back(item.Value());
    }

    return output;
}

auto Contact::PaymentCodes(alloc::Default alloc) const
    -> Set<opentxs::PaymentCode>
{
    auto out = Set<opentxs::PaymentCode>{alloc};
    const auto& api = imp_->api_;
    const auto data = [this] {
        auto lock = Lock{imp_->lock_};

        return imp_->merged_data(lock);
    }();

    if (nullptr == data) { return out; }

    using SectionType = identity::wot::claim::SectionType;
    const auto section = data->Section(SectionType::Procedure);

    if (nullptr == section) { return out; }

    for (const auto& [type, group] : *section) {
        for (const auto& [id, item] : *group) {
            auto code = api.Factory().PaymentCodeFromBase58(item->Value());

            if (code.Valid()) { out.emplace(std::move(code)); }
        }
    }

    return out;
}

auto Contact::PhoneNumbers(bool active) const -> UnallocatedCString
{
    auto lock = Lock{imp_->lock_};
    const auto data = imp_->merged_data(lock);
    lock.unlock();

    if (false == bool(data)) { return {}; }

    return data->PhoneNumbers(active);
}

auto Contact::Print() const -> UnallocatedCString
{
    auto lock = Lock{imp_->lock_};
    std::stringstream out{};
    out << "Contact: " << String::Factory(imp_->id_, imp_->api_.Crypto())->Get()
        << ", version " << imp_->version_ << "revision " << imp_->revision_
        << "\n"
        << "Label: " << imp_->label_ << "\n";

    if (false == imp_->parent_.empty()) {
        out << "Merged to: "
            << String::Factory(imp_->parent_, imp_->api_.Crypto())->Get()
            << "\n";
    }

    if (false == imp_->merged_children_.empty()) {
        out << "Merged contacts:\n";

        for (const auto& id : imp_->merged_children_) {
            out << " * " << String::Factory(id, imp_->api_.Crypto())->Get()
                << "\n";
        }
    }

    if (0 < imp_->nyms_.size()) {
        out << "Contains nyms:\n";

        for (const auto& it : imp_->nyms_) {
            const auto& id = it.first;
            out << " * " << String::Factory(id, imp_->api_.Crypto())->Get();

            if (id == imp_->primary_nym_) { out << " (primary)"; }

            out << "\n";
        }
    }

    auto data = imp_->merged_data(lock);

    if (data) { out << UnallocatedCString(*data); }

    out << std::endl;

    return out.str();
}

auto Contact::RemoveNym(const identifier::Nym& nymID) -> bool
{
    auto lock = Lock{imp_->lock_};

    auto result = imp_->nyms_.erase(nymID);

    if (imp_->primary_nym_ == nymID) { imp_->primary_nym_ = {}; }

    return (0 < result);
}

auto Contact::Serialize(proto::Contact& output) const -> bool
{
    auto lock = Lock{imp_->lock_};
    output.set_version(imp_->version_);
    output.set_id(String::Factory(imp_->id_, imp_->api_.Crypto())->Get());
    output.set_revision(imp_->revision_);
    output.set_label(imp_->label_);

    if (imp_->contact_data_) {
        imp_->contact_data_->Serialize(*output.mutable_contactdata());
    }

    output.set_mergedto(
        String::Factory(imp_->parent_, imp_->api_.Crypto())->Get());

    for (const auto& child : imp_->merged_children_) {
        output.add_merged(String::Factory(child, imp_->api_.Crypto())->Get());
    }

    return true;
}

void Contact::SetLabel(std::string_view label)
{
    auto lock = Lock{imp_->lock_};
    imp_->label_ = label;
    imp_->revision_++;

    for (const auto& it : imp_->nyms_) {
        const auto& nymID = it.first;
        imp_->api_.Wallet().SetNymAlias(nymID, label);
    }
}

auto Contact::SocialMediaProfiles(
    const identity::wot::claim::ClaimType type,
    bool active) const -> UnallocatedCString
{
    auto lock = Lock{imp_->lock_};
    const auto data = imp_->merged_data(lock);
    lock.unlock();

    if (false == bool(data)) { return {}; }

    return data->SocialMediaProfiles(type, active);
}

auto Contact::SocialMediaProfileTypes() const
    -> const UnallocatedSet<identity::wot::claim::ClaimType>
{
    auto lock = Lock{imp_->lock_};
    const auto data = imp_->merged_data(lock);
    lock.unlock();

    return data->SocialMediaProfileTypes();
}

auto Contact::Type() const -> identity::wot::claim::ClaimType
{
    auto lock = Lock{imp_->lock_};

    return imp_->type(lock);
}

void Contact::Update(const proto::Nym& serialized)
{
    auto nym = imp_->api_.Wallet().Internal().Nym(serialized);

    if (false == bool(nym)) {
        LogError()()("Invalid serialized nym.").Flush();

        return;
    }

    auto lock = Lock{imp_->lock_};
    const auto& nymID = nym->ID();
    auto it = imp_->nyms_.find(nymID);

    if (imp_->nyms_.end() == it) {
        imp_->add_nym(lock, nym, false);
    } else {
        it->second = nym;
    }

    imp_->update_label(lock, *nym);
    using enum identity::wot::claim::Attribute;
    static const auto attrib =
        Vector<identity::wot::claim::Attribute>{Primary, Active, Local};
    const auto now = Clock::to_time_t(Clock::now());
    auto claim =
        std::make_shared<identity::wot::claim::Item>(factory::ContactItem(
            imp_->api_.Factory().Claim(
                imp_->id_,
                identity::wot::claim::SectionType::Event,
                identity::wot::claim::ClaimType::Refreshed,
                std::to_string(now),
                attrib),
            {}  // TODO allocator
            ));
    claim->SetVersion(identity::wot::claim::DefaultVersion());
    imp_->add_claim(lock, claim);
}

Contact::~Contact() = default;
}  // namespace opentxs
