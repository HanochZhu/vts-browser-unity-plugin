
#include <vector>
#include <fstream>
#include <functional>
#include <unistd.h> // usleep

#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>

#include "dbglog/dbglog.hpp"

#include "utility/buildsys.hpp"
#include "utility/gccversion.hpp"

#include "service/cmdline.hpp"

#include "http/http.hpp"
#include "http/resourcefetcher.hpp"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

std::atomic<int> active;
std::atomic<int> succeded;
std::atomic<int> finished;
std::atomic<int> started;

class Task
{
public:
    Task(const std::string &url) : query(url)
    {
        active++;
        started++;
        query.timeout(5000);
    }

    ~Task()
    {
        active--;
    }

    void done(http::ResourceFetcher::MultiQuery &&queries)
    {
        finished++;
        http::ResourceFetcher::Query &q = *queries.begin();
        if (q.exc())
        {
            try
            {
                std::rethrow_exception(q.exc());
            }
            catch(std::exception &e)
            {
                LOG(err3) << "exception: " << e.what();
            }
            catch(...)
            {
                LOG(err3) << "unknown exception";
            }
        }
        else if (q.valid())
        {
            succeded++;
            const http::ResourceFetcher::Query::Body &body = q.get();
            LOG(info3) << "Downloaded: '" << q.location()
                       << "', size: " << body.data.length();
        }
        else
            LOG(err3) << "Failed: " << q.location()
                      << ", http code: " << q.ec().value();
    }

    http::ResourceFetcher::Query query;
};

class Test : public service::Cmdline {
public:
    Test()
        : service::Cmdline("http-clienttest", BUILD_TARGET_VERSION)
        , targetDownloads_(100)
        , threadCount_(2)
    {}

private:
    virtual void configuration(po::options_description &cmdline
                               , po::options_description &config
                               , po::positional_options_description &pd)
        UTILITY_OVERRIDE;

    virtual void configure(const po::variables_map &vars)
        UTILITY_OVERRIDE;

    virtual bool help(std::ostream &out, const std::string &what) const
        UTILITY_OVERRIDE
    {
        if (what.empty()) {
            out << R"RAW(http-clienttest
usage
    http-clienttest [target-downloads-count [urls-file-path]] [OPTIONS]

)RAW";
        }
        return false;
    }

    virtual int run() UTILITY_OVERRIDE;

    std::size_t targetDownloads_;
    boost::optional<fs::path> urls_;
    std::size_t threadCount_;
};


void Test::configuration(po::options_description &cmdline
                         , po::options_description&
                         , po::positional_options_description &pd)
{
    cmdline.add_options()
        ("count", po::value(&targetDownloads_)
         ->required()->default_value(targetDownloads_)
         , "Number of donwloads to perform.")
         ("urls", po::value<fs::path>()
          , "Path to URL file")
        ("threadCount", po::value(&threadCount_)
         ->default_value(threadCount_)->required()
         , "Number of HTTP threads (and CURL clients).")
         ;

    pd.add("count", 1).add("urls", 1);
}

void Test::configure(const po::variables_map &vars)
{
    if (vars.count("urls")) {
        urls_ = vars["urls"].as<fs::path>();
    }
}

int Test::run()
{
    std::vector<std::string> urls({
        "https://www.melown.com/",
        "https://www.melown.com/tutorials.html",
        "https://www.melown.com/blog.html",
    });

    if (urls_) {
        LOG(info4) << "Loading urls from file.";
        std::string line;
        std::ifstream f(urls_->string());
        if (f.is_open()) {
            urls.clear();
            while (std::getline(f,line)) {
                if (!line.empty()) {
                    urls.push_back(line);
                }
            }
            f.close();
        } else {
            LOG(warn4) << "Failed to open specified file.";
        }
    }
    LOG(info4) << "Will download from " << urls.size() << " urls.";

    http::Http htt;
    http::ResourceFetcher fetcher(htt.fetcher());

    {
        http::ContentFetcher::Options options;
        options.maxTotalConections = 10;
        options.pipelining = 2;
        htt.startClient(threadCount_, &options);
    }

    for (unsigned long i = 0; i < targetDownloads_; i++)
    {
        while (active > 25)
            usleep(1000);
        auto t = std::make_shared<Task>(urls[rand() % urls.size()]);
        fetcher.perform(t->query, std::bind(&Task::done, t,
                                            std::placeholders::_1));
    }

    LOG(info3) << "Waiting for threads to stop.";
    htt.stop();

    LOG(info4) << "Client stopped"
               << ", downloads started: " << started
               << ", finished: " << finished
               << ", succeded: " << succeded
               << ".";

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    return Test()(argc, argv);
}
