// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::api::session::Contacts

#include "api/session/Contacts.hpp"  // IWYU pragma: associated

#include <Contact.pb.h>  // IWYU pragma: keep
#include <Nym.pb.h>      // IWYU pragma: keep
#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <span>
#include <stdexcept>

#include "BoostAsio.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/session/Factory.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/alloc/Boost.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Contact.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/UnitType.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/Data.hpp"
#include "opentxs/identity/wot/claim/Group.hpp"
#include "opentxs/identity/wot/claim/Item.hpp"
#include "opentxs/identity/wot/claim/Section.hpp"
#include "opentxs/identity/wot/claim/SectionType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::factory
{
auto ContactAPI(const api::session::Client& api) noexcept
    -> std::unique_ptr<api::session::Contacts>
{
    using ReturnType = opentxs::api::session::imp::Contacts;

    return std::make_unique<ReturnType>(api);
}
}  // namespace opentxs::factory

namespace opentxs::api::session::imp
{
Contacts::Contacts(const api::session::Client& api)
    : api_(api)
    , lock_()
    , blockchain_()
    , contact_map_()
    , contact_name_map_([&] {
        auto output = ContactNameMap{};

        for (const auto& [id, alias] : api_.Storage().ContactList()) {
            output.emplace(api_.Factory().IdentifierFromBase58(id), alias);
        }

        return output;
    }())
    , publisher_(api_.Network().ZeroMQ().Internal().PublishSocket())
    , pipeline_(api_.Network().ZeroMQ().Internal().Pipeline(
          [this](auto&& in) { pipeline(std::move(in)); },
          "api::session::Contacts",
          {{CString{api_.Endpoints().NymCreated()},
            opentxs::network::zeromq::socket::Direction::Connect},
           {CString{api_.Endpoints().NymDownload()},
            opentxs::network::zeromq::socket::Direction::Connect},
           {CString{api_.Endpoints().Shutdown()},
            opentxs::network::zeromq::socket::Direction::Connect}}))
    , timer_(api_.Network().Asio().Internal().GetTimer())
{
    // WARNING: do not access api_.Wallet() during construction
    publisher_->Start(api_.Endpoints().ContactUpdate().data());

    // TODO update Storage to record contact ids that need to be updated
    // in blockchain api in cases where the process was interrupted due to
    // library shutdown

    LogTrace()(OT_PRETTY_CLASS())("using ZMQ batch ")(pipeline_.BatchID())
        .Flush();
}

auto Contacts::add_contact(const rLock& lock, opentxs::Contact* contact) const
    -> Contacts::ContactMap::iterator
{
    OT_ASSERT(nullptr != contact);

    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    const auto& id = contact->ID();
    auto& it = contact_map_[id];
    it.second.reset(contact);

    return contact_map_.find(id);
}

void Contacts::check_identifiers(
    const identifier::Generic& inputNymID,
    const PaymentCode& paymentCode,
    bool& haveNymID,
    bool& havePaymentCode,
    identifier::Nym& outputNymID) const
{
    if (paymentCode.Valid()) { havePaymentCode = true; }

    if (false == inputNymID.empty()) {
        haveNymID = true;
        outputNymID.Assign(inputNymID);
    } else if (havePaymentCode) {
        haveNymID = true;
        outputNymID.Assign(paymentCode.ID());
    }
}

auto Contacts::check_nyms() noexcept -> void
{
    auto buf = std::array<std::byte, 4096>{};
    auto alloc = alloc::BoostMonotonic{buf.data(), buf.size()};
    const auto contacts = [&] {
        auto out = Vector<identifier::Generic>{&alloc};
        auto lock = rLock{lock_};
        out.reserve(contact_name_map_.size());

        for (auto& [key, value] : contact_name_map_) { out.emplace_back(key); }

        return out;
    }();
    auto nyms = Vector<identifier::Nym>{&alloc};

    for (const auto& id : contacts) {
        const auto contact = Contact(id);

        OT_ASSERT(contact);

        auto ids = contact->Nyms();
        std::move(ids.begin(), ids.end(), std::back_inserter(nyms));
    }

    for (const auto& id : nyms) {
        const auto nym = api_.Wallet().Nym(id);

        if (nym) {
            LogInsane()(OT_PRETTY_CLASS())(id, api_.Crypto())("found").Flush();
        } else {
            LogInsane()(OT_PRETTY_CLASS())(id, api_.Crypto())("not found")
                .Flush();
        }
    }
}

auto Contacts::contact(const rLock& lock, const identifier::Generic& id) const
    -> std::shared_ptr<const opentxs::Contact>
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    const auto it = obtain_contact(lock, id);

    if (contact_map_.end() != it) { return it->second.second; }

    return {};
}

auto Contacts::contact(const rLock& lock, std::string_view label) const
    -> std::shared_ptr<const opentxs::Contact>
{
    auto contact = std::make_unique<opentxs::Contact>(api_, label);

    if (false == bool(contact)) {
        LogError()(OT_PRETTY_CLASS())("Unable to create new contact.").Flush();

        return {};
    }

    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    const auto& contactID = contact->ID();

    OT_ASSERT(0 == contact_map_.count(contactID));

    auto it = add_contact(lock, contact.release());
    auto& output = it->second.second;
    const auto proto = [&] {
        auto out = proto::Contact{};
        output->Serialize(out);
        return out;
    }();

    if (false == api_.Storage().Store(proto)) {
        LogError()(OT_PRETTY_CLASS())("Unable to save contact.").Flush();
        contact_map_.erase(it);

        return {};
    }

    contact_name_map_[contactID] = output->Label();
    // Not parsing changed addresses because this is a new contact

    return output;
}

auto Contacts::Contact(const identifier::Generic& id) const
    -> std::shared_ptr<const opentxs::Contact>
{
    auto lock = rLock{lock_};

    return contact(lock, id);
}

auto Contacts::ContactID(const identifier::Nym& nymID) const
    -> identifier::Generic
{
    return api_.Storage().ContactOwnerNym(nymID);
}

auto Contacts::ContactList() const -> ObjectList
{
    return api_.Storage().ContactList();
}

auto Contacts::ContactName(const identifier::Generic& id) const
    -> UnallocatedCString
{
    return ContactName(id, UnitType::Error);
}

auto Contacts::ContactName(const identifier::Generic& id, UnitType currencyHint)
    const -> UnallocatedCString
{
    auto alias = UnallocatedCString{};
    const auto fallback = [&](const rLock&) {
        if (false == alias.empty()) { return alias; }

        auto [it, added] =
            contact_name_map_.try_emplace(id, id.asBase58(api_.Crypto()));

        OT_ASSERT(added);

        return it->second;
    };
    auto lock = rLock{lock_};

    {
        auto it = contact_name_map_.find(id);

        if (contact_name_map_.end() != it) {
            alias = it->second;

            if (alias.empty()) { contact_name_map_.erase(it); }
        }
    }

    using Type = UnitType;

    if ((Type::Error == currencyHint) && (false == alias.empty())) {
        const auto isPaymentCode = [&] {
            auto code = api_.Factory().PaymentCodeFromBase58(alias);

            return code.Valid();
        }();

        if (false == isPaymentCode) { return alias; }
    }

    auto contact = this->contact(lock, id);

    if (!contact) { return fallback(lock); }

    if (const auto& label = contact->Label(); false == label.empty()) {
        auto& output = contact_name_map_[id];
        output = std::move(label);

        return output;
    }

    const auto data = contact->Data();

    OT_ASSERT(data);

    if (auto name = data->Name(); false == name.empty()) {
        auto& output = contact_name_map_[id];
        output = std::move(name);

        return output;
    }

    using Section = identity::wot::claim::SectionType;

    if (Type::Error != currencyHint) {
        auto group = data->Group(Section::Procedure, UnitToClaim(currencyHint));

        if (group) {
            if (auto best = group->Best(); best) {
                if (auto value = best->Value(); false == value.empty()) {

                    return best->Value();
                }
            }
        }
    }

    const auto procedure = data->Section(Section::Procedure);

    if (procedure) {
        for (const auto& [type, group] : *procedure) {
            OT_ASSERT(group);

            if (0 < group->Size()) {
                const auto item = group->Best();

                OT_ASSERT(item);

                if (auto value = item->Value(); false == value.empty()) {

                    return value;
                }
            }
        }
    }

    return fallback(lock);
}

auto Contacts::import_contacts(const rLock& lock) -> void
{
    auto nyms = api_.Wallet().NymList();

    for (const auto& it : nyms) {
        const auto nymID = api_.Factory().NymIDFromBase58(it.first);
        const auto contactID = [&] {
            auto out = identifier::Generic{};
            out.Assign(nymID.data(), nymID.size());

            return out;
        }();

        api_.Storage().ContactOwnerNym(nymID);

        if (contactID.empty()) {
            const auto nym = api_.Wallet().Nym(nymID);

            if (false == bool(nym)) {
                throw std::runtime_error("Unable to load nym");
            }

            switch (nym->Claims().Type()) {
                case identity::wot::claim::ClaimType::Individual:
                case identity::wot::claim::ClaimType::Organization:
                case identity::wot::claim::ClaimType::Business:
                case identity::wot::claim::ClaimType::Government:
                case identity::wot::claim::ClaimType::Bot: {
                    auto code = api_.Factory().PaymentCodeFromBase58(
                        nym->PaymentCode());
                    new_contact(lock, nym->Alias(), nymID, code);
                } break;
                case identity::wot::claim::ClaimType::Error:
                case identity::wot::claim::ClaimType::Server:
                case identity::wot::claim::ClaimType::Prefix:
                case identity::wot::claim::ClaimType::Forename:
                case identity::wot::claim::ClaimType::Middlename:
                case identity::wot::claim::ClaimType::Surname:
                case identity::wot::claim::ClaimType::Pedigree:
                case identity::wot::claim::ClaimType::Suffix:
                case identity::wot::claim::ClaimType::Nickname:
                case identity::wot::claim::ClaimType::Commonname:
                case identity::wot::claim::ClaimType::Passport:
                case identity::wot::claim::ClaimType::National:
                case identity::wot::claim::ClaimType::Provincial:
                case identity::wot::claim::ClaimType::Military:
                case identity::wot::claim::ClaimType::Pgp:
                case identity::wot::claim::ClaimType::Otr:
                case identity::wot::claim::ClaimType::Ssl:
                case identity::wot::claim::ClaimType::Physical:
                case identity::wot::claim::ClaimType::Official:
                case identity::wot::claim::ClaimType::Birthplace:
                case identity::wot::claim::ClaimType::Home:
                case identity::wot::claim::ClaimType::Website:
                case identity::wot::claim::ClaimType::Opentxs:
                case identity::wot::claim::ClaimType::Phone:
                case identity::wot::claim::ClaimType::Email:
                case identity::wot::claim::ClaimType::Skype:
                case identity::wot::claim::ClaimType::Wire:
                case identity::wot::claim::ClaimType::Qq:
                case identity::wot::claim::ClaimType::Bitmessage:
                case identity::wot::claim::ClaimType::Whatsapp:
                case identity::wot::claim::ClaimType::Telegram:
                case identity::wot::claim::ClaimType::Kik:
                case identity::wot::claim::ClaimType::Bbm:
                case identity::wot::claim::ClaimType::Wechat:
                case identity::wot::claim::ClaimType::Kakaotalk:
                case identity::wot::claim::ClaimType::Facebook:
                case identity::wot::claim::ClaimType::Google:
                case identity::wot::claim::ClaimType::Linkedin:
                case identity::wot::claim::ClaimType::Vk:
                case identity::wot::claim::ClaimType::Aboutme:
                case identity::wot::claim::ClaimType::Onename:
                case identity::wot::claim::ClaimType::Twitter:
                case identity::wot::claim::ClaimType::Medium:
                case identity::wot::claim::ClaimType::Tumblr:
                case identity::wot::claim::ClaimType::Yahoo:
                case identity::wot::claim::ClaimType::Myspace:
                case identity::wot::claim::ClaimType::Meetup:
                case identity::wot::claim::ClaimType::Reddit:
                case identity::wot::claim::ClaimType::Hackernews:
                case identity::wot::claim::ClaimType::Wikipedia:
                case identity::wot::claim::ClaimType::Angellist:
                case identity::wot::claim::ClaimType::Github:
                case identity::wot::claim::ClaimType::Bitbucket:
                case identity::wot::claim::ClaimType::Youtube:
                case identity::wot::claim::ClaimType::Vimeo:
                case identity::wot::claim::ClaimType::Twitch:
                case identity::wot::claim::ClaimType::Snapchat:
                case identity::wot::claim::ClaimType::Vine:
                case identity::wot::claim::ClaimType::Instagram:
                case identity::wot::claim::ClaimType::Pinterest:
                case identity::wot::claim::ClaimType::Imgur:
                case identity::wot::claim::ClaimType::Flickr:
                case identity::wot::claim::ClaimType::Dribble:
                case identity::wot::claim::ClaimType::Behance:
                case identity::wot::claim::ClaimType::Deviantart:
                case identity::wot::claim::ClaimType::Spotify:
                case identity::wot::claim::ClaimType::Itunes:
                case identity::wot::claim::ClaimType::Soundcloud:
                case identity::wot::claim::ClaimType::Askfm:
                case identity::wot::claim::ClaimType::Ebay:
                case identity::wot::claim::ClaimType::Etsy:
                case identity::wot::claim::ClaimType::Openbazaar:
                case identity::wot::claim::ClaimType::Xboxlive:
                case identity::wot::claim::ClaimType::Playstation:
                case identity::wot::claim::ClaimType::Secondlife:
                case identity::wot::claim::ClaimType::Warcraft:
                case identity::wot::claim::ClaimType::Alias:
                case identity::wot::claim::ClaimType::Acquaintance:
                case identity::wot::claim::ClaimType::Friend:
                case identity::wot::claim::ClaimType::Spouse:
                case identity::wot::claim::ClaimType::Sibling:
                case identity::wot::claim::ClaimType::Member:
                case identity::wot::claim::ClaimType::Colleague:
                case identity::wot::claim::ClaimType::Parent:
                case identity::wot::claim::ClaimType::Child:
                case identity::wot::claim::ClaimType::Employer:
                case identity::wot::claim::ClaimType::Employee:
                case identity::wot::claim::ClaimType::Citizen:
                case identity::wot::claim::ClaimType::Photo:
                case identity::wot::claim::ClaimType::Gender:
                case identity::wot::claim::ClaimType::Height:
                case identity::wot::claim::ClaimType::Weight:
                case identity::wot::claim::ClaimType::Hair:
                case identity::wot::claim::ClaimType::Eye:
                case identity::wot::claim::ClaimType::Skin:
                case identity::wot::claim::ClaimType::Ethnicity:
                case identity::wot::claim::ClaimType::Language:
                case identity::wot::claim::ClaimType::Degree:
                case identity::wot::claim::ClaimType::Certification:
                case identity::wot::claim::ClaimType::Title:
                case identity::wot::claim::ClaimType::Skill:
                case identity::wot::claim::ClaimType::Award:
                case identity::wot::claim::ClaimType::Likes:
                case identity::wot::claim::ClaimType::Sexual:
                case identity::wot::claim::ClaimType::Political:
                case identity::wot::claim::ClaimType::Religious:
                case identity::wot::claim::ClaimType::Birth:
                case identity::wot::claim::ClaimType::Secondarygraduation:
                case identity::wot::claim::ClaimType::Universitygraduation:
                case identity::wot::claim::ClaimType::Wedding:
                case identity::wot::claim::ClaimType::Accomplishment:
                case identity::wot::claim::ClaimType::Btc:
                case identity::wot::claim::ClaimType::Eth:
                case identity::wot::claim::ClaimType::Xrp:
                case identity::wot::claim::ClaimType::Ltc:
                case identity::wot::claim::ClaimType::Dao:
                case identity::wot::claim::ClaimType::Xem:
                case identity::wot::claim::ClaimType::Dash:
                case identity::wot::claim::ClaimType::Maid:
                case identity::wot::claim::ClaimType::Lsk:
                case identity::wot::claim::ClaimType::Doge:
                case identity::wot::claim::ClaimType::Dgd:
                case identity::wot::claim::ClaimType::Xmr:
                case identity::wot::claim::ClaimType::Waves:
                case identity::wot::claim::ClaimType::Nxt:
                case identity::wot::claim::ClaimType::Sc:
                case identity::wot::claim::ClaimType::Steem:
                case identity::wot::claim::ClaimType::Amp:
                case identity::wot::claim::ClaimType::Xlm:
                case identity::wot::claim::ClaimType::Fct:
                case identity::wot::claim::ClaimType::Bts:
                case identity::wot::claim::ClaimType::Usd:
                case identity::wot::claim::ClaimType::Eur:
                case identity::wot::claim::ClaimType::Gbp:
                case identity::wot::claim::ClaimType::Inr:
                case identity::wot::claim::ClaimType::Aud:
                case identity::wot::claim::ClaimType::Cad:
                case identity::wot::claim::ClaimType::Sgd:
                case identity::wot::claim::ClaimType::Chf:
                case identity::wot::claim::ClaimType::Myr:
                case identity::wot::claim::ClaimType::Jpy:
                case identity::wot::claim::ClaimType::Cny:
                case identity::wot::claim::ClaimType::Nzd:
                case identity::wot::claim::ClaimType::Thb:
                case identity::wot::claim::ClaimType::Huf:
                case identity::wot::claim::ClaimType::Aed:
                case identity::wot::claim::ClaimType::Hkd:
                case identity::wot::claim::ClaimType::Mxn:
                case identity::wot::claim::ClaimType::Zar:
                case identity::wot::claim::ClaimType::Php:
                case identity::wot::claim::ClaimType::Sek:
                case identity::wot::claim::ClaimType::Tnbtc:
                case identity::wot::claim::ClaimType::Tnxrp:
                case identity::wot::claim::ClaimType::Tnltx:
                case identity::wot::claim::ClaimType::Tnxem:
                case identity::wot::claim::ClaimType::Tndash:
                case identity::wot::claim::ClaimType::Tnmaid:
                case identity::wot::claim::ClaimType::Tnlsk:
                case identity::wot::claim::ClaimType::Tndoge:
                case identity::wot::claim::ClaimType::Tnxmr:
                case identity::wot::claim::ClaimType::Tnwaves:
                case identity::wot::claim::ClaimType::Tnnxt:
                case identity::wot::claim::ClaimType::Tnsc:
                case identity::wot::claim::ClaimType::Tnsteem:
                case identity::wot::claim::ClaimType::Philosophy:
                case identity::wot::claim::ClaimType::Met:
                case identity::wot::claim::ClaimType::Fan:
                case identity::wot::claim::ClaimType::Supervisor:
                case identity::wot::claim::ClaimType::Subordinate:
                case identity::wot::claim::ClaimType::Contact:
                case identity::wot::claim::ClaimType::Refreshed:
                case identity::wot::claim::ClaimType::Bch:
                case identity::wot::claim::ClaimType::Tnbch:
                case identity::wot::claim::ClaimType::Owner:
                case identity::wot::claim::ClaimType::Property:
                case identity::wot::claim::ClaimType::Unknown:
                case identity::wot::claim::ClaimType::Ethereum_olympic:
                case identity::wot::claim::ClaimType::Ethereum_classic:
                case identity::wot::claim::ClaimType::Ethereum_expanse:
                case identity::wot::claim::ClaimType::Ethereum_morden:
                case identity::wot::claim::ClaimType::Ethereum_ropsten:
                case identity::wot::claim::ClaimType::Ethereum_rinkeby:
                case identity::wot::claim::ClaimType::Ethereum_kovan:
                case identity::wot::claim::ClaimType::Ethereum_sokol:
                case identity::wot::claim::ClaimType::Ethereum_poa:
                case identity::wot::claim::ClaimType::Pkt:
                case identity::wot::claim::ClaimType::Tnpkt:
                case identity::wot::claim::ClaimType::Regtest:
                case identity::wot::claim::ClaimType::Bnb:
                case identity::wot::claim::ClaimType::Sol:
                case identity::wot::claim::ClaimType::Usdt:
                case identity::wot::claim::ClaimType::Ada:
                case identity::wot::claim::ClaimType::Dot:
                case identity::wot::claim::ClaimType::Usdc:
                case identity::wot::claim::ClaimType::Shib:
                case identity::wot::claim::ClaimType::Luna:
                case identity::wot::claim::ClaimType::Avax:
                case identity::wot::claim::ClaimType::Uni:
                case identity::wot::claim::ClaimType::Link:
                case identity::wot::claim::ClaimType::Wbtc:
                case identity::wot::claim::ClaimType::Busd:
                case identity::wot::claim::ClaimType::Matic:
                case identity::wot::claim::ClaimType::Algo:
                case identity::wot::claim::ClaimType::Vet:
                case identity::wot::claim::ClaimType::Axs:
                case identity::wot::claim::ClaimType::Icp:
                case identity::wot::claim::ClaimType::Cro:
                case identity::wot::claim::ClaimType::Atom:
                case identity::wot::claim::ClaimType::Theta:
                case identity::wot::claim::ClaimType::Fil:
                case identity::wot::claim::ClaimType::Trx:
                case identity::wot::claim::ClaimType::Ftt:
                case identity::wot::claim::ClaimType::Etc:
                case identity::wot::claim::ClaimType::Ftm:
                case identity::wot::claim::ClaimType::Dai:
                case identity::wot::claim::ClaimType::Btcb:
                case identity::wot::claim::ClaimType::Egld:
                case identity::wot::claim::ClaimType::Hbar:
                case identity::wot::claim::ClaimType::Xtz:
                case identity::wot::claim::ClaimType::Mana:
                case identity::wot::claim::ClaimType::Near:
                case identity::wot::claim::ClaimType::Grt:
                case identity::wot::claim::ClaimType::Cake:
                case identity::wot::claim::ClaimType::Eos:
                case identity::wot::claim::ClaimType::Flow:
                case identity::wot::claim::ClaimType::Aave:
                case identity::wot::claim::ClaimType::Klay:
                case identity::wot::claim::ClaimType::Ksm:
                case identity::wot::claim::ClaimType::Xec:
                case identity::wot::claim::ClaimType::Miota:
                case identity::wot::claim::ClaimType::Hnt:
                case identity::wot::claim::ClaimType::Rune:
                case identity::wot::claim::ClaimType::Bsv:
                case identity::wot::claim::ClaimType::Leo:
                case identity::wot::claim::ClaimType::Neo:
                case identity::wot::claim::ClaimType::One:
                case identity::wot::claim::ClaimType::Qnt:
                case identity::wot::claim::ClaimType::Ust:
                case identity::wot::claim::ClaimType::Mkr:
                case identity::wot::claim::ClaimType::Enj:
                case identity::wot::claim::ClaimType::Chz:
                case identity::wot::claim::ClaimType::Ar:
                case identity::wot::claim::ClaimType::Stx:
                case identity::wot::claim::ClaimType::Btt:
                case identity::wot::claim::ClaimType::Hot:
                case identity::wot::claim::ClaimType::Sand:
                case identity::wot::claim::ClaimType::Omg:
                case identity::wot::claim::ClaimType::Celo:
                case identity::wot::claim::ClaimType::Zec:
                case identity::wot::claim::ClaimType::Comp:
                case identity::wot::claim::ClaimType::Tfuel:
                case identity::wot::claim::ClaimType::Kda:
                case identity::wot::claim::ClaimType::Lrc:
                case identity::wot::claim::ClaimType::Qtum:
                case identity::wot::claim::ClaimType::Crv:
                case identity::wot::claim::ClaimType::Ht:
                case identity::wot::claim::ClaimType::Nexo:
                case identity::wot::claim::ClaimType::Sushi:
                case identity::wot::claim::ClaimType::Kcs:
                case identity::wot::claim::ClaimType::Bat:
                case identity::wot::claim::ClaimType::Okb:
                case identity::wot::claim::ClaimType::Dcr:
                case identity::wot::claim::ClaimType::Icx:
                case identity::wot::claim::ClaimType::Rvn:
                case identity::wot::claim::ClaimType::Scrt:
                case identity::wot::claim::ClaimType::Rev:
                case identity::wot::claim::ClaimType::Audio:
                case identity::wot::claim::ClaimType::Zil:
                case identity::wot::claim::ClaimType::Tusd:
                case identity::wot::claim::ClaimType::Yfi:
                case identity::wot::claim::ClaimType::Mina:
                case identity::wot::claim::ClaimType::Perp:
                case identity::wot::claim::ClaimType::Xdc:
                case identity::wot::claim::ClaimType::Tel:
                case identity::wot::claim::ClaimType::Snx:
                case identity::wot::claim::ClaimType::Btg:
                case identity::wot::claim::ClaimType::Afn:
                case identity::wot::claim::ClaimType::All:
                case identity::wot::claim::ClaimType::Amd:
                case identity::wot::claim::ClaimType::Ang:
                case identity::wot::claim::ClaimType::Aoa:
                case identity::wot::claim::ClaimType::Ars:
                case identity::wot::claim::ClaimType::Awg:
                case identity::wot::claim::ClaimType::Azn:
                case identity::wot::claim::ClaimType::Bam:
                case identity::wot::claim::ClaimType::Bbd:
                case identity::wot::claim::ClaimType::Bdt:
                case identity::wot::claim::ClaimType::Bgn:
                case identity::wot::claim::ClaimType::Bhd:
                case identity::wot::claim::ClaimType::Bif:
                case identity::wot::claim::ClaimType::Bmd:
                case identity::wot::claim::ClaimType::Bnd:
                case identity::wot::claim::ClaimType::Bob:
                case identity::wot::claim::ClaimType::Brl:
                case identity::wot::claim::ClaimType::Bsd:
                case identity::wot::claim::ClaimType::Btn:
                case identity::wot::claim::ClaimType::Bwp:
                case identity::wot::claim::ClaimType::Byn:
                case identity::wot::claim::ClaimType::Bzd:
                case identity::wot::claim::ClaimType::Cdf:
                case identity::wot::claim::ClaimType::Clp:
                case identity::wot::claim::ClaimType::Cop:
                case identity::wot::claim::ClaimType::Crc:
                case identity::wot::claim::ClaimType::Cuc:
                case identity::wot::claim::ClaimType::Cup:
                case identity::wot::claim::ClaimType::Cve:
                case identity::wot::claim::ClaimType::Czk:
                case identity::wot::claim::ClaimType::Djf:
                case identity::wot::claim::ClaimType::Dkk:
                case identity::wot::claim::ClaimType::Dop:
                case identity::wot::claim::ClaimType::Dzd:
                case identity::wot::claim::ClaimType::Egp:
                case identity::wot::claim::ClaimType::Ern:
                case identity::wot::claim::ClaimType::Etb:
                case identity::wot::claim::ClaimType::Fjd:
                case identity::wot::claim::ClaimType::Fkp:
                case identity::wot::claim::ClaimType::Gel:
                case identity::wot::claim::ClaimType::Ggp:
                case identity::wot::claim::ClaimType::Ghs:
                case identity::wot::claim::ClaimType::Gip:
                case identity::wot::claim::ClaimType::Gmd:
                case identity::wot::claim::ClaimType::Gnf:
                case identity::wot::claim::ClaimType::Gtq:
                case identity::wot::claim::ClaimType::Gyd:
                case identity::wot::claim::ClaimType::Hnl:
                case identity::wot::claim::ClaimType::Hrk:
                case identity::wot::claim::ClaimType::Htg:
                case identity::wot::claim::ClaimType::Idr:
                case identity::wot::claim::ClaimType::Ils:
                case identity::wot::claim::ClaimType::Imp:
                case identity::wot::claim::ClaimType::Iqd:
                case identity::wot::claim::ClaimType::Irr:
                case identity::wot::claim::ClaimType::Isk:
                case identity::wot::claim::ClaimType::Jep:
                case identity::wot::claim::ClaimType::Jmd:
                case identity::wot::claim::ClaimType::Jod:
                case identity::wot::claim::ClaimType::Kes:
                case identity::wot::claim::ClaimType::Kgs:
                case identity::wot::claim::ClaimType::Khr:
                case identity::wot::claim::ClaimType::Kmf:
                case identity::wot::claim::ClaimType::Kpw:
                case identity::wot::claim::ClaimType::Krw:
                case identity::wot::claim::ClaimType::Kwd:
                case identity::wot::claim::ClaimType::Kyd:
                case identity::wot::claim::ClaimType::Kzt:
                case identity::wot::claim::ClaimType::Lak:
                case identity::wot::claim::ClaimType::Lbp:
                case identity::wot::claim::ClaimType::Lkr:
                case identity::wot::claim::ClaimType::Lrd:
                case identity::wot::claim::ClaimType::Lsl:
                case identity::wot::claim::ClaimType::Lyd:
                case identity::wot::claim::ClaimType::Mad:
                case identity::wot::claim::ClaimType::Mdl:
                case identity::wot::claim::ClaimType::Mga:
                case identity::wot::claim::ClaimType::Mkd:
                case identity::wot::claim::ClaimType::Mmk:
                case identity::wot::claim::ClaimType::Mnt:
                case identity::wot::claim::ClaimType::Mop:
                case identity::wot::claim::ClaimType::Mru:
                case identity::wot::claim::ClaimType::Mur:
                case identity::wot::claim::ClaimType::Mvr:
                case identity::wot::claim::ClaimType::Mwk:
                case identity::wot::claim::ClaimType::Mzn:
                case identity::wot::claim::ClaimType::Nad:
                case identity::wot::claim::ClaimType::Ngn:
                case identity::wot::claim::ClaimType::Nio:
                case identity::wot::claim::ClaimType::Nok:
                case identity::wot::claim::ClaimType::Npr:
                case identity::wot::claim::ClaimType::Omr:
                case identity::wot::claim::ClaimType::Pab:
                case identity::wot::claim::ClaimType::Pen:
                case identity::wot::claim::ClaimType::Pgk:
                case identity::wot::claim::ClaimType::Pkr:
                case identity::wot::claim::ClaimType::Pln:
                case identity::wot::claim::ClaimType::Pyg:
                case identity::wot::claim::ClaimType::Qar:
                case identity::wot::claim::ClaimType::Ron:
                case identity::wot::claim::ClaimType::Rsd:
                case identity::wot::claim::ClaimType::Rub:
                case identity::wot::claim::ClaimType::Rwf:
                case identity::wot::claim::ClaimType::Sar:
                case identity::wot::claim::ClaimType::Sbd:
                case identity::wot::claim::ClaimType::Scr:
                case identity::wot::claim::ClaimType::Sdg:
                case identity::wot::claim::ClaimType::Shp:
                case identity::wot::claim::ClaimType::Sll:
                case identity::wot::claim::ClaimType::Sos:
                case identity::wot::claim::ClaimType::Spl:
                case identity::wot::claim::ClaimType::Srd:
                case identity::wot::claim::ClaimType::Stn:
                case identity::wot::claim::ClaimType::Svc:
                case identity::wot::claim::ClaimType::Syp:
                case identity::wot::claim::ClaimType::Szl:
                case identity::wot::claim::ClaimType::Tjs:
                case identity::wot::claim::ClaimType::Tmt:
                case identity::wot::claim::ClaimType::Tnd:
                case identity::wot::claim::ClaimType::Top:
                case identity::wot::claim::ClaimType::Try:
                case identity::wot::claim::ClaimType::Ttd:
                case identity::wot::claim::ClaimType::Tvd:
                case identity::wot::claim::ClaimType::Twd:
                case identity::wot::claim::ClaimType::Tzs:
                case identity::wot::claim::ClaimType::Uah:
                case identity::wot::claim::ClaimType::Ugx:
                case identity::wot::claim::ClaimType::Uyu:
                case identity::wot::claim::ClaimType::Uzs:
                case identity::wot::claim::ClaimType::Vef:
                case identity::wot::claim::ClaimType::Vnd:
                case identity::wot::claim::ClaimType::Vuv:
                case identity::wot::claim::ClaimType::Wst:
                case identity::wot::claim::ClaimType::Xaf:
                case identity::wot::claim::ClaimType::Xcd:
                case identity::wot::claim::ClaimType::Xdr:
                case identity::wot::claim::ClaimType::Xof:
                case identity::wot::claim::ClaimType::Xpf:
                case identity::wot::claim::ClaimType::Yer:
                case identity::wot::claim::ClaimType::Zmw:
                case identity::wot::claim::ClaimType::Zwd:
                case identity::wot::claim::ClaimType::Custom:
                case identity::wot::claim::ClaimType::Tnbsv:
                case identity::wot::claim::ClaimType::TnXec:
                default: {
                }
            }
        }
    }
}

auto Contacts::init(const std::shared_ptr<const crypto::Blockchain>& blockchain)
    -> void
{
    OT_ASSERT(blockchain);

    blockchain_ = blockchain;

    OT_ASSERT(false == blockchain_.expired());
}

void Contacts::init_nym_map(const rLock& lock)
{
    LogDetail()(OT_PRETTY_CLASS())("Upgrading indices.").Flush();

    for (const auto& it : api_.Storage().ContactList()) {
        const auto& contactID = api_.Factory().IdentifierFromBase58(it.first);
        auto loaded = load_contact(lock, contactID);

        if (contact_map_.end() == loaded) {

            throw std::runtime_error("failed to load contact");
        }

        auto& contact = loaded->second.second;

        if (false == bool(contact)) {

            throw std::runtime_error("null contact pointer");
        }

        const auto type = contact->Type();

        if (identity::wot::claim::ClaimType::Error == type) {
            LogError()(OT_PRETTY_CLASS())("Invalid contact ")(it.first)(".")
                .Flush();
            api_.Storage().DeleteContact(it.first);
        }

        const auto nyms = contact->Nyms();

        for (const auto& nym : nyms) { update_nym_map(lock, nym, *contact); }
    }

    api_.Storage().ContactSaveIndices();
}

auto Contacts::load_contact(const rLock& lock, const identifier::Generic& id)
    const -> Contacts::ContactMap::iterator
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    auto serialized = proto::Contact{};
    const auto loaded =
        api_.Storage().Load(id.asBase58(api_.Crypto()), serialized, SILENT);

    if (false == loaded) {
        LogDetail()(OT_PRETTY_CLASS())("Unable to load contact ")(
            id, api_.Crypto())
            .Flush();

        return contact_map_.end();
    }

    auto contact = std::make_unique<opentxs::Contact>(api_, serialized);

    if (false == bool(contact)) {
        LogError()(OT_PRETTY_CLASS())(
            ": Unable to instantate serialized contact.")
            .Flush();

        return contact_map_.end();
    }

    return add_contact(lock, contact.release());
}

auto Contacts::Merge(
    const identifier::Generic& parent,
    const identifier::Generic& child) const
    -> std::shared_ptr<const opentxs::Contact>
{
    auto lock = rLock{lock_};
    auto childContact = contact(lock, child);

    if (false == bool(childContact)) {
        LogError()(OT_PRETTY_CLASS())("Child contact ")(child, api_.Crypto())(
            " can not be loaded.")
            .Flush();

        return {};
    }

    const auto& childID = childContact->ID();

    if (childID != child) {
        LogError()(OT_PRETTY_CLASS())("Child contact ")(child, api_.Crypto())(
            " is already merged into ")(childID, api_.Crypto())(".")
            .Flush();

        return {};
    }

    auto parentContact = contact(lock, parent);

    if (false == bool(parentContact)) {
        LogError()(OT_PRETTY_CLASS())("Parent contact ")(parent, api_.Crypto())(
            " can not be loaded.")
            .Flush();

        return {};
    }

    const auto& parentID = parentContact->ID();

    if (parentID != parent) {
        LogError()(OT_PRETTY_CLASS())("Parent contact ")(parent, api_.Crypto())(
            " is merged into ")(parentID, api_.Crypto())(".")
            .Flush();

        return {};
    }

    OT_ASSERT(childContact);
    OT_ASSERT(parentContact);

    auto& lhs = const_cast<opentxs::Contact&>(*parentContact);
    auto& rhs = const_cast<opentxs::Contact&>(*childContact);
    lhs += rhs;
    const auto lProto = [&] {
        auto out = proto::Contact{};
        lhs.Serialize(out);
        return out;
    }();
    const auto rProto = [&] {
        auto out = proto::Contact{};
        rhs.Serialize(out);
        return out;
    }();

    if (false == api_.Storage().Store(rProto)) {
        LogError()(OT_PRETTY_CLASS())(": Unable to create save child contact.")
            .Flush();

        OT_FAIL;
    }

    if (false == api_.Storage().Store(lProto)) {
        LogError()(OT_PRETTY_CLASS())(": Unable to create save parent contact.")
            .Flush();

        OT_FAIL;
    }

    contact_map_.erase(child);
    auto blockchain = blockchain_.lock();

    if (blockchain) {
        blockchain->Internal().ProcessMergedContact(lhs, rhs);
    } else {
        LogVerbose()(OT_PRETTY_CLASS())(
            ": Warning: contact not updated in blockchain API")
            .Flush();
    }

    return parentContact;
}

auto Contacts::mutable_contact(const rLock& lock, const identifier::Generic& id)
    const -> std::unique_ptr<Editor<opentxs::Contact>>
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    std::unique_ptr<Editor<opentxs::Contact>> output{nullptr};

    auto it = contact_map_.find(id);

    if (contact_map_.end() == it) { it = load_contact(lock, id); }

    if (contact_map_.end() == it) { return {}; }

    std::function<void(opentxs::Contact*)> callback =
        [&](opentxs::Contact* in) -> void { this->save(in); };
    output = std::make_unique<Editor<opentxs::Contact>>(
        it->second.second.get(), callback);

    return output;
}

auto Contacts::mutable_Contact(const identifier::Generic& id) const
    -> std::unique_ptr<Editor<opentxs::Contact>>
{
    auto lock = rLock{lock_};
    auto output = mutable_contact(lock, id);
    lock.unlock();

    return output;
}

auto Contacts::new_contact(
    const rLock& lock,
    std::string_view label,
    const identifier::Nym& nymID,
    const PaymentCode& code) const -> std::shared_ptr<const opentxs::Contact>
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    bool haveNymID{false};
    bool havePaymentCode{false};
    auto inputNymID = identifier::Nym{};
    check_identifiers(nymID, code, haveNymID, havePaymentCode, inputNymID);

    if (haveNymID) {
        const auto contactID = api_.Storage().ContactOwnerNym(inputNymID);

        if (false == contactID.empty()) {

            return update_existing_contact(lock, label, code, contactID);
        }
    }

    auto newContact = contact(lock, label);

    if (false == bool(newContact)) { return {}; }

    identifier::Generic contactID = newContact->ID();
    newContact.reset();
    auto output = mutable_contact(lock, contactID);

    if (false == bool(output)) { return {}; }

    auto& mContact = output->get();

    if (false == inputNymID.empty()) {
        auto nym = api_.Wallet().Nym(inputNymID);

        if (nym) {
            mContact.AddNym(nym, true);
        } else {
            mContact.AddNym(inputNymID, true);
        }

        update_nym_map(lock, inputNymID, mContact, true);
    }

    if (code.Valid()) { mContact.AddPaymentCode(code, true); }

    output.reset();

    return contact(lock, contactID);
}

auto Contacts::NewContact(const UnallocatedCString& label) const
    -> std::shared_ptr<const opentxs::Contact>
{
    auto lock = rLock{lock_};

    return contact(lock, label);
}

auto Contacts::NewContact(
    const UnallocatedCString& label,
    const identifier::Nym& nymID,
    const PaymentCode& paymentCode) const
    -> std::shared_ptr<const opentxs::Contact>
{
    auto lock = rLock{lock_};

    return new_contact(lock, label, nymID, paymentCode);
}

auto Contacts::NewContactFromAddress(
    const UnallocatedCString& address,
    const UnallocatedCString& label,
    const opentxs::blockchain::Type currency) const
    -> std::shared_ptr<const opentxs::Contact>
{
    auto blockchain = blockchain_.lock();

    if (false == bool(blockchain)) {
        LogVerbose()(OT_PRETTY_CLASS())("shutting down ").Flush();

        return {};
    }

    auto lock = rLock{lock_};
    const auto existing = blockchain->LookupContacts(address);

    switch (existing.size()) {
        case 0: {
        } break;
        case 1: {
            return contact(lock, *existing.cbegin());
        }
        default: {
            LogError()(OT_PRETTY_CLASS())(": multiple contacts claim address ")(
                address)
                .Flush();

            return {};
        }
    }

    auto newContact = contact(lock, label);

    OT_ASSERT(newContact);

    auto& it = contact_map_.at(newContact->ID());
    auto& contact = *it.second;

    if (false == contact.AddBlockchainAddress(address, currency)) {
        LogError()(OT_PRETTY_CLASS())(": Failed to add address to contact.")
            .Flush();

        OT_FAIL;
    }

    const auto proto = [&] {
        auto out = proto::Contact{};
        contact.Serialize(out);
        return out;
    }();

    if (false == api_.Storage().Store(proto)) {
        LogError()(OT_PRETTY_CLASS())("Unable to save contact.").Flush();

        OT_FAIL;
    }

    blockchain->Internal().ProcessContact(contact);

    return newContact;
}

auto Contacts::NymToContact(const identifier::Nym& nymID) const
    -> identifier::Generic
{
    const auto contactID = ContactID(nymID);

    if (false == contactID.empty()) { return contactID; }

    // Contact does not yet exist. Create it.
    UnallocatedCString label{""};
    auto nym = api_.Wallet().Nym(nymID);
    auto code = api_.Factory().PaymentCodeFromBase58(UnallocatedCString{});

    if (nym) {
        label = nym->Claims().Name();
        code = api_.Factory().PaymentCodeFromBase58(nym->PaymentCode());
    }

    const auto contact = NewContact(label, nymID, code);

    if (contact) { return contact->ID(); }

    static const auto blank = identifier::Generic{};

    return blank;
}

auto Contacts::obtain_contact(const rLock& lock, const identifier::Generic& id)
    const -> Contacts::ContactMap::iterator
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    auto it = contact_map_.find(id);

    if (contact_map_.end() != it) { return it; }

    return load_contact(lock, id);
}

auto Contacts::PaymentCodeToContact(
    const UnallocatedCString& serialized,
    const opentxs::blockchain::Type currency) const -> identifier::Generic
{
    static const auto blank = identifier::Generic{};
    const auto code = api_.Factory().PaymentCodeFromBase58(serialized);

    if (0 == code.Version()) { return blank; }

    return PaymentCodeToContact(code, currency);
}

auto Contacts::PaymentCodeToContact(
    const PaymentCode& code,
    const opentxs::blockchain::Type currency) const -> identifier::Generic
{
    // NOTE for now we assume that payment codes are always nym id sources. This
    // won't always be true.
    auto lock = rLock{lock_};
    const auto& nymID = code.ID();
    const auto contactID = [&]() -> identifier::Generic {
        auto id = ContactID(nymID);

        if (false == id.empty()) { return id; }

        auto nym = api_.Wallet().Nym(nymID);
        auto label = code.asBase58();

        if (nym) {
            auto name = nym->Claims().Name();

            if (false == name.empty()) { label = std::move(name); }
        }

        auto contact = new_contact(lock, label, nymID, code);

        if (contact) {

            return contact->ID();
        } else {

            return {};
        }
    }();

    if (contactID.empty()) {

        return {};
    } else {
        {
            auto contactE = mutable_contact(lock, contactID);
            auto& c = contactE->get();
            const auto chain = BlockchainToUnit(currency);
            const auto existing = c.PaymentCode(chain);
            c.AddPaymentCode(code, existing.empty(), chain);
        }

        return contactID;
    }
}

auto Contacts::pipeline(opentxs::network::zeromq::Message&& in) noexcept -> void
{
    const auto body = in.Payload();

    if (1 > body.size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid message").Flush();

        OT_FAIL;
    }

    const auto work = [&] {
        try {

            return body[0].as<Work>();
        } catch (...) {

            OT_FAIL;
        }
    }();
    switch (work) {
        case Work::shutdown: {
            pipeline_.Close();
        } break;
        case Work::nymcreated:
        case Work::nymupdated: {
            OT_ASSERT(1 < body.size());

            const auto id = [&] {
                auto out = identifier::Nym{};
                out.Assign(body[1].Bytes());

                return out;
            }();
            const auto nym = api_.Wallet().Nym(id);

            OT_ASSERT(nym);

            update(*nym);
        } break;
        case Work::refresh: {
            check_nyms();
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())("Unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto Contacts::refresh_indices(const rLock& lock, opentxs::Contact& contact)
    const -> void
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    const auto nyms = contact.Nyms();

    for (const auto& nymid : nyms) {
        update_nym_map(lock, nymid, contact, true);
    }

    const auto& id = contact.ID();
    contact_name_map_[id] = contact.Label();
    publisher_->Send([&] {
        auto work = opentxs::network::zeromq::tagged_message(
            WorkType::ContactUpdated, true);
        work.AddFrame(id);

        return work;
    }());
}

auto Contacts::refresh_nyms() noexcept -> void
{
    static constexpr auto interval = std::chrono::minutes{5};
    timer_.SetRelative(interval);
    timer_.Wait([this](const auto& ec) {
        if (ec) {
            if (unexpected_asio_error(ec)) {
                LogError()(OT_PRETTY_CLASS())("received asio error (")(
                    ec.value())(") :")(ec)
                    .Flush();
            }
        } else {
            pipeline_.Push(
                opentxs::network::zeromq::tagged_message(Work::refresh, true));
            refresh_nyms();
        }
    });
}

void Contacts::save(opentxs::Contact* contact) const
{
    OT_ASSERT(nullptr != contact);

    const auto proto = [&] {
        auto out = proto::Contact{};
        contact->Serialize(out);
        return out;
    }();

    if (false == api_.Storage().Store(proto)) {
        LogError()(OT_PRETTY_CLASS())(": Unable to create or save contact.")
            .Flush();

        OT_FAIL;
    }

    const auto& id = contact->ID();

    if (false == api_.Storage().SetContactAlias(
                     id.asBase58(api_.Crypto()), contact->Label())) {
        LogError()(OT_PRETTY_CLASS())(": Unable to create or save contact.")
            .Flush();

        OT_FAIL;
    }

    auto lock = rLock{lock_};
    refresh_indices(lock, *contact);
    auto blockchain = blockchain_.lock();

    if (blockchain) {
        blockchain->Internal().ProcessContact(*contact);
    } else {
        LogVerbose()(OT_PRETTY_CLASS())(
            ": Warning: contact not updated in blockchain API")
            .Flush();
    }
}

void Contacts::start()
{
    const auto level = api_.Storage().ContactUpgradeLevel();

    switch (level) {
        case 0:
        case 1: {
            auto lock = rLock{lock_};
            init_nym_map(lock);
            import_contacts(lock);
            [[fallthrough]];
        }
        case 2:
        default: {
        }
    }
}

auto Contacts::update(const identity::Nym& nym) const
    -> std::shared_ptr<const opentxs::Contact>
{
    using enum identity::wot::claim::ClaimType;
    const auto& data = nym.Claims();

    switch (data.Type()) {
        case Individual:
        case Organization:
        case Business:
        case Government:
        case Bot: {
        } break;
        case Error:
        case Server:
        case Prefix:
        case Forename:
        case Middlename:
        case Surname:
        case Pedigree:
        case Suffix:
        case Nickname:
        case Commonname:
        case Passport:
        case National:
        case Provincial:
        case Military:
        case Pgp:
        case Otr:
        case Ssl:
        case Physical:
        case Official:
        case Birthplace:
        case Home:
        case Website:
        case Opentxs:
        case Phone:
        case Email:
        case Skype:
        case Wire:
        case Qq:
        case Bitmessage:
        case Whatsapp:
        case Telegram:
        case Kik:
        case Bbm:
        case Wechat:
        case Kakaotalk:
        case Facebook:
        case Google:
        case Linkedin:
        case Vk:
        case Aboutme:
        case Onename:
        case Twitter:
        case Medium:
        case Tumblr:
        case Yahoo:
        case Myspace:
        case Meetup:
        case Reddit:
        case Hackernews:
        case Wikipedia:
        case Angellist:
        case Github:
        case Bitbucket:
        case Youtube:
        case Vimeo:
        case Twitch:
        case Snapchat:
        case Vine:
        case Instagram:
        case Pinterest:
        case Imgur:
        case Flickr:
        case Dribble:
        case Behance:
        case Deviantart:
        case Spotify:
        case Itunes:
        case Soundcloud:
        case Askfm:
        case Ebay:
        case Etsy:
        case Openbazaar:
        case Xboxlive:
        case Playstation:
        case Secondlife:
        case Warcraft:
        case Alias:
        case Acquaintance:
        case Friend:
        case Spouse:
        case Sibling:
        case Member:
        case Colleague:
        case Parent:
        case Child:
        case Employer:
        case Employee:
        case Citizen:
        case Photo:
        case Gender:
        case Height:
        case Weight:
        case Hair:
        case Eye:
        case Skin:
        case Ethnicity:
        case Language:
        case Degree:
        case Certification:
        case Title:
        case Skill:
        case Award:
        case Likes:
        case Sexual:
        case Political:
        case Religious:
        case Birth:
        case Secondarygraduation:
        case Universitygraduation:
        case Wedding:
        case Accomplishment:
        case Btc:
        case Eth:
        case Xrp:
        case Ltc:
        case Dao:
        case Xem:
        case Dash:
        case Maid:
        case Lsk:
        case Doge:
        case Dgd:
        case Xmr:
        case Waves:
        case Nxt:
        case Sc:
        case Steem:
        case Amp:
        case Xlm:
        case Fct:
        case Bts:
        case Usd:
        case Eur:
        case Gbp:
        case Inr:
        case Aud:
        case Cad:
        case Sgd:
        case Chf:
        case Myr:
        case Jpy:
        case Cny:
        case Nzd:
        case Thb:
        case Huf:
        case Aed:
        case Hkd:
        case Mxn:
        case Zar:
        case Php:
        case Sek:
        case Tnbtc:
        case Tnxrp:
        case Tnltx:
        case Tnxem:
        case Tndash:
        case Tnmaid:
        case Tnlsk:
        case Tndoge:
        case Tnxmr:
        case Tnwaves:
        case Tnnxt:
        case Tnsc:
        case Tnsteem:
        case Philosophy:
        case Met:
        case Fan:
        case Supervisor:
        case Subordinate:
        case Contact:
        case Refreshed:
        case Bch:
        case Tnbch:
        case Owner:
        case Property:
        case Unknown:
        case Ethereum_olympic:
        case Ethereum_classic:
        case Ethereum_expanse:
        case Ethereum_morden:
        case Ethereum_ropsten:
        case Ethereum_rinkeby:
        case Ethereum_kovan:
        case Ethereum_sokol:
        case Ethereum_poa:
        case Pkt:
        case Tnpkt:
        case Regtest:
        case Bnb:
        case Sol:
        case Usdt:
        case Ada:
        case Dot:
        case Usdc:
        case Shib:
        case Luna:
        case Avax:
        case Uni:
        case Link:
        case Wbtc:
        case Busd:
        case Matic:
        case Algo:
        case Vet:
        case Axs:
        case Icp:
        case Cro:
        case Atom:
        case Theta:
        case Fil:
        case Trx:
        case Ftt:
        case Etc:
        case Ftm:
        case Dai:
        case Btcb:
        case Egld:
        case Hbar:
        case Xtz:
        case Mana:
        case Near:
        case Grt:
        case Cake:
        case Eos:
        case Flow:
        case Aave:
        case Klay:
        case Ksm:
        case Xec:
        case Miota:
        case Hnt:
        case Rune:
        case Bsv:
        case Leo:
        case Neo:
        case One:
        case Qnt:
        case Ust:
        case Mkr:
        case Enj:
        case Chz:
        case Ar:
        case Stx:
        case Btt:
        case Hot:
        case Sand:
        case Omg:
        case Celo:
        case Zec:
        case Comp:
        case Tfuel:
        case Kda:
        case Lrc:
        case Qtum:
        case Crv:
        case Ht:
        case Nexo:
        case Sushi:
        case Kcs:
        case Bat:
        case Okb:
        case Dcr:
        case Icx:
        case Rvn:
        case Scrt:
        case Rev:
        case Audio:
        case Zil:
        case Tusd:
        case Yfi:
        case Mina:
        case Perp:
        case Xdc:
        case Tel:
        case Snx:
        case Btg:
        case Afn:
        case All:
        case Amd:
        case Ang:
        case Aoa:
        case Ars:
        case Awg:
        case Azn:
        case Bam:
        case Bbd:
        case Bdt:
        case Bgn:
        case Bhd:
        case Bif:
        case Bmd:
        case Bnd:
        case Bob:
        case Brl:
        case Bsd:
        case Btn:
        case Bwp:
        case Byn:
        case Bzd:
        case Cdf:
        case Clp:
        case Cop:
        case Crc:
        case Cuc:
        case Cup:
        case Cve:
        case Czk:
        case Djf:
        case Dkk:
        case Dop:
        case Dzd:
        case Egp:
        case Ern:
        case Etb:
        case Fjd:
        case Fkp:
        case Gel:
        case Ggp:
        case Ghs:
        case Gip:
        case Gmd:
        case Gnf:
        case Gtq:
        case Gyd:
        case Hnl:
        case Hrk:
        case Htg:
        case Idr:
        case Ils:
        case Imp:
        case Iqd:
        case Irr:
        case Isk:
        case Jep:
        case Jmd:
        case Jod:
        case Kes:
        case Kgs:
        case Khr:
        case Kmf:
        case Kpw:
        case Krw:
        case Kwd:
        case Kyd:
        case Kzt:
        case Lak:
        case Lbp:
        case Lkr:
        case Lrd:
        case Lsl:
        case Lyd:
        case Mad:
        case Mdl:
        case Mga:
        case Mkd:
        case Mmk:
        case Mnt:
        case Mop:
        case Mru:
        case Mur:
        case Mvr:
        case Mwk:
        case Mzn:
        case Nad:
        case Ngn:
        case Nio:
        case Nok:
        case Npr:
        case Omr:
        case Pab:
        case Pen:
        case Pgk:
        case Pkr:
        case Pln:
        case Pyg:
        case Qar:
        case Ron:
        case Rsd:
        case Rub:
        case Rwf:
        case Sar:
        case Sbd:
        case Scr:
        case Sdg:
        case Shp:
        case Sll:
        case Sos:
        case Spl:
        case Srd:
        case Stn:
        case Svc:
        case Syp:
        case Szl:
        case Tjs:
        case Tmt:
        case Tnd:
        case Top:
        case Try:
        case Ttd:
        case Tvd:
        case Twd:
        case Tzs:
        case Uah:
        case Ugx:
        case Uyu:
        case Uzs:
        case Vef:
        case Vnd:
        case Vuv:
        case Wst:
        case Xaf:
        case Xcd:
        case Xdr:
        case Xof:
        case Xpf:
        case Yer:
        case Zmw:
        case Zwd:
        case Custom:
        case Tnbsv:
        case TnXec:
        default: {
            return {};
        }
    }

    const auto& nymID = nym.ID();
    auto lock = rLock{lock_};
    const auto contactID = api_.Storage().ContactOwnerNym(nymID);
    const auto label = Contact::ExtractLabel(nym);

    if (contactID.empty()) {
        LogDetail()(OT_PRETTY_CLASS())("Nym ")(nymID, api_.Crypto())(
            " is not associated with a contact. Creating a new contact "
            "named ")(label)
            .Flush();
        auto code = api_.Factory().PaymentCodeFromBase58(nym.PaymentCode());

        return new_contact(lock, label, nymID, code);
    }

    {
        auto contact = mutable_contact(lock, contactID);
        auto serialized = proto::Nym{};

        if (false == nym.Internal().Serialize(serialized)) {
            LogError()(OT_PRETTY_CLASS())("Failed to serialize nym.").Flush();

            return {};
        }

        contact->get().Update(serialized);
        const auto name = nym.Name();

        if (false == name.empty()) { contact->get().SetLabel(name); }

        contact.reset();
    }

    auto contact = obtain_contact(lock, contactID);

    OT_ASSERT(contact_map_.end() != contact);

    auto& output = contact->second.second;

    OT_ASSERT(output);

    api_.Storage().RelabelThread(
        output->ID().asBase58(api_.Crypto()), output->Label());

    return output;
}

auto Contacts::update_existing_contact(
    const rLock& lock,
    std::string_view label,
    const PaymentCode& code,
    const identifier::Generic& contactID) const
    -> std::shared_ptr<const opentxs::Contact>
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    auto it = obtain_contact(lock, contactID);

    OT_ASSERT(contact_map_.end() != it);

    auto& contactMutex = it->second.first;
    auto& contact = it->second.second;

    OT_ASSERT(contact);

    Lock contactLock(contactMutex);
    const auto& existingLabel = contact->Label();

    if ((existingLabel != label) && (false == label.empty())) {
        contact->SetLabel(label);
    }

    contact->AddPaymentCode(code, true);
    save(contact.get());

    return contact;
}

void Contacts::update_nym_map(
    const rLock& lock,
    const identifier::Nym& nymID,
    opentxs::Contact& contact,
    const bool replace) const
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    const auto contactID = api_.Storage().ContactOwnerNym(nymID);
    const bool exists = (false == contactID.empty());
    const auto& incomingID = contact.ID();
    const bool same = (incomingID == contactID);

    if (exists && (false == same)) {
        if (replace) {
            auto it = load_contact(lock, contactID);

            if (contact_map_.end() != it) {

                throw std::runtime_error("contact not found");
            }

            auto& oldContact = it->second.second;

            if (false == bool(oldContact)) {
                throw std::runtime_error("null contact pointer");
            }

            oldContact->RemoveNym(nymID);
            const auto proto = [&] {
                auto out = proto::Contact{};
                oldContact->Serialize(out);
                return out;
            }();

            if (false == api_.Storage().Store(proto)) {
                LogError()(OT_PRETTY_CLASS())(
                    ": Unable to create or save contact.")
                    .Flush();

                OT_FAIL;
            }
        } else {
            LogError()(OT_PRETTY_CLASS())("Duplicate nym found.").Flush();
            contact.RemoveNym(nymID);
            const auto proto = [&] {
                auto out = proto::Contact{};
                contact.Serialize(out);
                return out;
            }();

            if (false == api_.Storage().Store(proto)) {
                LogError()(OT_PRETTY_CLASS())(
                    ": Unable to create or save contact.")
                    .Flush();

                OT_FAIL;
            }
        }
    }

    auto blockchain = blockchain_.lock();

    if (blockchain) {
        blockchain->Internal().ProcessContact(contact);
    } else {
        LogVerbose()(OT_PRETTY_CLASS())(
            ": Warning: contact not updated in blockchain API")
            .Flush();
    }
}

auto Contacts::verify_write_lock(const rLock& lock) const -> bool
{
    if (lock.mutex() != &lock_) {
        LogError()(OT_PRETTY_CLASS())("Incorrect mutex.").Flush();

        return false;
    }

    if (false == lock.owns_lock()) {
        LogError()(OT_PRETTY_CLASS())("Lock not owned.").Flush();

        return false;
    }

    return true;
}

Contacts::~Contacts()
{
    timer_.Cancel();
    pipeline_.Close();
}
}  // namespace opentxs::api::session::imp
