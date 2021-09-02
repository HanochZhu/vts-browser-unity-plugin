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

#ifndef shared_dbglog_sink_hpp_included_
#define shared_dbglog_sink_hpp_included_

#include <string>
#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include "level.hpp"

namespace dbglog {

class Sink : boost::noncopyable {
public:
    typedef boost::shared_ptr<Sink> pointer;

    typedef std::vector<pointer> list;

    Sink(const mask &mask, const std::string &name)
        : shared_mask_(false), mask_(~mask.get()), name_(name)
    {}

    virtual ~Sink() {}

    inline bool check_level(level l) const {
        return !(mask_ & l) || (l == fatal);
    }

    virtual void write(const std::string &line) = 0;

    const std::string& name() const { return name_; }

    void set_mask(const mask &m) {  mask_ = ~m.get();  }

    void set_mask(unsigned int m) { mask_ = ~m; }

    unsigned int get_mask() const { return ~mask_; }

    std::string get_mask_string() const {
        return mask(~mask_).as_string();
    }

    bool shared_mask() const { return shared_mask_; }

    void shared_mask(bool v) { shared_mask_ = v; }

private:
    bool shared_mask_;
    unsigned int mask_;
    const std::string name_;
};

} // namespace dbglog

#endif // shared_dbglog_sink_hpp_included_
