/**
 * Copyright (c) 2017 Melown Technologies SE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * *  Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "mask.hpp"
#include "level.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_as.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

#include <stdexcept>

namespace dbglog {

namespace detail {

std::string mask2string(unsigned int mask, dbglog::level l)
{
    switch (mask & l) {
    case debug: return "D";

    case info1: return "I1";
    case info2: return "I2";
    case info3: return "I3";
    case info4: return "I4";

    case warn1: return "W1";
    case warn2: return "W2";
    case warn3: return "W3";
    case warn4: return "W4";

    case err1: return "E1";
    case err2: return "E2";
    case err3: return "E3";
    case err4: return "E4";

    default: return "";
    }
}

} // namespace detail

std::string mask::as_string() const
{
    unsigned int m(mask_);
    if (dbglog::none == m) {
        return "NONE";
    } else if (dbglog::all == m) {
            return "ALL";
    }

    return std::string
        (detail::mask2string(m, dbglog::debug)
         + detail::mask2string(m, dbglog::info1)
         + detail::mask2string(m, dbglog::warn1)
         + detail::mask2string(m, dbglog::err1));
}

void mask::from_string(const std::string &str)
{
    using boost::spirit::qi::string;
    using boost::spirit::qi::char_;
    using boost::spirit::qi::parse;
    using boost::spirit::qi::lexeme;
    using boost::spirit::qi::_1;
    using boost::spirit::ascii::space;
    using boost::spirit::as_string;
    using boost::phoenix::ref;

    unsigned int m(0);

    typedef boost::spirit::qi::rule<std::string::const_iterator> rule;

    rule default_(string("DEFAULT")
                  [boost::phoenix::ref(m) = dbglog::default_]);
    rule none(string("NONE")[boost::phoenix::ref(m) = dbglog::none]);
    rule all(string("ALL")[boost::phoenix::ref(m) = dbglog::all]);
    rule verbose(string("VERBOSE")[boost::phoenix::ref(m) = dbglog::verbose]);
    rule noDebug(string("ND")
                    [boost::phoenix::ref(m) = dbglog::noDebug]);

    rule debug(char_('D')[boost::phoenix::ref(m) |= dbglog::debug]);

    rule info1(string("I1")[boost::phoenix::ref(m) |= dbglog::info1]);
    rule info2(string("I2")[boost::phoenix::ref(m) |= dbglog::info2]);
    rule info3(string("I3")[boost::phoenix::ref(m) |= dbglog::info3]);
    rule info4(string("I4")[boost::phoenix::ref(m) |= dbglog::info4]);

    rule warn1(string("W1")[boost::phoenix::ref(m) |= dbglog::warn1]);
    rule warn2(string("W2")[boost::phoenix::ref(m) |= dbglog::warn2]);
    rule warn3(string("W3")[boost::phoenix::ref(m) |= dbglog::warn3]);
    rule warn4(string("W4")[boost::phoenix::ref(m) |= dbglog::warn4]);

    rule err1(string("E1")[boost::phoenix::ref(m) |= dbglog::err1]);
    rule err2(string("E2")[boost::phoenix::ref(m) |= dbglog::err2]);
    rule err3(string("E3")[boost::phoenix::ref(m) |= dbglog::err3]);
    rule err4(string("E4")[boost::phoenix::ref(m) |= dbglog::err4]);

    rule grammar(default_ | all | none | verbose | noDebug
                 | +(debug | info1 | info2 | info3 | info4
                     | warn1 | warn2 | warn3 | warn4
                     | err1 | err2 | err3 | err4));

    std::string::const_iterator first(str.begin());
    std::string::const_iterator last(str.end());
    if (!parse(first, last, grammar) || (first != last)) {
        throw std::runtime_error("Bad mask syntax.");
    }

    mask_ = m;
}

mask max(const mask &l, const mask &r)
{
    // maximum of both masks -> negative minimum of negative masks
    return ~(~l.get() | ~r.get());
}

mask min(const mask &l, const mask &r)
{
    // minimum of both masks -> bitwise-OR of both masks
    return l.get() | r.get();
}

} // namespace dbglog
