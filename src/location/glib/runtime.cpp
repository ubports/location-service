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

#include <boost/core/ignore_unused.hpp>

#include <gio/gunixinputstream.h>
#include <glib.h>

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

    if (sigprocmask(SIG_BLOCK, &sigset, NULL) < 0)
        throw std::system_error(errno, std::system_category());

    int result = ::signalfd(-1, &sigset, SFD_CLOEXEC | SFD_NONBLOCK);
    if (result == -1)
        throw std::system_error(errno, std::system_category());

    return result;
}

int event_fd_or_throw()
{
    int result = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (result == -1)
        throw std::system_error(errno, std::system_category());

    return result;
}

void handle_log_message(const gchar* log_domain,
                        GLogLevelFlags log_level,
                        const gchar* message,
                        gpointer user_data)
{
    boost::ignore_unused(log_domain, user_data);

    if (log_level & G_LOG_LEVEL_ERROR)
        LOG(ERROR) << message;
    else if (log_level & G_LOG_LEVEL_CRITICAL)
        LOG(ERROR) << message;
    else if (log_level & G_LOG_LEVEL_WARNING)
        LOG(WARNING) << message;
    else if (log_level & G_LOG_LEVEL_MESSAGE)
        LOG(INFO) << message;
    else if (log_level & G_LOG_LEVEL_INFO)
        LOG(INFO) << message;
    else if (log_level & G_LOG_LEVEL_DEBUG)
        VLOG(1) << message;
}

}  // namespace

location::glib::Runtime* location::glib::Runtime::instance()
{
    return runtime;
}

location::glib::Runtime::Runtime()
    : main_loop_{nullptr},
      event_fd_{event_fd_or_throw()},
      signal_fd_{-1},
      event_fd_input_stream{make_shared_object(g_unix_input_stream_new(event_fd_, true))}
{
    g_input_stream_read_async(event_fd_input_stream.get(), &event_fd_buffer, sizeof(event_fd_buffer), G_PRIORITY_LOW,
                              nullptr, Runtime::on_event_fd_read_finished, this);

    runtime = this;
}

location::glib::Runtime::Runtime(WithOwnMainLoop)
    : main_loop_{g_main_loop_new(nullptr, false)},
      event_fd_{event_fd_or_throw()},
      signal_fd_{signal_fd_or_throw()},
      event_fd_input_stream{make_shared_object(g_unix_input_stream_new(event_fd_, true))},
      signal_fd_input_stream{make_shared_object(g_unix_input_stream_new(signal_fd_, true))}
{
    g_input_stream_read_async(event_fd_input_stream.get(), &event_fd_buffer, sizeof(event_fd_buffer), G_PRIORITY_LOW,
                              nullptr, Runtime::on_event_fd_read_finished, this);

    g_input_stream_read_async(signal_fd_input_stream.get(), &signal_fd_buffer, sizeof(signal_fd_buffer), G_PRIORITY_LOW,
                              nullptr, Runtime::on_signal_fd_read_finished, this);
    runtime = this;
}

location::glib::Runtime::~Runtime()
{
    if (main_loop_)
        g_main_loop_unref(main_loop_);
    runtime = nullptr;
}

void location::glib::Runtime::redirect_logging()
{
    g_log_set_default_handler(handle_log_message, nullptr);
}

int location::glib::Runtime::run()
{
    if (!main_loop_)
        throw std::runtime_error{"Missing main loop"};
    g_main_loop_run(main_loop_);

    return EXIT_SUCCESS;
}

void location::glib::Runtime::stop()
{
    if (!main_loop_)
        throw std::runtime_error{"Missing main loop"};
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
    boost::ignore_unused(source);

    if (auto thiz = static_cast<Runtime*>(user_data))
    {
        if (sizeof(thiz->signal_fd_buffer) == g_input_stream_read_finish(thiz->signal_fd_input_stream.get(), result, nullptr))
        {
            thiz->on_signal_fd_read_finished();
        }
    }
}

void location::glib::Runtime::on_event_fd_read_finished(
        GObject* source, GAsyncResult* result, gpointer user_data)
{
    boost::ignore_unused(source);

    if (auto thiz = static_cast<Runtime*>(user_data))
    {
        if (sizeof(thiz->event_fd_buffer) == g_input_stream_read_finish(thiz->event_fd_input_stream.get(), result, nullptr))
        {
            thiz->on_event_fd_read_finished();
        }
    }
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
