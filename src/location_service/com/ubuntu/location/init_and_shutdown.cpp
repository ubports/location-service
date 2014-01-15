#include <com/ubuntu/location/init_and_shutdown.h>
#include <com/ubuntu/location/logging.h>

#include <gflags/gflags.h>

namespace cul = com::ubuntu::location;

namespace
{
struct LoggingInitializer
{
    LoggingInitializer()
    {
        google::InitGoogleLogging("com.ubuntu.location");
        FLAGS_v = 10;
    }

    ~LoggingInitializer()
    {
        google::ShutdownGoogleLogging();
    }
};

static LoggingInitializer logging_initializer;
}

void cul::init(int* argc, char*** argv)
{
    static const bool remove_parsed_flags = true;
    google::ParseCommandLineFlags(argc, argv, remove_parsed_flags);
}

void cul::shutdown()
{
    google::ShutDownCommandLineFlags();
}
