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

#include <location/glib/runtime.h>

#include <location/logging.h>

#include <gio/gunixinputstream.h>

#include <system_error>

namespace
{

location::glib::Runtime* runtime{nullptr};

int signal_fd_or_throw()
{
    sigset_t sigset;
    sigemptyset(&sigset);

    sigaddset(&sigset, SIGINT);
    sigaddset(&sigset, SIGTERM);

    int result = ::signalfd(-1, &sigset, SFD_CLOEXEC | SFD_NONBLOCK);
    if (result == -1) throw std::system_error(errno, std::system_category());
    return result;
}

int event_fd_or_throw()
{
    int result = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (result == -1) throw std::system_error(errno, std::system_category());
    return result;
}

}  // namespace

location::glib::Runtime* location::glib::Runtime::instance()
{
    return runtime;
}

location::glib::Runtime::Runtime()
    : main_loop_{g_main_loop_new(nullptr, false)},
      event_fd_{event_fd_or_throw()},
      signal_fd_{signal_fd_or_throw()},
      event_fd_input_stream{make_shared_object(g_unix_input_stream_new(event_fd_, true))},
      signal_fd_input_stream{make_shared_object(g_unix_input_stream_new(signal_fd_, true))}
{
    g_input_stream_read_async(signal_fd_input_stream.get(), &signal_fd_buffer, sizeof(signal_fd_buffer), G_PRIORITY_LOW,
                              nullptr, Runtime::on_signal_fd_read_finished, this);
    g_input_stream_read_async(event_fd_input_stream.get(), &event_fd_buffer, sizeof(event_fd_buffer), G_PRIORITY_LOW,
                              nullptr, Runtime::on_event_fd_read_finished, this);
    runtime = this;
}

location::glib::Runtime::~Runtime()
{

    g_main_loop_unref(main_loop_);
    runtime = nullptr;
}

int location::glib::Runtime::run()
{
    g_main_loop_run(main_loop_);
    return EXIT_SUCCESS;
}

void location::glib::Runtime::stop()
{
    g_main_loop_quit(main_loop_);
}

void location::glib::Runtime::dispatch(const std::function<void()>& f)
{
    static std::uint64_t value{1};

    std::lock_guard<std::mutex> lg{guard_};
    functors_.push(f);

    if (functors_.size() == 1)
        ::write(event_fd_, &value, sizeof(value));
}

void location::glib::Runtime::on_signal_fd_read_finished(
        GObject* source, GAsyncResult* result, gpointer user_data)
{
    if (auto thiz = static_cast<Runtime*>(user_data))
        thiz->on_signal_fd_read_finished();
}

void location::glib::Runtime::on_event_fd_read_finished(
        GObject* source, GAsyncResult* result, gpointer user_data)
{
    if (auto thiz = static_cast<Runtime*>(user_data))
        thiz->on_event_fd_read_finished();
}

void location::glib::Runtime::on_event_fd_read_finished()
{
    std::queue<std::function<void()>> functors;

    {
        std::lock_guard<std::mutex> lg{guard_};
        std::swap(functors_, functors);
    }

    while (!functors.empty())
    {
        functors.front()();
        functors.pop();
    }

    g_input_stream_read_async(event_fd_input_stream.get(), &event_fd_buffer, sizeof(event_fd_buffer), G_PRIORITY_LOW,
                              nullptr, Runtime::on_event_fd_read_finished, this);
}

void location::glib::Runtime::on_signal_fd_read_finished()
{
    if (signal_fd_buffer.ssi_signo == SIGINT || signal_fd_buffer.ssi_signo == SIGTERM)
        g_main_loop_quit(main_loop_);
    else
        g_input_stream_read_async(signal_fd_input_stream.get(), &signal_fd_buffer, sizeof(signal_fd_buffer), G_PRIORITY_LOW,
                                  nullptr, Runtime::on_signal_fd_read_finished, this);
}
