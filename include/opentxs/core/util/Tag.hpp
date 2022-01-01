// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <map>
#include <memory>
#include <memory_resource>
#include <string>
#include <vector>

namespace opentxs
{
class Tag;

using TagPtr = std::shared_ptr<Tag>;
using map_strings = std::pmr::map<std::string, std::string>;
using vector_tags = std::pmr::vector<TagPtr>;

class Tag
{
private:
    std::string name_;
    std::string text_;
    map_strings attributes_;
    vector_tags tags_;

public:
    auto name() const -> const std::string& { return name_; }
    auto text() const -> const std::string& { return text_; }
    auto attributes() const -> const map_strings& { return attributes_; }
    auto tags() const -> const vector_tags& { return tags_; }

    void set_name(const std::string& str_name) { name_ = str_name; }
    void set_text(const std::string& str_text) { text_ = str_text; }

    void add_attribute(
        const std::string& str_att_name,
        const std::string& str_att_value);

    void add_attribute(
        const std::string& str_att_name,
        const char* sz_att_value);

    void add_tag(TagPtr& tag_input);
    void add_tag(
        const std::string& str_tag_name,
        const std::string& str_tag_value);

    Tag(const std::string& str_name);

    Tag(const std::string& str_name, const std::string& str_text);

    Tag(const std::string& str_name, const char* sztext);

    void output(std::string& str_output) const;
    void outputXML(std::string& str_output) const;
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
