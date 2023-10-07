// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/zeromq/zap/Request.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <memory>
#include <span>
#include <sstream>
#include <string_view>
#include <utility>

#include "internal/network/zeromq/zap/Factory.hpp"
#include "internal/network/zeromq/zap/Request.hpp"
#include "internal/network/zeromq/zap/ZAP.hpp"
#include "network/zeromq/message/Message.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/Container.hpp"

namespace opentxs::factory
{
auto ZAPRequest(
    const ReadView address,
    const ReadView domain,
    const network::zeromq::zap::Mechanism mechanism,
    const ReadView requestID,
    const ReadView identity,
    const ReadView version) noexcept -> network::zeromq::zap::Request
{
    using ReturnType = network::zeromq::zap::Request;

    return std::make_unique<ReturnType::Imp>(
               address, domain, mechanism, requestID, identity, version)
        .release();
}
}  // namespace opentxs::factory

namespace opentxs::network::zeromq::zap
{
const UnallocatedSet<UnallocatedCString> Request::Imp::accept_versions_{
    default_version_};
const Request::Imp::MechanismMap Request::Imp::mechanism_map_{
    {Mechanism::Null, "NULL"},
    {Mechanism::Plain, "PLAIN"},
    {Mechanism::Curve, "CURVE"},
};
const Request::Imp::MechanismReverseMap Request::Imp::mechanism_reverse_map_{
    reverse_map(mechanism_map_)};

Request::Imp::Imp(
    const ReadView address,
    const ReadView domain,
    const network::zeromq::zap::Mechanism mechanism,
    const ReadView requestID,
    const ReadView identity,
    const ReadView version) noexcept
    : zeromq::Message::Imp()
{
    AddFrame(version);
    AddFrame(requestID);
    AddFrame(domain);
    AddFrame(address);
    AddFrame(identity);
    AddFrame(mechanism_to_string(mechanism));
}

Request::Imp::Imp() noexcept
    : zeromq::Message::Imp()
{
}

Request::Imp::Imp(const Imp& rhs) noexcept
    : zeromq::Message::Imp(rhs)
{
}

auto Request::Imp::mechanism_to_string(const zap::Mechanism in)
    -> UnallocatedCString
{
    try {
        return mechanism_map_.at(in);
    } catch (...) {

        return "";
    }
}

auto Request::Imp::string_to_mechanism(const ReadView in) -> zap::Mechanism
{
    const auto key = UnallocatedCString{in};

    try {
        return mechanism_reverse_map_.at(key);
    } catch (...) {

        return Mechanism::Unknown;
    }
}
}  // namespace opentxs::network::zeromq::zap

namespace opentxs::network::zeromq::zap
{
Request::Request(Imp* imp) noexcept
    : imp_(imp)
{
    assert_false(nullptr == imp_);
}

Request::Request() noexcept
    : Request(std::make_unique<Imp>().release())
{
}

Request::Request(const Request& rhs) noexcept
    : Request(std::make_unique<Imp>(*rhs.imp_).release())
{
}

Request::Request(Request&& rhs) noexcept
    : Request(std::make_unique<Imp>().release())
{
    swap(rhs);
}

auto Request::operator=(const Request& rhs) noexcept -> Request&
{
    auto old = std::unique_ptr<Imp>(imp_);
    imp_ = std::make_unique<Imp>(*rhs.imp_).release();

    return *this;
}

auto Request::operator=(Request&& rhs) noexcept -> Request&
{
    swap(rhs);

    return *this;
}

auto Request::swap(Message& rhs) noexcept -> void
{
    if (auto* other = dynamic_cast<Request*>(&rhs); nullptr == other) {
        Message::swap(rhs);
        Request::imp_ = nullptr;
    } else {
        swap(*other);
    }
}

auto Request::swap(Request& rhs) noexcept -> void
{
    Message::swap(rhs);
    std::swap(Request::imp_, rhs.Request::imp_);
}

auto Request::Address() const noexcept -> ReadView
{
    const auto body = imp_->Payload();

    if (Imp::address_position_ < body.size()) {

        return body[Imp::address_position_].Bytes();
    } else {

        return {};
    }
}

auto Request::Credentials() const noexcept -> std::span<const Frame>
{
    auto body = imp_->Payload();

    if (body.size() > Imp::credentials_start_position_) {

        return body.subspan(Imp::credentials_start_position_);
    } else {

        return {};
    }
}

auto Request::Debug() const noexcept -> UnallocatedCString
{
    auto output = std::stringstream{};
    const auto req = RequestID();
    const auto id = Identity();
    const auto mechanism = [&]() -> ReadView {
        const auto body = imp_->Payload();

        if (Imp::mechanism_position_ < body.size()) {

            return body[Imp::mechanism_position_].Bytes();
        } else {

            return {};
        }
    }();
    const auto credentials = Credentials();
    output << "Version: " << Version() << "\n";
    output << "Request ID: 0x"
           << to_hex(reinterpret_cast<const std::byte*>(req.data()), req.size())
           << "\n";
    output << "Domain: " << Domain() << "\n";
    output << "Address: " << Address() << "\n";
    output << "Identity: 0x"
           << to_hex(reinterpret_cast<const std::byte*>(id.data()), id.size())
           << "\n";
    output << "Mechanism: " << mechanism << "\n";

    switch (Mechanism()) {
        case Mechanism::Plain: {
            if (0 < credentials.size()) {
                output << "* Username: " << credentials[0].Bytes() << '\n';
            }

            if (1 < credentials.size()) {
                output << "* Username: " << credentials[1].Bytes() << '\n';
            }
        } break;
        case Mechanism::Curve: {
            for (const auto& credential : credentials) {
                output << "* Pubkey: 0x"
                       << to_hex(
                              reinterpret_cast<const std::byte*>(
                                  credential.data()),
                              credential.size())
                       << '\n';
            }
        } break;
        case Mechanism::Null:
        case Mechanism::Unknown:
        default: {
        }
    }

    return output.str();
}

auto Request::Domain() const noexcept -> ReadView
{
    const auto body = imp_->Payload();

    if (Imp::domain_position_ < body.size()) {

        return body[Imp::domain_position_].Bytes();
    } else {

        return {};
    }
}

auto Request::Identity() const noexcept -> ReadView
{
    const auto body = imp_->Payload();

    if (Imp::identity_position_ < body.size()) {

        return body[Imp::identity_position_].Bytes();
    } else {

        return {};
    }
}

auto Request::Mechanism() const noexcept -> zap::Mechanism
{
    const auto body = imp_->Payload();

    if (Imp::mechanism_position_ < body.size()) {

        return Imp::string_to_mechanism(body[Imp::mechanism_position_].Bytes());
    } else {

        return {};
    }
}

auto Request::RequestID() const noexcept -> ReadView
{
    const auto body = imp_->Payload();

    if (Imp::request_id_position_ < body.size()) {

        return body[Imp::request_id_position_].Bytes();
    } else {

        return {};
    }
}

auto Request::Validate() const noexcept -> std::pair<bool, UnallocatedCString>
{
    auto output = std::pair<bool, UnallocatedCString>{false, ""};
    auto& [success, error] = output;
    const auto body = imp_->Payload();
    const auto size = body.size();

    if (Imp::version_position_ >= size) {
        error = "Missing version";
        LogError()()(error).Flush();

        return output;
    }

    if (0 == Imp::accept_versions_.count(UnallocatedCString{Version()})) {
        error = UnallocatedCString{"Invalid version ("} +
                UnallocatedCString{Version()} + ")";
        LogError()()(error).Flush();

        return output;
    }

    if (Imp::request_id_position_ >= size) {
        error = "Missing request ID";
        LogError()()(error).Flush();

        return output;
    }

    const auto requestSize = body[Imp::request_id_position_].size();

    if (Imp::max_string_field_size_ < requestSize) {
        error = UnallocatedCString("Request ID too long (") +
                std::to_string(requestSize) + ")";
        LogError()()(error).Flush();

        return output;
    }

    if (Imp::domain_position_ >= size) {
        error = "Missing domain";
        LogError()()(error).Flush();

        return output;
    }

    const auto domainSize = body[Imp::domain_position_].size();

    if (Imp::max_string_field_size_ < domainSize) {
        error = UnallocatedCString("Domain too long (") +
                std::to_string(domainSize) + ")";
        LogError()()(error).Flush();

        return output;
    }

    if (Imp::address_position_ >= size) {
        error = "Missing address";
        LogError()()(error).Flush();

        return output;
    }

    const auto addressSize = body[Imp::address_position_].size();

    if (Imp::max_string_field_size_ < addressSize) {
        error = UnallocatedCString("Address too long (") +
                std::to_string(addressSize) + ")";
        LogError()()(error).Flush();

        return output;
    }

    if (Imp::identity_position_ >= size) {
        error = "Missing identity";
        LogError()()(error).Flush();

        return output;
    }

    const auto identitySize = body[Imp::identity_position_].size();

    if (Imp::max_string_field_size_ < identitySize) {
        error = UnallocatedCString("Identity too long (") +
                std::to_string(identitySize) + ")";
        LogError()()(error).Flush();

        return output;
    }

    if (Imp::mechanism_position_ >= size) {
        error = "Missing mechanism";
        LogError()()(error).Flush();

        return output;
    }

    const UnallocatedCString mechanism{body[Imp::mechanism_position_].Bytes()};
    const bool validMechanism =
        1 == Imp::mechanism_reverse_map_.count(mechanism);

    if (false == validMechanism) {
        error = UnallocatedCString("Unknown mechanism (") + mechanism + ")";
        LogError()()(error).Flush();

        return output;
    }

    const auto credentials = Credentials();
    const auto count = credentials.size();

    switch (Mechanism()) {
        case Mechanism::Null: {
            if (0 != count) {
                error = UnallocatedCString("Too many credentials (") +
                        std::to_string(count) + ")";
                LogError()()(error).Flush();

                return output;
            }
        } break;
        case Mechanism::Plain: {
            if (1 > count) {
                error = "Missing username";
                LogError()()(error).Flush();

                return output;
            }

            const auto username = credentials[0].size();

            if (Imp::max_string_field_size_ < username) {
                error = UnallocatedCString("Username too long (") +
                        std::to_string(username) + ")";
                LogError()()(error).Flush();

                return output;
            }

            if (2 > count) {
                error = "Missing password";
                LogError()()(error).Flush();

                return output;
            }

            const auto password = credentials[1].size();

            if (Imp::max_string_field_size_ < password) {
                error = UnallocatedCString("Password too long (") +
                        std::to_string(password) + ")";
                LogError()()(error).Flush();

                return output;
            }
        } break;
        case Mechanism::Curve: {
            if (1 != count) {
                error = UnallocatedCString("Wrong number of credentials (") +
                        std::to_string(count) + ")";
                LogError()()(error).Flush();

                return output;
            }

            const auto pubkey = credentials[0].size();

            if (Imp::pubkey_size_ != pubkey) {
                error = UnallocatedCString("Wrong pubkey size (") +
                        std::to_string(pubkey) + ")";
                LogError()()(error).Flush();

                return output;
            }
        } break;
        case Mechanism::Unknown:
        default: {
            LogError()()("Invalid mechanism.").Flush();

            return output;
        }
    }

    success = true;
    error = "OK";

    return output;
}

auto Request::Version() const noexcept -> ReadView
{
    const auto body = imp_->Payload();

    if (Imp::version_position_ < body.size()) {

        return body[Imp::version_position_].Bytes();
    } else {

        return {};
    }
}

Request::~Request()
{
    if (nullptr != Request::imp_) {
        delete Request::imp_;
        Request::imp_ = nullptr;
        Message::imp_ = nullptr;
    }
}
}  // namespace opentxs::network::zeromq::zap
