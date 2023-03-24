// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "crypto/hasher/HasherPrivate.hpp"  // IWYU pragma: associated

#include <openssl/evp.h>
#if __has_include(<openssl/types.h>)
// TODO openssl-3
#include <openssl/types.h>  // IWYU pragma: keep
#elif __has_include(<openssl/ossl_typ.h>)
#include <openssl/ossl_typ.h>  // IWYU pragma: keep
#endif

#include "crypto/library/openssl/OpenSSL.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::crypto
{
HasherPrivate::HasherPrivate(const void* evp, const void* stage2) noexcept
    : size_1_([&]() -> std::size_t {
        if (nullptr != evp) {
            const auto* type = static_cast<const EVP_MD*>(evp);

            if (const auto size = EVP_MD_size(type); 0 < size) {

                return size;
            } else {

                return 0_uz;
            }
        } else {

            return 0_uz;
        }
    }())
    , size_2_([&]() -> std::size_t {
        if (nullptr != stage2) {
            const auto* type = static_cast<const EVP_MD*>(stage2);

            if (const auto size = EVP_MD_size(type); 0 < size) {

                return size;
            } else {

                return 0_uz;
            }
        } else {

            return size_1_;
        }
    }())
    , context_(EVP_MD_CTX_new())
    , stage_2_(stage2)
{
    auto* context = static_cast<EVP_MD_CTX*>(context_);
    const auto* type = static_cast<const EVP_MD*>(evp);

    if (1 != EVP_DigestInit_ex(context, type, nullptr)) { free_context(); }
}

HasherPrivate::HasherPrivate() noexcept
    : HasherPrivate(nullptr, nullptr)
{
}

auto HasherPrivate::free_context() noexcept -> void
{
    if (nullptr != context_) {
        EVP_MD_CTX_free(static_cast<EVP_MD_CTX*>(context_));
        context_ = nullptr;
    }
}

auto HasherPrivate::operator()(ReadView data) noexcept -> bool
{
    if (nullptr == context_) { return false; }

    auto* context = static_cast<EVP_MD_CTX*>(context_);

    return 1 == EVP_DigestUpdate(context, data.data(), data.size());
}

auto HasherPrivate::operator()(Writer&& destination) noexcept -> bool
{
    if (nullptr == context_) { return false; }

    if (nullptr == stage_2_) {
        auto buf = destination.Reserve(size_1_);

        if (false == buf.IsValid(size_1_)) { return false; }

        auto* context = static_cast<EVP_MD_CTX*>(context_);
        const auto rc =
            EVP_DigestFinal_ex(context, buf.as<unsigned char>(), nullptr);
        free_context();

        return 1 == rc;
    } else {
        auto intermediate = ByteArray{};
        {
            auto buf = intermediate.WriteInto().Reserve(size_1_);

            if (false == buf.IsValid(size_1_)) { return false; }

            auto* context = static_cast<EVP_MD_CTX*>(context_);
            const auto rc =
                EVP_DigestFinal_ex(context, buf.as<unsigned char>(), nullptr);
            free_context();

            if (1 != rc) { return false; }
        }

        const auto data = intermediate.Bytes();
        auto context2 = OpenSSL_EVP_MD_CTX{EVP_MD_CTX_new(), &EVP_MD_CTX_free};
        const auto* type = static_cast<const EVP_MD*>(stage_2_);

        if (1 != EVP_DigestInit_ex(context2.get(), type, nullptr)) {
            return false;
        }

        if (1 != EVP_DigestUpdate(context2.get(), data.data(), data.size())) {
            return false;
        }

        auto buf = destination.Reserve(size_2_);

        if (false == buf.IsValid(size_2_)) { return false; }

        return 1 == EVP_DigestFinal_ex(
                        context2.get(), buf.as<unsigned char>(), nullptr);
    }
}

HasherPrivate::~HasherPrivate() { free_context(); }
}  // namespace opentxs::crypto
