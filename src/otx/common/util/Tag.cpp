// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "internal/otx/common/util/Tag.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "opentxs/util/Container.hpp"

namespace opentxs
{
void Tag::add_attribute(
    const UnallocatedCString& str_att_name,
    const char* sz_att_value)
{
    UnallocatedCString str_temp(sz_att_value);

    add_attribute(str_att_name, str_temp);
}

void Tag::add_attribute(
    const UnallocatedCString& str_att_name,
    const UnallocatedCString& str_att_value)
{
    std::pair<UnallocatedCString, UnallocatedCString> temp =
        std::make_pair(str_att_name, str_att_value);

    attributes_.insert(temp);
}

void Tag::output(UnallocatedCString& str_output) const
{
    outputXML(str_output);
}

void Tag::outputXML(UnallocatedCString& str_output) const
{
    str_output += ("<" + name_);

    if (!attributes_.empty()) {
        for (auto& kv : attributes_) {
            str_output += ("\n " + kv.first + "=\"" + kv.second + "\"");
        }
    }

    if (text_.empty() && tags_.empty()) {
        str_output += " />\n";
    } else {
        str_output += ">\n";

        if (!text_.empty()) {
            str_output += text_;
        } else if (!tags_.empty()) {
            for (auto& kv : tags_) { kv->output(str_output); }
        }

        str_output += ("\n</" + name_ + ">\n");
    }
}

void Tag::add_tag(TagPtr& tag_input) { tags_.push_back(tag_input); }

void Tag::add_tag(
    const UnallocatedCString& str_tag_name,
    const UnallocatedCString& str_tag_value)
{
    TagPtr p1 = std::make_shared<Tag>(str_tag_name, str_tag_value);

    add_tag(p1);
}

Tag::Tag(const UnallocatedCString& str_name)
    : name_(str_name)
    , text_()
    , attributes_()
    , tags_()
{
}

Tag::Tag(const UnallocatedCString& str_name, const UnallocatedCString& str_text)
    : name_(str_name)
    , text_(str_text)
    , attributes_()
    , tags_()
{
}

Tag::Tag(const UnallocatedCString& str_name, const char* sztext)
    : name_(str_name)
    , text_(sztext)
    , attributes_()
    , tags_()
{
}
}  // namespace opentxs
