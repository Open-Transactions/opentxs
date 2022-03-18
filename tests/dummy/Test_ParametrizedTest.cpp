// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <ostream>
#include <string>

// parametrized test at global namespace
struct ParametrizedTestAtGlobalNamespaceStruct {
    ::std::string mName;
    bool mExpectedValue;

    friend ::std::ostream& operator<<(
        ::std::ostream& os,
        const ParametrizedTestAtGlobalNamespaceStruct& obj)
    {
        return os << "Test name: '" << obj.mName << "'";
    }
};

class ParametrizedTestAtGlobalNamespaceClass
    : public ::testing::TestWithParam<ParametrizedTestAtGlobalNamespaceStruct>
{
private:
    bool mBool;

public:
    ParametrizedTestAtGlobalNamespaceClass()
        : mBool(false)
    {
    }

    bool getBool() const { return mBool; }

    void SetUp() override { mBool = GetParam().mExpectedValue; }

    void TearDown() override { mBool = false; }
};

ParametrizedTestAtGlobalNamespaceStruct
    ParametrizedClassAtGlobalNamespace_TestVector[] = {
        {"YES", true},
        {"NO", false},
};

INSTANTIATE_TEST_CASE_P(
    ParametrizedTestAtGlobalNamespace,
    ParametrizedTestAtGlobalNamespaceClass,
    ::testing::ValuesIn(ParametrizedClassAtGlobalNamespace_TestVector));

TEST_P(ParametrizedTestAtGlobalNamespaceClass, simpleParametrizedTest)
{
    EXPECT_EQ(getBool(), GetParam().mExpectedValue);
}

// parametrized test at unnamed namespace
namespace
{
struct ParametrizedTestAtUnnamedNamespaceStruct {
    ::std::string mName;
    bool mExpectedValue;

    friend ::std::ostream& operator<<(
        ::std::ostream& os,
        const ParametrizedTestAtUnnamedNamespaceStruct& obj)
    {
        return os << "Test name: '" << obj.mName << "'";
    }
};

class ParametrizedTestAtUnnamedNamespaceClass
    : public ::testing::TestWithParam<ParametrizedTestAtUnnamedNamespaceStruct>
{
private:
    bool mBool;

public:
    ParametrizedTestAtUnnamedNamespaceClass()
        : mBool(false)
    {
    }

    bool getBool() const { return mBool; }

    void SetUp() override { mBool = GetParam().mExpectedValue; }

    void TearDown() override { mBool = false; }
};

ParametrizedTestAtUnnamedNamespaceStruct
    ParametrizedClassAtUnnamedNamespace_TestVector[] = {
        {"YES", true},
        {"NO", false},
};

INSTANTIATE_TEST_CASE_P(
    ParametrizedTestAtUnnamedNamespace,
    ParametrizedTestAtUnnamedNamespaceClass,
    ::testing::ValuesIn(ParametrizedClassAtUnnamedNamespace_TestVector));

TEST_P(ParametrizedTestAtUnnamedNamespaceClass, simpleParametrizedTest)
{
    EXPECT_EQ(getBool(), GetParam().mExpectedValue);
}
}  // namespace

// parametrized test at named namespace
namespace ottest
{
namespace DummyTest
{
struct ParametrizedTestAtNamedNamespaceStruct {
    ::std::string mName;
    bool mExpectedValue;

    friend ::std::ostream& operator<<(
        ::std::ostream& os,
        const ParametrizedTestAtNamedNamespaceStruct& obj)
    {
        return os << "Test name: '" << obj.mName << "'";
    }
};

class ParametrizedTestAtNamedNamespaceClass
    : public ::testing::TestWithParam<ParametrizedTestAtNamedNamespaceStruct>
{
private:
    bool mBool;

public:
    ParametrizedTestAtNamedNamespaceClass()
        : mBool(false)
    {
    }

    bool getBool() const { return mBool; }

    void SetUp() override { mBool = GetParam().mExpectedValue; }

    void TearDown() override { mBool = false; }
};

ParametrizedTestAtNamedNamespaceStruct
    ParametrizedClassAtNamedNamespace_TestVector[] = {
        {"YES", true},
        {"NO", false},
};

INSTANTIATE_TEST_CASE_P(
    ParametrizedTestAtNamedNamespace,
    ParametrizedTestAtNamedNamespaceClass,
    ::testing::ValuesIn(ParametrizedClassAtNamedNamespace_TestVector));

TEST_P(ParametrizedTestAtNamedNamespaceClass, simpleParametrizedTest)
{
    EXPECT_EQ(getBool(), GetParam().mExpectedValue);
}

}  // namespace DummyTest
}  // namespace ottest
