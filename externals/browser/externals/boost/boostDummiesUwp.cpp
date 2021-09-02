
/*
This file contains DUMMY implementation for supposedly unused boost functions.
When any of these functions is called, the application will forcibly terminate.
This is to make you aware of using unsupported functions.
*/

#include "boost/filesystem.hpp"
#include "windows_file_codecvt.hpp"

#include <windows.h>


namespace boost
{
namespace filesystem
{
namespace detail
{

bool create_directories(const path& p, system::error_code* ec)
{
    fprintf(stderr, "create_directories\n");
    std::terminate();
    return false;
}

void rename(const path& old_p, const path& new_p, system::error_code* ec)
{
    fprintf(stderr, "rename\n");
    std::terminate();
}

file_status status(const path&p, system::error_code* ec)
{
    fprintf(stderr, "status\n");
    std::terminate();
    return {};
}

boost::uintmax_t remove_all(const path& p, system::error_code* ec)
{
    fprintf(stderr, "remove_all\n");
    std::terminate();
    return {};
}

}
}
}


std::codecvt_base::result windows_file_codecvt::do_in(std::mbstate_t& state,
    const char* from, const char* from_end, const char*& from_next,
    wchar_t* to, wchar_t* to_end, wchar_t*& to_next) const
{
    UINT codepage = CP_ACP;

    int count;
    if ((count = ::MultiByteToWideChar(codepage, MB_PRECOMPOSED, from,
        from_end - from, to, to_end - to)) == 0)
    {
        return error;  // conversion failed
    }

    from_next = from_end;
    to_next = to + count;
    *to_next = L'\0';
    return ok;
}

std::codecvt_base::result windows_file_codecvt::do_out(std::mbstate_t & state,
    const wchar_t* from, const wchar_t* from_end, const wchar_t*& from_next,
    char* to, char* to_end, char*& to_next) const
{
    UINT codepage = CP_ACP;

    int count;
    if ((count = ::WideCharToMultiByte(codepage, WC_NO_BEST_FIT_CHARS, from,
        from_end - from, to, to_end - to, 0, 0)) == 0)
    {
        return error;  // conversion failed
    }

    from_next = from_end;
    to_next = to + count;
    *to_next = '\0';
    return ok;
}

