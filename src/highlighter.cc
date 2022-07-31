/**
 * Copyright (c) 2020, Timothy Stack
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * * Neither the name of Timothy Stack nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "highlighter.hh"

#include "config.h"

highlighter&
highlighter::operator=(const highlighter& other)
{
    if (this == &other) {
        return *this;
    }

    this->h_pattern = other.h_pattern;
    this->h_fg = other.h_fg;
    this->h_bg = other.h_bg;
    this->h_role = other.h_role;
    this->h_regex = other.h_regex;
    this->h_format_name = other.h_format_name;
    this->h_attrs = other.h_attrs;
    this->h_text_formats = other.h_text_formats;
    this->h_nestable = other.h_nestable;

    return *this;
}

void
highlighter::annotate_capture(attr_line_t& al, const line_range& lr) const
{
    auto& vc = view_colors::singleton();
    auto& sa = al.get_attrs();

    if (!(lr.lr_start < lr.lr_end
          && (this->h_nestable
              || find_string_attr_containing(sa, &VC_STYLE, lr) == sa.end())))
    {
        return;
    }
    if (!this->h_fg.empty()) {
        sa.emplace_back(lr,
                        VC_FOREGROUND.value(
                            vc.match_color(this->h_fg)
                                .value_or(view_colors::MATCH_COLOR_DEFAULT)));
    }
    if (!this->h_bg.empty()) {
        sa.emplace_back(lr,
                        VC_BACKGROUND.value(
                            vc.match_color(this->h_bg)
                                .value_or(view_colors::MATCH_COLOR_DEFAULT)));
    }
    if (this->h_role != role_t::VCR_NONE) {
        sa.emplace_back(lr, VC_ROLE.value(this->h_role));
    }
    if (!this->h_attrs.empty()) {
        sa.emplace_back(lr, VC_STYLE.value(this->h_attrs));
    }
}

void
highlighter::annotate(attr_line_t& al, int start) const
{
    auto& vc = view_colors::singleton();
    const auto& str = al.get_string();
    auto& sa = al.get_attrs();
    auto sf = string_fragment::from_substr(
        str, start, std::min(size_t{8192}, str.size()));

    pcre_context_static<60> pc;
    pcre_input pi(sf);

    while (this->h_regex->match(pc, pi)) {
        if (pc.get_count() == 1) {
            line_range lr{start + pc.all()->c_begin, start + pc.all()->c_end};

            this->annotate_capture(al, lr);
        } else {
            for (size_t lpc = 0; lpc < pc.get_count() - 1; lpc++) {
                line_range lr{start + pc[lpc]->c_begin, start + pc[lpc]->c_end};
                const auto* name = this->h_regex->name_for_capture(lpc);

                if (name != nullptr && name[0]) {
                    auto ident_attrs = vc.attrs_for_ident(name);

                    ident_attrs.ta_attrs |= this->h_attrs.ta_attrs;
                    if (this->h_role != role_t::VCR_NONE) {
                        auto role_attrs = vc.attrs_for_role(this->h_role);

                        ident_attrs.ta_attrs |= role_attrs.ta_attrs;
                    }
                    sa.emplace_back(lr, VC_STYLE.value(ident_attrs));
                } else {
                    this->annotate_capture(al, lr);
                }
            }
        }
    }
}
