// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <memory>

namespace ottest
{
namespace ot = opentxs;

class OPENTXS_EXPORT ContactSection : public ::testing::Test
{
public:
    ContactSection();

    const ot::api::session::Client& api_;
    const ot::identity::wot::claim::Section contact_section_;
    const std::shared_ptr<ot::identity::wot::claim::Group> contact_group_;
    const std::shared_ptr<ot::identity::wot::claim::Item> active_contact_item_;
};
}  // namespace ottest
