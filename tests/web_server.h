/*
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */
#ifndef TESTING_WEB_SERVER_H_
#define TESTING_WEB_SERVER_H_

#include <core/posix/exit.h>
#include <core/posix/signal.h>

#include <gtest/gtest.h>

#include <cstdint>

#include <functional>
#include <thread>

#include "mongoose.h"

namespace testing
{
namespace web
{
namespace server
{
// Configuration options for creating a testing web server.
struct Configuration
{
    // The port to expose the web-server on.
    std::uint16_t port;
    // Function that is invoked for individual client requests.
    std::function<int(mg_connection*)> request_handler;
};
}
}
// Returns an executable web-server for the given configuration.
inline std::function<core::posix::exit::Status()> a_web_server(const web::server::Configuration& configuration)
{
    return [configuration]()
    {
        bool terminated = false;

        // Register for SIG_TERM
        auto trap = core::posix::trap_signals_for_all_subsequent_threads({core::posix::Signal::sig_term});
        // On SIG_TERM, we set terminated to false and request a clean shutdown
        // of the polling loop.
        trap->signal_raised().connect([trap, &terminated](core::posix::Signal)
        {
            trap->stop();
            terminated = true;
        });

        struct Context
        {
            static int on_request(mg_connection* conn, mg_event ev)
            {
                auto thiz = static_cast<Context*>(conn->server_param);

                switch (ev)
                {
                case MG_REQUEST:
                    return thiz->handle_request(conn);
                case MG_AUTH:
                    return MG_TRUE;
                default:
                    return MG_FALSE;
                }

                return MG_FALSE;
            }

            int handle_request(mg_connection* conn)
            {
                return configuration.request_handler(conn);
            }

            const testing::web::server::Configuration& configuration;
        } context{configuration};

        std::thread trap_worker
        {
            [trap]()
            {
                trap->run();
            }
        };

        auto server = mg_create_server(&context, Context::on_request);
        // Setup the port on which the server should be exposed.
        mg_set_option(server, "listening_port", std::to_string(configuration.port).c_str());
        // Start the polling loop
        for (;;)
        {
            mg_poll_server(server, 200);

            if (terminated)
                break;
        }

        // Cleanup, and free server instance
        mg_destroy_server(&server);

        if (trap_worker.joinable())
            trap_worker.join();

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };
}
}

#endif // TESTING_WEB_SERVER_H_
