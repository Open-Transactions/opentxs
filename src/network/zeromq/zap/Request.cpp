// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/zap/Request.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include "network/zeromq/Message.hpp"

#include <map>
#include <set>
#include <sstream>
#include <string>

#include "Request.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::zap::Request>;

#define MAX_STRING_FIELD_SIZE 255
#define PUBKEY_SIZE 32

#define OT_METHOD "opentxs::network::zeromq::zap::implementation::Request"

namespace opentxs::network::zeromq::zap
{
OTZMQZAPRequest Request::Factory()
{
    return OTZMQZAPRequest(new implementation::Request());
}

OTZMQZAPRequest Request::Factory(
    const std::string& address,
    const std::string& domain,
    const zap::Mechanism mechanism,
    const Data& requestID,
    const Data& identity,
    const std::string& version)
{
    return OTZMQZAPRequest(new implementation::Request(
        address, domain, mechanism, requestID, identity, version));
}
}  // namespace opentxs::network::zeromq::zap

namespace opentxs::network::zeromq::zap::implementation
{
const std::set<std::string> Request::accept_versions_{"1.0"};
const Request::MechanismMap Request::mechanism_map_{
    {Mechanism::Null, "NULL"},
    {Mechanism::Plain, "PLAIN"},
    {Mechanism::Curve, "CURVE"},
};
const Request::MechanismReverseMap Request::mechanism_reverse_map_{
    invert_mechanism_map(mechanism_map_)};

Request::Request()
    : zeromq::implementation::Message()
{
}

Request::Request(
    const std::string& address,
    const std::string& domain,
    const zap::Mechanism mechanism,
    const Data& requestID,
    const Data& identity,
    const std::string& version)
    : Request()
{
    messages_.emplace_back(Frame::Factory(version));
    messages_.emplace_back(Frame::Factory(requestID));
    messages_.emplace_back(Frame::Factory(domain));
    messages_.emplace_back(Frame::Factory(address));
    messages_.emplace_back(Frame::Factory(identity));
    messages_.emplace_back(Frame::Factory(mechanism_to_string(mechanism)));
}

Request::Request(const Request& rhs)
    : zeromq::Message()
    , zeromq::zap::Request()
    , zeromq::implementation::Message(rhs)
{
}

const FrameSection Request::Credentials() const
{
    const std::size_t position{body_position() + CREDENTIALS_START_POSITION};
    auto size = std::max(messages_.size() - position, std::size_t{0});

    return FrameSection(this, position, size);
}

std::string Request::Debug() const
{
    std::stringstream output{};
    const auto body = Body();

    if (VERSION_POSITION < body.size()) {
        output << "Version: " << std::string(body.at(VERSION_POSITION)) << "\n";
    }

    if (REQUEST_ID_POSITION < body.size()) {
        const auto& requestID = body.at(REQUEST_ID_POSITION);

        if (0 < requestID.size()) {
            output << "Request ID: 0x" << Data::Factory(requestID)->asHex()
                   << "\n";
        }
    }

    if (DOMAIN_POSITION < body.size()) {
        output << "Domain: " << std::string(body.at(DOMAIN_POSITION)) << "\n";
    }

    if (ADDRESS_POSITION < body.size()) {
        output << "Address: " << std::string(body.at(ADDRESS_POSITION)) << "\n";
    }

    if (IDENTITY_POSITION < body.size()) {
        const auto& identity = body.at(IDENTITY_POSITION);

        if (0 < identity.size()) {
            output << "Identity: 0x" << Data::Factory(identity)->asHex()
                   << "\n";
        }
    }

    if (MECHANISM_POSITION < body.size()) {
        output << "Mechanism: " << std::string(body.at(MECHANISM_POSITION))
               << "\n";
        const auto credentials = Credentials();

        switch (Mechanism()) {
            case Mechanism::Plain: {
                if (0 < credentials.size()) {
                    LogOutput(OT_METHOD)(__FUNCTION__)(": * Username: ")(
                        std::string(credentials.at(0)))(".")
                        .Flush();
                }

                if (1 < credentials.size()) {
                    LogOutput(OT_METHOD)(__FUNCTION__)(": * Password: ")(
                        std::string(credentials.at(1)))(".")
                        .Flush();
                }
            } break;
            case Mechanism::Curve: {
                for (const auto& credential : credentials) {
                    LogOutput(OT_METHOD)(__FUNCTION__)(": * Pubkey: 0x ")(
                        Data::Factory(credential)->asHex())(".")
                        .Flush();
                }
            } break;
            case Mechanism::Null:
            case Mechanism::Unknown:
            default: {
            }
        }
    }

    return output.str();
}

Request::MechanismReverseMap Request::invert_mechanism_map(
    const MechanismMap& input)
{
    MechanismReverseMap output{};

    for (const auto& [type, string] : input) { output.emplace(string, type); }

    return output;
}

std::string Request::mechanism_to_string(const zap::Mechanism in)
{
    try {
        return mechanism_map_.at(in);
    } catch (...) {

        return "";
    }
}

zap::Mechanism Request::string_to_mechanism(const std::string& in)
{
    try {
        return mechanism_reverse_map_.at(in);
    } catch (...) {

        return Mechanism::Unknown;
    }
}

std::pair<bool, std::string> Request::Validate() const
{
    std::pair<bool, std::string> output{false, ""};
    auto& [success, error] = output;
    const auto body = body_position();

    if (VERSION_POSITION + body >= messages_.size()) {
        error = "Missing version";
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(error)(".").Flush();

        return output;
    }

    if (0 == accept_versions_.count(Version())) {
        error = std::string("Invalid version (") + Version() + ")";
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(error)(".").Flush();

        return output;
    }

    if (REQUEST_ID_POSITION + body >= messages_.size()) {
        error = "Missing request ID";
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(error)(".").Flush();

        return output;
    }

    const auto requestSize = Body_at(REQUEST_ID_POSITION).size();

    if (MAX_STRING_FIELD_SIZE < requestSize) {
        error = std::string("Request ID too long (") +
                std::to_string(requestSize) + ")";
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(error)(".").Flush();

        return output;
    }

    if (DOMAIN_POSITION + body >= messages_.size()) {
        error = "Missing domain";
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(error)(".").Flush();

        return output;
    }

    const auto domainSize = Body_at(DOMAIN_POSITION).size();

    if (MAX_STRING_FIELD_SIZE < domainSize) {
        error =
            std::string("Domain too long (") + std::to_string(domainSize) + ")";
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(error)(".").Flush();

        return output;
    }

    if (ADDRESS_POSITION + body >= messages_.size()) {
        error = "Missing address";
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(error)(".").Flush();

        return output;
    }

    const auto addressSize = Body_at(ADDRESS_POSITION).size();

    if (MAX_STRING_FIELD_SIZE < addressSize) {
        error = std::string("Address too long (") +
                std::to_string(addressSize) + ")";
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(error)(".").Flush();

        return output;
    }

    if (IDENTITY_POSITION + body >= messages_.size()) {
        error = "Missing identity";
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(error)(".").Flush();

        return output;
    }

    const auto identitySize = Body_at(IDENTITY_POSITION).size();

    if (MAX_STRING_FIELD_SIZE < identitySize) {
        error = std::string("Identity too long (") +
                std::to_string(identitySize) + ")";
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(error)(".").Flush();

        return output;
    }

    if (MECHANISM_POSITION + body >= messages_.size()) {
        error = "Missing mechanism";
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(error)(".").Flush();

        return output;
    }

    const std::string mechanism{messages_.at(MECHANISM_POSITION + body).get()};
    const bool validMechanism = 1 == mechanism_reverse_map_.count(mechanism);

    if (false == validMechanism) {
        error = std::string("Unknown mechanism (") + mechanism + ")";
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(error)(".").Flush();

        return output;
    }

    const auto credentials = Credentials();
    const auto count = credentials.size();

    switch (Mechanism()) {
        case Mechanism::Null: {
            if (0 != count) {
                error = std::string("Too many credentials (") +
                        std::to_string(count) + ")";
                LogOutput(OT_METHOD)(__FUNCTION__)(": ")(error)(".").Flush();

                return output;
            }
        } break;
        case Mechanism::Plain: {
            if (1 > count) {
                error = "Missing username";
                LogOutput(OT_METHOD)(__FUNCTION__)(": ")(error)(".").Flush();

                return output;
            }

            const auto username = credentials.at(0).size();

            if (MAX_STRING_FIELD_SIZE < username) {
                error = std::string("Username too long (") +
                        std::to_string(username) + ")";
                LogOutput(OT_METHOD)(__FUNCTION__)(": ")(error)(".").Flush();

                return output;
            }

            if (2 > count) {
                error = "Missing password";
                LogOutput(OT_METHOD)(__FUNCTION__)(": ")(error)(".").Flush();

                return output;
            }

            const auto password = credentials.at(1).size();

            if (MAX_STRING_FIELD_SIZE < password) {
                error = std::string("Password too long (") +
                        std::to_string(password) + ")";
                LogOutput(OT_METHOD)(__FUNCTION__)(": ")(error)(".").Flush();

                return output;
            }
        } break;
        case Mechanism::Curve: {
            if (1 != count) {
                error = std::string("Wrong number of credentials (") +
                        std::to_string(count) + ")";
                LogOutput(OT_METHOD)(__FUNCTION__)(": ")(error)(".").Flush();

                return output;
            }

            const auto pubkey = credentials.at(0).size();

            if (PUBKEY_SIZE != pubkey) {
                error = std::string("Wrong pubkey size (") +
                        std::to_string(pubkey) + ")";
                LogOutput(OT_METHOD)(__FUNCTION__)(": ")(error)(".").Flush();

                return output;
            }
        } break;
        case Mechanism::Unknown:
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid mechanism.").Flush();

            return output;
        }
    }

    success = true;
    error = "OK";

    return output;
}
}  // namespace opentxs::network::zeromq::zap::implementation
