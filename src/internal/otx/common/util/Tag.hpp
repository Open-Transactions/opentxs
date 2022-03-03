// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <algorithm>
#include <memory>

#include "opentxs/Version.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs
{
class Tag;

using TagPtr = std::shared_ptr<Tag>;
using map_strings = UnallocatedMap<UnallocatedCString, UnallocatedCString>;
using vector_tags = UnallocatedVector<TagPtr>;

class Tag
{
private:
    UnallocatedCString name_;
    UnallocatedCString text_;
    map_strings attributes_;
    vector_tags tags_;

public:
    auto name() const -> const UnallocatedCString& { return name_; }
    auto text() const -> const UnallocatedCString& { return text_; }
    auto attributes() const -> const map_strings& { return attributes_; }
    auto tags() const -> const vector_tags& { return tags_; }

    void set_name(const UnallocatedCString& str_name) { name_ = str_name; }
    void set_text(const UnallocatedCString& str_text) { text_ = str_text; }

    void add_attribute(
        const UnallocatedCString& str_att_name,
        const UnallocatedCString& str_att_value);

    void add_attribute(
        const UnallocatedCString& str_att_name,
        const char* sz_att_value);

    void add_tag(TagPtr& tag_input);
    void add_tag(
        const UnallocatedCString& str_tag_name,
        const UnallocatedCString& str_tag_value);

    Tag(const UnallocatedCString& str_name);

    Tag(const UnallocatedCString& str_name, const UnallocatedCString& str_text);

    Tag(const UnallocatedCString& str_name, const char* sztext);

    void output(UnallocatedCString& str_output) const;
    void outputXML(UnallocatedCString& str_output) const;
};

}  // namespace opentxs

/*


 <HOME>

 <NICKNAME>Old Farmer's Ranch</NICKNAME>

 <ADDRESS
  number="6801"
  street1="Old Ranch Road"
  street2="unit 2567"
  postalcode="99781"
  city="Cracked Rock"
  state="UT"
  country="USA"
 />

 </HOME>


 */
