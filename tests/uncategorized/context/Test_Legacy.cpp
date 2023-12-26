// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstdint>
#include <type_traits>

#include "opentxs/api/Paths.internal.hpp"
#include "ottest/Basic.hpp"  // IWYU pragma: keep
#include "ottest/fixtures/context/Filename.hpp"

namespace ottest
{
TEST_F(Filename, GetFilenameBin)
{
    const ot::UnallocatedCString exp{"filename.bin"};
    const ot::UnallocatedCString s{
        opentxs::api::internal::Paths::GetFilenameBin("filename")};
    ASSERT_STREQ(s.c_str(), exp.c_str());
}

TEST_F(Filename, getFilenameBin_invalid_input)
{
    ASSERT_TRUE(opentxs::api::internal::Paths::GetFilenameBin("-1").empty());
    ASSERT_TRUE(opentxs::api::internal::Paths::GetFilenameBin("").empty());
    ASSERT_TRUE(opentxs::api::internal::Paths::GetFilenameBin(nullptr).empty());
}

TEST_F(Filename, GetFilenameA)
{
    const ot::UnallocatedCString exp{"filename.a"};
    const ot::UnallocatedCString s{
        opentxs::api::internal::Paths::GetFilenameA("filename")};
    ASSERT_STREQ(s.c_str(), exp.c_str());
}

TEST_F(Filename, getFilenameA_invalid_input)
{
    ASSERT_TRUE(opentxs::api::internal::Paths::GetFilenameA("-1").empty());
    ASSERT_TRUE(opentxs::api::internal::Paths::GetFilenameA("").empty());
    ASSERT_TRUE(opentxs::api::internal::Paths::GetFilenameA(nullptr).empty());
}

TEST_F(Filename, GetFilenameR)
{
    const ot::UnallocatedCString exp{"filename.r"};
    const ot::UnallocatedCString s{
        opentxs::api::internal::Paths::GetFilenameR("filename")};
    ASSERT_STREQ(s.c_str(), exp.c_str());
}

TEST_F(Filename, getFilenameR_invalid_input)
{
    ASSERT_TRUE(opentxs::api::internal::Paths::GetFilenameR("-1").empty());
    ASSERT_TRUE(opentxs::api::internal::Paths::GetFilenameR("").empty());
    ASSERT_TRUE(opentxs::api::internal::Paths::GetFilenameR(nullptr).empty());
}

TEST_F(Filename, GetFilenameRct)
{
    {
        const ot::UnallocatedCString exp{"123.rct"};
        const ot::UnallocatedCString s{
            opentxs::api::internal::Paths::GetFilenameRct(123)};
        ASSERT_STREQ(s.c_str(), exp.c_str());
    }
    {
        const ot::UnallocatedCString exp{"0.rct"};
        const ot::UnallocatedCString s{
            opentxs::api::internal::Paths::GetFilenameRct(000)};
        ASSERT_STREQ(s.c_str(), exp.c_str());
    }
}

TEST_F(Filename, getFilenameRct_invalid_input)
{
    ASSERT_TRUE(opentxs::api::internal::Paths::GetFilenameRct(-1).empty());
}

TEST_F(Filename, GetFilenameCrn)
{
    {
        const ot::UnallocatedCString exp{"123.crn"};
        static_assert(
            std::is_same_v<int64_t, opentxs::TransactionNumber>,
            "type is not matching");  // detect if type change
        const ot::UnallocatedCString s{
            opentxs::api::internal::Paths::GetFilenameCrn(123)};
        ASSERT_STREQ(s.c_str(), exp.c_str());
    }
    {
        const ot::UnallocatedCString exp{"0.crn"};
        const ot::UnallocatedCString s{
            opentxs::api::internal::Paths::GetFilenameCrn(000)};
        ASSERT_STREQ(s.c_str(), exp.c_str());
    }
}

TEST_F(Filename, getFilenameCrn_invalid_input)
{
    ASSERT_TRUE(opentxs::api::internal::Paths::GetFilenameCrn(-1).empty());
}

TEST_F(Filename, GetFilenameSuccess)
{
    const ot::UnallocatedCString exp{"filename.success"};
    const ot::UnallocatedCString s{
        opentxs::api::internal::Paths::GetFilenameSuccess("filename")};
    ASSERT_STREQ(s.c_str(), exp.c_str());
}

TEST_F(Filename, getFilenameSuccess_invalid_input)
{
    ASSERT_TRUE(
        opentxs::api::internal::Paths::GetFilenameSuccess("-1").empty());
    ASSERT_TRUE(opentxs::api::internal::Paths::GetFilenameSuccess("").empty());
    ASSERT_TRUE(
        opentxs::api::internal::Paths::GetFilenameSuccess(nullptr).empty());
}

TEST_F(Filename, GetFilenameFail)
{
    const ot::UnallocatedCString exp{"filename.fail"};
    const ot::UnallocatedCString s{
        opentxs::api::internal::Paths::GetFilenameFail("filename")};
    ASSERT_STREQ(s.c_str(), exp.c_str());
}

TEST_F(Filename, getFilenameFail_invalid_input)
{
    ASSERT_TRUE(opentxs::api::internal::Paths::GetFilenameFail("-1").empty());
    ASSERT_TRUE(opentxs::api::internal::Paths::GetFilenameFail("").empty());
    ASSERT_TRUE(
        opentxs::api::internal::Paths::GetFilenameFail(nullptr).empty());
}

TEST_F(Filename, GetFilenameError)
{
    const ot::UnallocatedCString exp{"filename.error"};
    const ot::UnallocatedCString s{
        opentxs::api::internal::Paths::GetFilenameError("filename")};
    ASSERT_STREQ(s.c_str(), exp.c_str());
}

TEST_F(Filename, getFilenameError_invalid_input)
{
    ASSERT_TRUE(opentxs::api::internal::Paths::GetFilenameError("-1").empty());
    ASSERT_TRUE(opentxs::api::internal::Paths::GetFilenameError("").empty());
    ASSERT_TRUE(
        opentxs::api::internal::Paths::GetFilenameError(nullptr).empty());
}

TEST_F(Filename, GetFilenameLst)
{
    const ot::UnallocatedCString exp{"filename.lst"};
    const ot::UnallocatedCString s{
        opentxs::api::internal::Paths::GetFilenameLst("filename")};
    ASSERT_STREQ(s.c_str(), exp.c_str());
}

TEST_F(Filename, getFilenameLst_invalid_input)
{
    ASSERT_TRUE(opentxs::api::internal::Paths::GetFilenameError("-1").empty());
    ASSERT_TRUE(opentxs::api::internal::Paths::GetFilenameError("").empty());
    ASSERT_TRUE(
        opentxs::api::internal::Paths::GetFilenameError(nullptr).empty());
}
}  // namespace ottest
