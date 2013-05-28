#include "com/ubuntu/location/logging.h"

namespace
{
struct LoggingInitializer
{
    LoggingInitializer()
    {
        google::InitGoogleLogging("com.ubuntu.location");
    }

    ~LoggingInitializer()
    {
        google::ShutdownGoogleLogging();
    }
};

static LoggingInitializer logging_initializer;
}

