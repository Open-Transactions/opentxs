// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "interface/ui/profile/Profile.hpp"  // IWYU pragma: associated

#include <functional>
#include <memory>
#include <string_view>
#include <thread>
#include <tuple>
#include <utility>

#include "interface/ui/base/List.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/interface/ui/Profile.hpp"
#include "internal/interface/ui/ProfileSection.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/wot/claim/Attribute.hpp"
#include "opentxs/identity/wot/claim/Data.hpp"
#include "opentxs/identity/wot/claim/Section.hpp"
#include "opentxs/identity/wot/claim/SectionType.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/NymEditor.hpp"
#include "opentxs/util/PasswordPrompt.hpp"  // IWYU pragma: keep

// NOLINTBEGIN(cert-dcl58-cpp)
template struct std::pair<int, opentxs::UnallocatedCString>;
// NOLINTEND(cert-dcl58-cpp)

namespace opentxs::factory
{
auto ProfileModel(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept -> std::unique_ptr<ui::internal::Profile>
{
    using ReturnType = ui::implementation::Profile;

    return std::make_unique<ReturnType>(api, nymID, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
const UnallocatedSet<identity::wot::claim::SectionType> Profile::allowed_types_{
    identity::wot::claim::SectionType::Communication,
    identity::wot::claim::SectionType::Profile};

const UnallocatedMap<identity::wot::claim::SectionType, int>
    Profile::sort_keys_{
        {identity::wot::claim::SectionType::Communication, 0},
        {identity::wot::claim::SectionType::Profile, 1}};

Profile::Profile(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    : ProfileList(api, nymID, cb, false)
    , api_(api)
    , listeners_({
          {api_.Endpoints().NymDownload().data(),
           new MessageProcessor<Profile>(&Profile::process_nym)},
      })
    , callbacks_()
    , name_(nym_name(api_.Crypto(), api_.Wallet(), nymID))
    , payment_code_()
{
    setup_listeners(api_, listeners_);
    startup_ = std::make_unique<std::thread>(&Profile::startup, this);

    OT_ASSERT(startup_);
}

auto Profile::AddClaim(
    const identity::wot::claim::SectionType section,
    const identity::wot::claim::ClaimType type,
    const UnallocatedCString& value,
    const bool primary,
    const bool active) const noexcept -> bool
{
    auto reason = api_.Factory().PasswordPrompt("Adding a claim to nym");
    auto nym = api_.Wallet().mutable_Nym(primary_id_, reason);

    switch (section) {
        case identity::wot::claim::SectionType::Scope: {

            return nym.SetScope(type, value, primary, reason);
        }
        case identity::wot::claim::SectionType::Communication: {
            switch (type) {
                case identity::wot::claim::ClaimType::Email: {

                    return nym.AddEmail(value, primary, active, reason);
                }
                case identity::wot::claim::ClaimType::Phone: {

                    return nym.AddPhoneNumber(value, primary, active, reason);
                }
                case identity::wot::claim::ClaimType::Opentxs: {

                    return nym.AddPreferredOTServer(value, primary, reason);
                }
                case identity::wot::claim::ClaimType::Error:
                case identity::wot::claim::ClaimType::Individual:
                case identity::wot::claim::ClaimType::Organization:
                case identity::wot::claim::ClaimType::Business:
                case identity::wot::claim::ClaimType::Government:
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
                case identity::wot::claim::ClaimType::Bot:
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
        } break;
        case identity::wot::claim::SectionType::Profile: {

            return nym.AddSocialMediaProfile(
                value, type, primary, active, reason);
        }
        case identity::wot::claim::SectionType::Contract: {

            return nym.AddContract(
                value, ClaimToUnit(type), primary, active, reason);
        }
        case identity::wot::claim::SectionType::Procedure: {
            return nym.AddPaymentCode(
                value, ClaimToUnit(type), primary, active, reason);
        }
        default: {
        }
    }

    Claim claim{};
    auto& [id, claimSection, claimType, claimValue, start, end, attributes] =
        claim;
    id = "";
    claimSection = translate(section);
    claimType = translate(type);
    claimValue = value;
    start = {};
    end = {};

    if (primary) {
        attributes.emplace(translate(identity::wot::claim::Attribute::Primary));
    }

    if (primary || active) {
        attributes.emplace(translate(identity::wot::claim::Attribute::Active));
    }

    return nym.AddClaim(claim, reason);
}

auto Profile::AllowedItems(
    const identity::wot::claim::SectionType section,
    const UnallocatedCString& lang) const noexcept -> Profile::ItemTypeList
{
    return ui::ProfileSection::AllowedItems(section, lang);
}

auto Profile::AllowedSections(const UnallocatedCString& lang) const noexcept
    -> Profile::SectionTypeList
{
    SectionTypeList output{};

    for (const auto& type : allowed_types_) {
        output.emplace_back(
            type, proto::TranslateSectionName(translate(type), lang));
    }

    return output;
}

auto Profile::check_type(const identity::wot::claim::SectionType type) noexcept
    -> bool
{
    return 1 == allowed_types_.count(type);
}

auto Profile::ClearCallbacks() const noexcept -> void
{
    Widget::ClearCallbacks();
    auto lock = Lock{callbacks_.lock_};
    callbacks_.cb_ = {};
}

auto Profile::construct_row(
    const ProfileRowID& id,
    const ContactSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::ProfileSectionWidget(*this, api_, id, index, custom);
}

auto Profile::Delete(
    const int sectionType,
    const int type,
    const UnallocatedCString& claimID) const noexcept -> bool
{
    rLock lock{recursive_lock_};
    const auto& section = lookup(lock, static_cast<ProfileRowID>(sectionType));

    if (false == section.Valid()) { return false; }

    return section.Delete(type, claimID);
}

auto Profile::DisplayName() const noexcept -> UnallocatedCString
{
    rLock lock{recursive_lock_};

    return name_;
}

auto Profile::nym_name(
    const api::session::Crypto& crypto,
    const api::session::Wallet& wallet,
    const identifier::Nym& nymID) noexcept -> UnallocatedCString
{
    for (const auto& [id, name] : wallet.NymList()) {
        if (nymID.asBase58(crypto) == id) { return name; }
    }

    return {};
}

auto Profile::PaymentCode() const noexcept -> UnallocatedCString
{
    rLock lock{recursive_lock_};

    return payment_code_;
}

void Profile::process_nym(const identity::Nym& nym) noexcept
{
    {
        const auto name = nym.Alias();
        const auto code = nym.PaymentCode();
        auto nameChanged{false};
        auto codeChanged{false};

        {
            auto lock = rLock{recursive_lock_};

            if (name_ != name) {
                name_ = name;
                nameChanged = true;
            }

            if (payment_code_ != code) {
                payment_code_ = code;
                codeChanged = true;
            }
        }

        {
            auto lock = Lock{callbacks_.lock_};
            auto& cb = callbacks_.cb_;

            if (nameChanged && cb.name_) { cb.name_(name.c_str()); }
            if (codeChanged && cb.payment_code_) {
                cb.payment_code_(code.c_str());
            }
        }

        if (nameChanged || codeChanged) { UpdateNotify(); }
    }

    auto active = UnallocatedSet<ProfileRowID>{};

    for (const auto& section : nym.Claims()) {
        const auto& type = section.first;

        if (check_type(type)) {
            CustomData custom{
                new identity::wot::claim::Section(*section.second)};
            add_item(type, sort_key(type), custom);
            active.emplace(type);
        }
    }

    delete_inactive(active);
}

void Profile::process_nym(const Message& message) noexcept
{
    wait_for_startup();

    OT_ASSERT(1 < message.Body().size());

    const auto nymID = api_.Factory().NymIDFromHash(message.Body_at(1).Bytes());

    OT_ASSERT(false == nymID.empty());

    if (nymID != primary_id_) { return; }

    const auto nym = api_.Wallet().Nym(nymID);

    OT_ASSERT(nym);

    process_nym(*nym);
}

auto Profile::SetActive(
    const int sectionType,
    const int type,
    const UnallocatedCString& claimID,
    const bool active) const noexcept -> bool
{
    rLock lock{recursive_lock_};
    const auto& section = lookup(lock, static_cast<ProfileRowID>(sectionType));

    if (false == section.Valid()) { return false; }

    return section.SetActive(type, claimID, active);
}

auto Profile::SetCallbacks(Callbacks&& cb) noexcept -> void
{
    auto lock = Lock{callbacks_.lock_};
    callbacks_.cb_ = std::move(cb);
}

auto Profile::SetPrimary(
    const int sectionType,
    const int type,
    const UnallocatedCString& claimID,
    const bool primary) const noexcept -> bool
{
    rLock lock{recursive_lock_};
    const auto& section = lookup(lock, static_cast<ProfileRowID>(sectionType));

    if (false == section.Valid()) { return false; }

    return section.SetPrimary(type, claimID, primary);
}

auto Profile::SetValue(
    const int sectionType,
    const int type,
    const UnallocatedCString& claimID,
    const UnallocatedCString& value) const noexcept -> bool
{
    rLock lock{recursive_lock_};
    const auto& section = lookup(lock, static_cast<ProfileRowID>(sectionType));

    if (false == section.Valid()) { return false; }

    return section.SetValue(type, claimID, value);
}

auto Profile::sort_key(const identity::wot::claim::SectionType type) noexcept
    -> int
{
    return sort_keys_.at(type);
}

void Profile::startup() noexcept
{
    LogVerbose()(OT_PRETTY_CLASS())("Loading nym ")(primary_id_).Flush();
    const auto nym = api_.Wallet().Nym(primary_id_);

    OT_ASSERT(nym);

    process_nym(*nym);
    finish_startup();
}

Profile::~Profile()
{
    ClearCallbacks();

    for (const auto& it : listeners_) { delete it.second; }
}
}  // namespace opentxs::ui::implementation
