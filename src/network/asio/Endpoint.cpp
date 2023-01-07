// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/network/asio/Endpoint.hpp"  // IWYU pragma: associated

#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "internal/network/asio/Types.hpp"
#include "network/asio/Endpoint.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Bytes.hpp"

namespace opentxs::network::asio
{
Endpoint::Imp::Imp(Type type, ReadView raw, Port port) noexcept(false)
    : type_(type)
    , data_([&]() -> tcp::endpoint {
        if (const auto addr = address_from_binary(raw); addr.has_value()) {

            return tcp::endpoint{*addr, port};
        } else {

            throw std::runtime_error{"Invalid params"};
        }
    }())
    , bytes_(space(serialize(data_.address()).Bytes()))
{
}

Endpoint::Imp::Imp() noexcept
    : type_(Type::none)
    , data_()
    , bytes_()
{
}

Endpoint::Imp::Imp(const Imp& rhs) noexcept = default;

Endpoint::Imp::~Imp() = default;

Endpoint::Endpoint(Type type, ReadView bytes, Port port) noexcept(false)
    : imp_(std::make_unique<Imp>(type, bytes, port).release())
{
}

Endpoint::Endpoint() noexcept
    : imp_(std::make_unique<Imp>().release())
{
}

Endpoint::Endpoint(const Endpoint& rhs) noexcept
    : imp_(std::make_unique<Imp>(*rhs.imp_).release())
{
}

Endpoint::Endpoint(Endpoint&& rhs) noexcept
    : imp_()
{
    std::swap(imp_, rhs.imp_);
}

auto Endpoint::operator=(const Endpoint& rhs) noexcept -> Endpoint&
{
    if (this != &rhs) {
        auto imp = std::unique_ptr<Imp>{imp_};
        imp = std::make_unique<Imp>(*rhs.imp_);
        imp_ = imp.release();
    }

    return *this;
}

auto Endpoint::operator=(Endpoint&& rhs) noexcept -> Endpoint&
{
    if (this != &rhs) { std::swap(imp_, rhs.imp_); }

    return *this;
}

auto Endpoint::GetAddress() const noexcept -> UnallocatedCString
{
    return imp_->data_.address().to_string();
}

auto Endpoint::GetBytes() const noexcept -> ReadView
{
    return reader(imp_->bytes_);
}

auto Endpoint::GetInternal() const noexcept -> const Imp& { return *imp_; }

auto Endpoint::GetMapped() const noexcept -> UnallocatedCString
{
    return map_4_to_6(imp_->data_.address()).to_string();
}

auto Endpoint::GetPort() const noexcept -> Port { return imp_->data_.port(); }

auto Endpoint::GetType() const noexcept -> Type { return imp_->type_; }

auto Endpoint::str() const noexcept -> UnallocatedCString
{
    auto output = std::stringstream{};
    output << imp_->data_;

    return output.str();
}

Endpoint::~Endpoint() { std::unique_ptr<Imp>{imp_}.reset(); }
}  // namespace opentxs::network::asio
