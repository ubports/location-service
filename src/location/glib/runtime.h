/*
 * Copyright © 2017 Canonical Ltd.
 *
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
#ifndef LOCATION_GLIB_RUNTIME_H_
#define LOCATION_GLIB_RUNTIME_H_

#include <location/visibility.h>
#include <location/glib/shared_object.h>

#include <mutex>
#include <queue>

#include <sys/eventfd.h>
#include <sys/signalfd.h>

#include <gio/gio.h>

namespace location
{
namespace glib
{

class LOCATION_DLL_PUBLIC Runtime
{
public:
    struct WithOwnMainLoop {};

    static Runtime* instance();

    Runtime();
    explicit Runtime(WithOwnMainLoop);
    ~Runtime();

    // redirect_logging interacts with the glib runtime
    // to redirect logging to location::Logging facilities.
    void redirect_logging();

    // executes the underlying event loop and blocks until
    // the runtime is stopped or a SIGINT/SIGTERM is delivered
    // to the application.
    int run();

    // stop requests a Runtime instance to shut down.
    void stop();

    // dispatch executes f in order on the thread that is executing the
    // runtime.
    void dispatch(const std::function<void()>& f);

private:
    static void on_signal_fd_read_finished(
            GObject* source, GAsyncResult* result, gpointer user_data);

    static void on_event_fd_read_finished(
            GObject* source, GAsyncResult* result, gpointer user_data);

    // dispatch puts all known functors in functors_ to execution.
    void on_event_fd_read_finished();
    void on_signal_fd_read_finished();

    GMainLoop* main_loop_;

    int event_fd_;
    int signal_fd_;

    SharedObject<GInputStream> event_fd_input_stream;
    SharedObject<GInputStream> signal_fd_input_stream;

    std::uint64_t event_fd_buffer;
    signalfd_siginfo signal_fd_buffer;

    std::mutex guard_;
    std::queue<std::function<void()>> functors_;

};

}  // namespace glib
}  // namespace location

#endif  // LOCATION_GLIB_RUNTIME_H_
