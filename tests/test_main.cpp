#include "com/ubuntu/location/init_and_shutdown.h"

#include <gtest/gtest.h>

#include <cstdlib>

int main(int argc, char** argv)
{
	// Initialize the location library
	::com::ubuntu::location::init(&argc, &argv);
	::std::atexit(com::ubuntu::location::shutdown);
	// This allows the user to override the flag on the command line.
	::testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}