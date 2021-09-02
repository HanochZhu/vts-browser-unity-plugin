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

#include "../logfile.hpp"

namespace dbglog {

logger_file::logger_file()
    : use_file_(false), fd_(::open("/dev/null", O_WRONLY))
{
    if (-1 == fd_) {
        throw std::runtime_error
            ("Cannot open /dev/null for log file. Oops.");
    }
}

logger_file::~logger_file() {
    if (-1 == fd_) {
        return;
    }

    ::close(fd_);

    for (auto fd : ties_) {
        ::close(fd);
    }
}

bool logger_file::log_file(const std::string &filename
              , ::mode_t mode)
{
    boost::mutex::scoped_lock guard(m_);
    if (filename.empty()) {
        if (!open_file("/dev/null", fd_, mode)) {
            return false;
        }
        use_file_ = false;
        retie(); // back to /dev/null to allow log file to be closed
    } else {
        if (!open_file(filename, fd_, mode)) {
            return false;
        }
        use_file_ = true;
        retie(); // point ties to freshly open log file
    }

    filename_ = filename;
    return true;
}

bool logger_file::log_file_truncate() {
    boost::mutex::scoped_lock guard(m_);
    if (filename_.empty()) {
        return false;
    }

    if (::ftruncate(fd_, 0) == -1) {
        std::cerr << "Error truncating log file <" << filename_
                  << ">: " << errno << std::endl;
        return false;
    }

    return true;
}

bool logger_file::tie(int fd, bool remember) {
    boost::mutex::scoped_lock guard(m_);

    // check for duplicate
    if (ties_.find(fd) != ties_.end()) {
        // already tied -> fine
        return true;
    }

    // TODO: add untie function + remember fd to be re-tied on file reopen
    if (-1 == safeDup2(fd_, fd)) {
        std::cerr << "Error dupplicating fd(" << fd_ << ") to fd("
                  << fd << "): " << errno << std::endl;
        return false;
    }

    // remember fd
    if (remember) {
        ties_.insert(fd);
    }
    return true;
}

bool logger_file::untie(int fd, const std::string &path
           , ::mode_t mode)
{
    boost::mutex::scoped_lock guard(m_);

    // check for existence
    if (ties_.find(fd) == ties_.end()) {
        // not tied -> error
        return false;
    }

    // point fd to something else
    if (!open_file(path, fd, mode)) {
        return false;
    }

    // forget fd
    ties_.erase(fd);
    return true;
}

bool logger_file::log_file_owner(uid_t owner, gid_t group) {
    if (fd_ < 0) { return false; }
    if (-1 == ::fchown(fd_, owner, group)) {
        return false;
    }
    return true;
}

bool logger_file::closeOnExec(bool value)
{
    // no log file -> fine
    if (fd_ < 0) { return true; }

    int flags(::fcntl(fd_, F_GETFD, 0));
    if (flags == -1) { return false; }
    if (value) {
        flags |= FD_CLOEXEC;
    } else {
        flags &= ~FD_CLOEXEC;
    }

    if (::fcntl(fd_, F_SETFD, flags) == -1) {
        return false;
    }
    return true;
}

bool logger_file::write_file(const char *data, size_t left) {
    if (!use_file_) {
        return false;
    }

    while (left) {
        ssize_t written(TEMP_FAILURE_RETRY(::write(fd_, data, left)));
        if (-1 == written) {
            std::cerr << "Error writing to log file: "
                      << errno << std::endl;
            break;
        }
        left -= written;
        data += written;
    }
    return true;
}

bool logger_file::open_file(const std::string &filename, int dest
               , ::mode_t mode)
{
    class file_closer {
    public:
        file_closer(int fd) : fd_(fd) {}
        ~file_closer() {
            if (-1 == fd_) {
                return;
            }

            ::close(fd_);
        }

        operator int() {
            return fd_;
        }

    private:
        int fd_;
    } f(::open(filename.c_str(), O_WRONLY | O_CREAT | O_APPEND, mode));

    if (-1 == f) {
        std::cerr << "Error opening log file <" << filename
                  << ">: " << errno << std::endl;
        return false;
    }

    if (-1 == safeDup2(f, dest)) {
        std::cerr << "Error dupplicating fd(" << f << ") to fd("
                  << dest << "): " << errno << std::endl;
        return false;
    }

    return true;
}

void logger_file::retie() {
    // retie all tied file descriptors
    for (auto fd : ties_) {
        if (-1 == safeDup2(fd_, fd)) {
            std::cerr << "Error dupplicating fd(" << fd_ << ") to fd("
                      << fd << "): " << errno << std::endl;
        }
    }
}

int logger_file::safeDup2(int oldfd, int newfd) {
    for (;;) {
        int res(::dup2(oldfd, newfd));
        if (res != -1) {
            // OK
            return res;
        }

        // some error
        switch (res) {
        case EBUSY:
            // race condition between open and dup -> try again
        case EINTR:
            // interrupted -> try again
            continue;
        }

        // fatal error
        return res;
    }
}

} // namespace dbglog
