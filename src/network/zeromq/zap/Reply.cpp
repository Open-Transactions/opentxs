// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/zeromq/zap/Reply.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <memory>
#include <sstream>
#include <string_view>
#include <utility>

#include "internal/network/zeromq/zap/Factory.hpp"
#include "internal/network/zeromq/zap/Reply.hpp"
#include "internal/network/zeromq/zap/Request.hpp"
#include "internal/network/zeromq/zap/ZAP.hpp"
#include "internal/util/LogMacros.hpp"
#include "network/zeromq/message/Message.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Container.hpp"

namespace opentxs::factory
{
auto ZAPReply(
    const network::zeromq::zap::Request& request,
    const network::zeromq::zap::Status code,
    const ReadView status) noexcept -> network::zeromq::zap::Reply
{
    using ReturnType = network::zeromq::zap::Reply;

    return ZAPReply(
        request, code, status, {}, {}, ReturnType::Imp::default_version_);
}

auto ZAPReply(
    const network::zeromq::zap::Request& request,
    const network::zeromq::zap::Status code,
    const ReadView status,
    const ReadView userID,
    const ReadView metadata,
    const ReadView version) noexcept -> network::zeromq::zap::Reply
{
    using ReturnType = network::zeromq::zap::Reply;

    return std::make_unique<ReturnType::Imp>(
               request, code, status, userID, metadata, version)
        .release();
}
}  // namespace opentxs::factory

namespace opentxs::network::zeromq::zap
{
const Reply::Imp::CodeMap Reply::Imp::code_map_{
    {Status::Success, "200"},
    {Status::TemporaryError, "300"},
    {Status::AuthFailure, "400"},
    {Status::SystemError, "500"},
};

const Reply::Imp::CodeReverseMap Reply::Imp::code_reverse_map_{
    reverse_map(code_map_)};

Reply::Imp::Imp(
    const SimpleCallback header,
    const ReadView requestID,
    const zap::Status& code,
    const ReadView status,
    const ReadView userID,
    const ReadView metadata,
    const ReadView version) noexcept
    : zeromq::Message::Imp()
{
    const auto codeBytes = code_to_string(code);

    if (header) { header(); }

    StartBody();
    AddFrame(version);
    AddFrame(requestID);
    AddFrame(codeBytes);
    AddFrame(status);
    AddFrame(userID);
    AddFrame(metadata);
}

Reply::Imp::Imp(
    const Request& request,
    const zap::Status& code,
    const ReadView status,
    const ReadView userID,
    const ReadView metadata,
    const ReadView version) noexcept
    : Imp(
          [&] {
              if (0 < request.Header().size()) {
                  for (const auto& frame : request.Header()) {
                      frames_.emplace_back(frame);
                  }
              }
          },
          request.RequestID(),
          code,
          status,
          userID,
          metadata,
          version)
{
}

Reply::Imp::Imp() noexcept
    : zeromq::Message::Imp()
{
}

Reply::Imp::Imp(const Imp& rhs) noexcept
    : zeromq::Message::Imp(rhs)
{
}

auto Reply::Imp::code_to_string(const zap::Status& code) noexcept
    -> UnallocatedCString
{
    try {
        return code_map_.at(code);
    } catch (...) {

        return "";
    }
}

auto Reply::Imp::string_to_code(const ReadView string) noexcept -> zap::Status
{
    const auto key = UnallocatedCString{string};

    try {
        return code_reverse_map_.at(key);
    } catch (...) {

        return Status::Unknown;
    }
}
}  // namespace opentxs::network::zeromq::zap

namespace opentxs::network::zeromq::zap
{
Reply::Reply(Imp* imp) noexcept
    : Message(imp)
    , imp_(imp)
{
    OT_ASSERT(Reply::imp_);
}

Reply::Reply(const Reply& rhs) noexcept
    : Reply(std::make_unique<Imp>(*rhs.imp_).release())
{
}

Reply::Reply(Reply&& rhs) noexcept
    : Reply(std::make_unique<Imp>().release())
{
    swap(rhs);
}

auto Reply::operator=(const Reply& rhs) noexcept -> Reply&
{
    auto old = std::unique_ptr<Imp>(imp_);
    imp_ = std::make_unique<Imp>(*rhs.imp_).release();

    return *this;
}

auto Reply::operator=(Reply&& rhs) noexcept -> Reply&
{
    swap(rhs);

    return *this;
}

auto Reply::Code() const noexcept -> zap::Status
{
    const auto code = [&]() -> ReadView {
        try {

            return Message::imp_->Body_at(Imp::status_code_position_).Bytes();
        } catch (...) {

            return {};
        }
    }();

    return Imp::string_to_code(code);
}

auto Reply::Debug() const noexcept -> UnallocatedCString
{
    const auto code = [&]() -> ReadView {
        try {

            return Message::imp_->Body_at(Imp::status_code_position_).Bytes();
        } catch (...) {

            return {};
        }
    }();
    const auto request = RequestID();
    const auto meta = Metadata();
    auto output = std::stringstream{};
    output << "Version: " << Version() << "\n";
    output << "Request ID: 0x"
           << to_hex(
                  reinterpret_cast<const std::byte*>(request.data()),
                  request.size())
           << "\n";
    output << "Status Code: " << code << "\n";
    output << "Status Text: " << Status() << "\n";
    output << "User ID: " << UserID() << "\n";
    output << "Metadata: 0x"
           << to_hex(
                  reinterpret_cast<const std::byte*>(meta.data()), meta.size())
           << "\n";

    return output.str();
}

auto Reply::Metadata() const noexcept -> ReadView
{
    try {

        return Message::imp_->Body_at(Imp::metadata_position_).Bytes();
    } catch (...) {

        return {};
    }
}

auto Reply::RequestID() const noexcept -> ReadView
{
    try {

        return Message::imp_->Body_at(Imp::request_id_position_).Bytes();
    } catch (...) {

        return {};
    }
}

auto Reply::Status() const noexcept -> ReadView
{
    try {

        return Message::imp_->Body_at(Imp::status_text_position_).Bytes();
    } catch (...) {

        return {};
    }
}

auto Reply::swap(Message& rhs) noexcept -> void
{
    if (auto* other = dynamic_cast<Reply*>(&rhs); nullptr == other) {
        Message::swap(rhs);
        Reply::imp_ = nullptr;
    } else {
        swap(*other);
    }
}

auto Reply::swap(Reply& rhs) noexcept -> void
{
    Message::swap(rhs);
    std::swap(Reply::imp_, rhs.Reply::imp_);
}

auto Reply::UserID() const noexcept -> ReadView
{
    try {

        return Message::imp_->Body_at(Imp::user_id_position_).Bytes();
    } catch (...) {

        return {};
    }
}

auto Reply::Version() const noexcept -> ReadView
{
    try {

        return Message::imp_->Body_at(Imp::version_position_).Bytes();
    } catch (...) {

        return {};
    }
}

Reply::~Reply()
{
    if (nullptr != Reply::imp_) {
        delete Reply::imp_;
        Reply::imp_ = nullptr;
        Message::imp_ = nullptr;
    }
}
}  // namespace opentxs::network::zeromq::zap
