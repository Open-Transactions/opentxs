// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <ostream>
#include <string>

// NOLINTBEGIN(modernize-avoid-c-arrays)
// parametrized test at global namespace
struct ParametrizedTestAtGlobalNamespaceStruct {
    ::std::string m_name_;
    bool m_expected_value_;

    friend auto operator<<(
        ::std::ostream& os,
        const ParametrizedTestAtGlobalNamespaceStruct& obj) -> ::std::ostream&
    {
        return os << "Test name: '" << obj.m_name_ << "'";
    }
};

class ParametrizedTestAtGlobalNamespaceClass
    : public ::testing::TestWithParam<ParametrizedTestAtGlobalNamespaceStruct>
{
private:
    bool m_bool_;

public:
    ParametrizedTestAtGlobalNamespaceClass()
        : m_bool_(false)
    {
    }

    auto getBool() const -> bool { return m_bool_; }

    void SetUp() override { m_bool_ = GetParam().m_expected_value_; }

    void TearDown() override { m_bool_ = false; }
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
    EXPECT_EQ(getBool(), GetParam().m_expected_value_);
}

// parametrized test at unnamed namespace
namespace
{
struct ParametrizedTestAtUnnamedNamespaceStruct {
    ::std::string m_name_;
    bool m_expected_value_;

    friend auto operator<<(
        ::std::ostream& os,
        const ParametrizedTestAtUnnamedNamespaceStruct& obj) -> ::std::ostream&
    {
        return os << "Test name: '" << obj.m_name_ << "'";
    }
};

class ParametrizedTestAtUnnamedNamespaceClass
    : public ::testing::TestWithParam<ParametrizedTestAtUnnamedNamespaceStruct>
{
private:
    bool m_bool_;

public:
    ParametrizedTestAtUnnamedNamespaceClass()
        : m_bool_(false)
    {
    }

    auto getBool() const -> bool { return m_bool_; }

    void SetUp() override { m_bool_ = GetParam().m_expected_value_; }

    void TearDown() override { m_bool_ = false; }
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
    EXPECT_EQ(getBool(), GetParam().m_expected_value_);
}
}  // namespace

// parametrized test at named namespace
namespace ottest::DummyTest
{
struct ParametrizedTestAtNamedNamespaceStruct {
    ::std::string m_name_;
    bool m_expected_value_;

    friend auto operator<<(
        ::std::ostream& os,
        const ParametrizedTestAtNamedNamespaceStruct& obj) -> ::std::ostream&
    {
        return os << "Test name: '" << obj.m_name_ << "'";
    }
};

class ParametrizedTestAtNamedNamespaceClass
    : public ::testing::TestWithParam<ParametrizedTestAtNamedNamespaceStruct>
{
private:
    bool m_bool_;

public:
    ParametrizedTestAtNamedNamespaceClass()
        : m_bool_(false)
    {
    }

    auto getBool() const -> bool { return m_bool_; }

    void SetUp() override { m_bool_ = GetParam().m_expected_value_; }

    void TearDown() override { m_bool_ = false; }
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
    EXPECT_EQ(getBool(), GetParam().m_expected_value_);
}

}  // namespace ottest::DummyTest
// NOLINTEND(modernize-avoid-c-arrays)
