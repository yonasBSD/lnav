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
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @file date_time_scanner.cc
 */

#include "config.h"

#include "date_time_scanner.hh"
#include "ptimec.hh"

size_t date_time_scanner::ftime(char *dst, size_t len, const exttm &tm) const
{
    off_t off = 0;

    PTIMEC_FORMATS[this->dts_fmt_lock].pf_ffunc(dst, off, len, tm);

    return (size_t) off;
}

bool next_format(const char * const fmt[], int &index, int &locked_index)
{
    bool retval = true;

    if (locked_index == -1) {
        index += 1;
        if (fmt[index] == nullptr) {
            retval = false;
        }
    }
    else if (index == locked_index) {
        retval = false;
    }
    else {
        index = locked_index;
    }

    return retval;
}

const char *date_time_scanner::scan(const char *time_dest,
                                    size_t time_len,
                                    const char * const time_fmt[],
                                    struct exttm *tm_out,
                                    struct timeval &tv_out,
                                    bool convert_local)
{
    int  curr_time_fmt = -1;
    bool found         = false;
    const char *retval = nullptr;

    if (!time_fmt) {
        time_fmt = PTIMEC_FORMAT_STR;
    }

    while (next_format(time_fmt,
                       curr_time_fmt,
                       this->dts_fmt_lock)) {
        *tm_out = this->dts_base_tm;
        tm_out->et_flags = 0;
        if (time_len > 1 &&
            time_dest[0] == '+' &&
            isdigit(time_dest[1])) {
            char time_cp[time_len + 1];
            int gmt_int, off;

            retval = nullptr;
            memcpy(time_cp, time_dest, time_len);
            time_cp[time_len] = '\0';
            if (sscanf(time_cp, "+%d%n", &gmt_int, &off) == 1) {
                time_t gmt = gmt_int;

                if (convert_local && this->dts_local_time) {
                    localtime_r(&gmt, &tm_out->et_tm);
#ifdef HAVE_STRUCT_TM_TM_ZONE
                    tm_out->et_tm.tm_zone = nullptr;
#endif
                    tm_out->et_tm.tm_isdst = 0;
                    gmt = tm2sec(&tm_out->et_tm);
                }
                tv_out.tv_sec = gmt;
                tv_out.tv_usec = 0;
                tm_out->et_flags = ETF_DAY_SET|ETF_MONTH_SET|ETF_YEAR_SET|ETF_MACHINE_ORIENTED|ETF_EPOCH_TIME;

                this->dts_fmt_lock = curr_time_fmt;
                this->dts_fmt_len = off;
                retval = time_dest + off;
                found = true;
                break;
            }
        }
        else if (time_fmt == PTIMEC_FORMAT_STR) {
            ptime_func func = PTIMEC_FORMATS[curr_time_fmt].pf_func;
            off_t off = 0;

#ifdef HAVE_STRUCT_TM_TM_ZONE
            if (!this->dts_keep_base_tz) {
                tm_out->et_tm.tm_zone = nullptr;
            }
#endif
            if (func(tm_out, time_dest, off, time_len)) {
                retval = &time_dest[off];

                if (tm_out->et_tm.tm_year < 70) {
                    tm_out->et_tm.tm_year = 80;
                }
                if (convert_local &&
                    (this->dts_local_time || tm_out->et_flags & ETF_EPOCH_TIME)) {
                    time_t gmt = tm2sec(&tm_out->et_tm);

                    this->to_localtime(gmt, *tm_out);
                }
                tv_out.tv_sec = tm2sec(&tm_out->et_tm);
                tv_out.tv_usec = tm_out->et_nsec / 1000;
                secs2wday(tv_out, &tm_out->et_tm);

                this->dts_fmt_lock = curr_time_fmt;
                this->dts_fmt_len  = retval - time_dest;

                found = true;
                break;
            }
        }
        else {
            off_t off = 0;

#ifdef HAVE_STRUCT_TM_TM_ZONE
            if (!this->dts_keep_base_tz) {
                tm_out->et_tm.tm_zone = nullptr;
            }
#endif
            if (ptime_fmt(time_fmt[curr_time_fmt], tm_out, time_dest, off, time_len) &&
                (time_dest[off] == '.' || time_dest[off] == ',' || off == (off_t)time_len)) {
                retval = &time_dest[off];
                if (tm_out->et_tm.tm_year < 70) {
                    tm_out->et_tm.tm_year = 80;
                }
                if (convert_local &&
                    (this->dts_local_time || tm_out->et_flags & ETF_EPOCH_TIME)) {
                    time_t gmt = tm2sec(&tm_out->et_tm);

                    this->to_localtime(gmt, *tm_out);
#ifdef HAVE_STRUCT_TM_TM_ZONE
                    tm_out->et_tm.tm_zone = nullptr;
#endif
                    tm_out->et_tm.tm_isdst = 0;
                }

                tv_out.tv_sec = tm2sec(&tm_out->et_tm);
                tv_out.tv_usec = tm_out->et_nsec / 1000;
                secs2wday(tv_out, &tm_out->et_tm);

                this->dts_fmt_lock = curr_time_fmt;
                this->dts_fmt_len  = retval - time_dest;

                found = true;
                break;
            }
        }
    }

    if (!found) {
        retval = nullptr;
    }

    if (retval != nullptr) {
        /* Try to pull out the milli/micro-second value. */
        if (retval[0] == '.' || retval[0] == ',') {
            off_t off = (retval - time_dest) + 1;

            if (ptime_f(tm_out, time_dest, off, time_len)) {
                tv_out.tv_usec = tm_out->et_nsec / 1000;
                this->dts_fmt_len += 7;
                retval += 7;
            }
            else if (ptime_L(tm_out, time_dest, off, time_len)) {
                tv_out.tv_usec = tm_out->et_nsec / 1000;
                this->dts_fmt_len += 4;
                retval += 4;
            }
        }
    }

    return retval;
}

void date_time_scanner::to_localtime(time_t t, exttm &tm_out)
{
    if (t < (24 * 60 * 60)) {
        // Don't convert and risk going past the epoch.
        return;
    }

    if (t < this->dts_local_offset_valid ||
        t >= this->dts_local_offset_expiry) {
        time_t new_gmt;

        localtime_r(&t, &tm_out.et_tm);
#ifdef HAVE_STRUCT_TM_TM_ZONE
        tm_out.et_tm.tm_zone = nullptr;
#endif
        tm_out.et_tm.tm_isdst = 0;

        new_gmt = tm2sec(&tm_out.et_tm);
        this->dts_local_offset_cache = t - new_gmt;
        this->dts_local_offset_valid = t;
        this->dts_local_offset_expiry = t + (EXPIRE_TIME - 1);
        this->dts_local_offset_expiry -=
            this->dts_local_offset_expiry % EXPIRE_TIME;
    }
    else {
        time_t adjust_gmt = t - this->dts_local_offset_cache;
        gmtime_r(&adjust_gmt, &tm_out.et_tm);
    }
}
