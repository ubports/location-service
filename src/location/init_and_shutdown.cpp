#include <location/init_and_shutdown.h>
#include <location/logging.h>

#include <gflags/gflags.h>

void location::init(int* argc, char*** argv)
{
    static const bool remove_parsed_flags = true;
    google::ParseCommandLineFlags(argc, argv, remove_parsed_flags);
}

void location::shutdown()
{
    google::ShutDownCommandLineFlags();
}
