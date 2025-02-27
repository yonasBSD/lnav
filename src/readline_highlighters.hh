/**
 * Copyright (c) 2014, Timothy Stack
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
 *
 * @file readline_highlighters.hh
 */

#ifndef readline_highlighters_hh
#define readline_highlighters_hh

#include "base/attr_line.hh"
#include "text_format.hh"

void readline_regex_highlighter(attr_line_t& line, std::optional<int> x);

void readline_command_highlighter(attr_line_t& line, std::optional<int> x);

void readline_sqlite_highlighter_int(attr_line_t& line,
                                     std::optional<int> x,
                                     line_range sub);
void readline_sqlite_highlighter(attr_line_t& line, std::optional<int> x);

void readline_shlex_highlighter_int(attr_line_t& al,
                                    std::optional<int> x,
                                    line_range sub);
void readline_shlex_highlighter(attr_line_t& line, std::optional<int> x);

void readline_lnav_highlighter(attr_line_t& line, std::optional<int> x);

void highlight_syntax(text_format_t tf, attr_line_t& al, std::optional<int> x);

#endif
