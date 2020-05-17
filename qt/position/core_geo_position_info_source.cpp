/*
 * Copyright 2013 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Thomas Voß <thomas.voss@canonical.com>
 */

#include "core_geo_position_info_source.h"

#include <cmath>

#include <QGuiApplication>
#include <QtCore>

#include <ubuntu/application/location/service.h>
#include <ubuntu/application/location/session.h>

#include <ubuntu/application/location/heading_update.h>
#include <ubuntu/application/location/position_update.h>
#include <ubuntu/application/location/velocity_update.h>

struct core::GeoPositionInfoSource::Private
{
    // If an application requests an individual position update with a
    // timeout value of 0, we bump the timeout to the default value of
    // 10 seconds.
    static const unsigned int default_timeout_in_ms = 10 * 1000;

    // Our session flags for connecting to the backend service.
    static const unsigned int empty_creation_flags = 0;
    
    // Translates a backend service error to a QGeoPositionInfoSource::Error
    static QGeoPositionInfoSource::Error ua_location_error_to_qt(UALocationServiceError error);

    // Called for every new position update arriving from the backend service.
    static void processPositionUpdate(
        UALocationPositionUpdate* position,
        void* context);

    // Called for every new heading update arriving from the backend service.
    static void processHeadingUpdate(
        UALocationHeadingUpdate* heading,
        void* context);

    // Called for every new heading update arriving from the backend service.
    static void processVelocityUpdate(
        UALocationVelocityUpdate* velocity,
        void* context);

    // Processes the incoming position update and translates it to Qt world.
    void handlePositionUpdate(UALocationPositionUpdate* position);

    // Processes the incoming heading update and translates it to Qt world.
    void handleHeadingUpdate(UALocationHeadingUpdate* heading);

    // Processes the incoming velocity update and translates it to Qt world.
    void handleVelocityUpdate(UALocationVelocityUpdate* velocity);

    void createLocationServiceSession();
    void destroyLocationServiceSession();

    // Creates a new instance and attempts to connect to the background service.
    // Stores errors in the error member.
    Private(core::GeoPositionInfoSource* parent);
    ~Private();

    core::GeoPositionInfoSource* parent;
    UALocationServiceSession* session;
    QMutex lastKnownPositionGuard;
    QGeoPositionInfo lastKnownPosition;
    QTimer timer;
    QGeoPositionInfoSource::Error error;
};

core::GeoPositionInfoSource::GeoPositionInfoSource(QObject *parent)
        : QGeoPositionInfoSource(parent),
          m_applicationActive(true),
          m_lastReqTimeout(-1),
          m_state(State::stopped),
          d(new Private(this))
{
    d->timer.setSingleShot(true);
    QObject::connect(&d->timer, SIGNAL(timeout()), this, SLOT(timeout()), Qt::DirectConnection);
    // Whenever we receive an update, we stop the timeout timer immediately.
    QObject::connect(this, SIGNAL(positionUpdated(const QGeoPositionInfo&)), &d->timer, SLOT(stop()));
    QObject::connect(qApp, SIGNAL(applicationStateChanged(Qt::ApplicationState)), this, SLOT(applicationStateChanged()));
    qRegisterMetaType<Qt::ApplicationState>("Qt::ApplicationState");
}

void core::GeoPositionInfoSource::applicationStateChanged()
{
    Qt::ApplicationState state = qApp->applicationState();
    if (state == Qt::ApplicationInactive) {
        if (m_applicationActive) {
            int state = m_state;
            stopUpdates();

            m_applicationActive = false;
            if (state == State::one_shot) {
                // Save current time out               
                if (d->timer.isActive()) {
                    m_lastReqTimeout = d->timer.interval();
                    d->timer.stop();
                }
            }
            else if (state == State::running) {
                // Stop continuous updates and suspend
                m_state = State::suspended;
            }
        }
    }
    else if (state == Qt::ApplicationActive) {
        if (!m_applicationActive) {
            m_applicationActive = true;

            // Only restart updates if active before suspending
            if (m_lastReqTimeout > -1) {
                requestUpdate(m_lastReqTimeout);
                m_lastReqTimeout = -1;
            }
            else if (m_state == State::suspended) {
                // Restart continuous updates
                startUpdates();
            }
        }
    }
}


core::GeoPositionInfoSource::~GeoPositionInfoSource()
{
}

void core::GeoPositionInfoSource::setUpdateInterval(int msec)
{
    // We emit our current error state whenever a caller tries to interact
    // with the source although we are in error state.
    if (error() != QGeoPositionInfoSource::NoError)
    {
        Q_EMIT(QGeoPositionInfoSource::error(d->error));
        return;
    }

    (void) msec;
}

void core::GeoPositionInfoSource::setPreferredPositioningMethods(PositioningMethods methods)
{
    // We emit our current error state whenever a caller tries to interact
    // with the source although we are in error state.
    if (error() != QGeoPositionInfoSource::NoError)
    {
        Q_EMIT(QGeoPositionInfoSource::error(d->error));
        return;
    }

    QGeoPositionInfoSource::setPreferredPositioningMethods(methods);
}

QGeoPositionInfo core::GeoPositionInfoSource::lastKnownPosition(bool fromSatellitePositioningMethodsOnly) const
{
    // We emit our current error state whenever a caller tries to interact
    // with the source although we are in error state.
    if (error() != QGeoPositionInfoSource::NoError)
        return QGeoPositionInfo();

    Q_UNUSED(fromSatellitePositioningMethodsOnly);
    QMutexLocker lock(&d->lastKnownPositionGuard);
    return QGeoPositionInfo(d->lastKnownPosition);
}

QGeoPositionInfoSource::PositioningMethods core::GeoPositionInfoSource::supportedPositioningMethods() const
{
    // We emit our current error state whenever a caller tries to interact
    // with the source although we are in error state.
    if (error() != QGeoPositionInfoSource::NoError)
    {
        return QGeoPositionInfoSource::NoPositioningMethods;
    }

    return AllPositioningMethods;
}

void core::GeoPositionInfoSource::startUpdates()
{
    if (d->session == nullptr) {
        d->createLocationServiceSession();
    }

    // We emit our current error state whenever a caller tries to interact
    // with the source although we are in error state.
    if (error() != QGeoPositionInfoSource::NoError)
    {
        Q_EMIT(QGeoPositionInfoSource::error(d->error));
        return;
    }

    ua_location_service_session_start_position_updates(d->session);
    ua_location_service_session_start_heading_updates(d->session);
    ua_location_service_session_start_velocity_updates(d->session);

    if (m_state != State::one_shot)
        m_state = State::running;
}


int core::GeoPositionInfoSource::minimumUpdateInterval() const {
    // We emit our current error state whenever a caller tries to interact
    // with the source although we are in error state.
    if (error() != QGeoPositionInfoSource::NoError)
    {
        return -1;
    }

    return 500;
}

void core::GeoPositionInfoSource::stopUpdates()
{
    ua_location_service_session_stop_position_updates(d->session);
    ua_location_service_session_stop_heading_updates(d->session);
    ua_location_service_session_stop_velocity_updates(d->session);

    m_state = State::stopped;
}

void core::GeoPositionInfoSource::requestUpdate(int timeout)
{
    // We emit our current error state whenever a caller tries to interact
    // with the source although we are in error state.
    if (error() != QGeoPositionInfoSource::NoError)
    {
        Q_EMIT(QGeoPositionInfoSource::error(d->error));
        return;
    }

    if (d->timer.isActive())
    {
        return;
    }

    // Bump the timeout if caller indicates "choose default value".
    if (timeout <= 0)
        timeout = Private::default_timeout_in_ms;

    startUpdates();
    d->timer.start(timeout);
}

void core::GeoPositionInfoSource::timeout()
{
    // Update timeout reached, clean up
    stopUpdates();

    Q_EMIT updateTimeout();
}

QGeoPositionInfoSource::Error core::GeoPositionInfoSource::error() const
{
    return d->error;
}

QGeoPositionInfoSource::Error core::GeoPositionInfoSource::Private::ua_location_error_to_qt(UALocationServiceError error)
{
    switch (error)
    {
    case UA_LOCATION_SERVICE_ERROR_NONE:
        return QGeoPositionInfoSource::NoError;
    case UA_LOCATION_SERVICE_ERROR_NO_CONNECTION:
        return QGeoPositionInfoSource::UnknownSourceError;
    case UA_LOCATION_SERVICE_ERROR_NO_ACCESS:
        return QGeoPositionInfoSource::AccessError;
    case UA_LOCATION_SERVICE_ERROR_GENERIC_ERROR:
        return QGeoPositionInfoSource::UnknownSourceError;
    }

    return QGeoPositionInfoSource::UnknownSourceError;
}

// Called for every new position update arriving from the backend service.
void core::GeoPositionInfoSource::Private::processPositionUpdate(UALocationPositionUpdate* position, void* context)
{
    if (!position)
        return;

    Private* thiz = static_cast<Private*>(context);

    if (!thiz)
        return;

    thiz->handlePositionUpdate(position);
}

// Called for every new heading update arriving from the backend service.
void core::GeoPositionInfoSource::Private::processHeadingUpdate(UALocationHeadingUpdate* heading, void* context)
{
    if (!heading)
        return;

    Private* thiz = static_cast<Private*>(context);

    if (!thiz)
        return;

    thiz->handleHeadingUpdate(heading);
}

// Called for every new heading update arriving from the backend service.
void core::GeoPositionInfoSource::Private::processVelocityUpdate(UALocationVelocityUpdate* velocity, void* context)
{
    if (!velocity)
        return;

    Private* thiz = static_cast<Private*>(context);

    if (!thiz)
        return;

    thiz->handleVelocityUpdate(velocity);
}

// Processes the incoming position update and translates it to Qt world.
void core::GeoPositionInfoSource::Private::handlePositionUpdate(UALocationPositionUpdate* position)
{
    QGeoCoordinate coord(
        ua_location_position_update_get_latitude_in_degree(position),
        ua_location_position_update_get_longitude_in_degree(position),
        ua_location_position_update_has_altitude(position) ? ua_location_position_update_get_altitude_in_meter(position) : 0);

    QMutexLocker lock(&lastKnownPositionGuard);

    lastKnownPosition.setCoordinate(coord);

    if (ua_location_position_update_has_horizontal_accuracy(position))
    {
        double accuracy = ua_location_position_update_get_horizontal_accuracy_in_meter(position);
        if (!std::isnan(accuracy))
            lastKnownPosition.setAttribute(QGeoPositionInfo::HorizontalAccuracy, accuracy);
    }

    if (ua_location_position_update_has_vertical_accuracy(position))
    {
        double accuracy = ua_location_position_update_get_vertical_accuracy_in_meter(position);
        if (!std::isnan(accuracy))
            lastKnownPosition.setAttribute(QGeoPositionInfo::VerticalAccuracy, accuracy);
    }

    lastKnownPosition.setTimestamp(
        QDateTime::fromMSecsSinceEpoch(
            ua_location_position_update_get_timestamp(position)/1000));

    QGeoPositionInfo info{lastKnownPosition};

    QMetaObject::invokeMethod(
        parent,
        "positionUpdated",
        Qt::QueuedConnection,
        Q_ARG(QGeoPositionInfo, info));

    if (timer.isActive())
        timer.stop();

    if (parent->m_state == State::one_shot)
        parent->stopUpdates();
}

// Processes the incoming heading update and translates it to Qt world.
void core::GeoPositionInfoSource::Private::handleHeadingUpdate(UALocationHeadingUpdate* heading)
{
    QMutexLocker lock(&lastKnownPositionGuard);

    lastKnownPosition.setAttribute(
        QGeoPositionInfo::Direction,
        ua_location_heading_update_get_heading_in_degree(heading));

    lastKnownPosition.setTimestamp(
        QDateTime::fromMSecsSinceEpoch(
            ua_location_heading_update_get_timestamp(heading)/1000));

    QGeoPositionInfo info{lastKnownPosition};

    QMetaObject::invokeMethod(
        parent,
        "positionUpdated",
        Qt::QueuedConnection,
        Q_ARG(QGeoPositionInfo, info));
}

// Processes the incoming velocity update and translates it to Qt world.
void core::GeoPositionInfoSource::Private::handleVelocityUpdate(UALocationVelocityUpdate* velocity)
{
    QMutexLocker lock(&lastKnownPositionGuard);

    lastKnownPosition.setAttribute(
        QGeoPositionInfo::GroundSpeed,
        ua_location_velocity_update_get_velocity_in_meters_per_second(velocity));

    lastKnownPosition.setTimestamp(
        QDateTime::fromMSecsSinceEpoch(
            ua_location_velocity_update_get_timestamp(velocity)/1000));

    QGeoPositionInfo info{lastKnownPosition};

    QMetaObject::invokeMethod(
        parent,
        "positionUpdated",
        Qt::QueuedConnection,
        Q_ARG(QGeoPositionInfo, info));
}

void core::GeoPositionInfoSource::Private::createLocationServiceSession()
{
    UALocationServiceError e;
    session = ua_location_service_try_create_session_for_high_accuracy(core::GeoPositionInfoSource::Private::empty_creation_flags, &e);

    error = core::GeoPositionInfoSource::Private::ua_location_error_to_qt(e);

    ua_location_service_session_set_position_updates_handler(
        session,
        core::GeoPositionInfoSource::Private::processPositionUpdate,
        this);

    ua_location_service_session_set_heading_updates_handler(
        session,
        core::GeoPositionInfoSource::Private::processHeadingUpdate,
        this);

    ua_location_service_session_set_velocity_updates_handler(
        session,
        core::GeoPositionInfoSource::Private::processVelocityUpdate,
        this);
}

void core::GeoPositionInfoSource::Private::destroyLocationServiceSession()
{
    if (session != nullptr) {
        ua_location_service_session_set_position_updates_handler(
            session,
            nullptr,
            nullptr);

        ua_location_service_session_set_heading_updates_handler(
            session,
            nullptr,
            nullptr);

        ua_location_service_session_set_velocity_updates_handler(
            session,
            nullptr,
            nullptr);

        ua_location_service_session_unref(session);
        session = nullptr;
    }
}

// Creates a new instance and attempts to connect to the background service.
// Stores errors in the error member.
core::GeoPositionInfoSource::Private::Private(core::GeoPositionInfoSource* parent)
    : parent(parent),
      session(nullptr),
      error(QGeoPositionInfoSource::NoError)
{
    qRegisterMetaType<QGeoPositionInfo>("QGeoPositionInfo");
}

core::GeoPositionInfoSource::Private::~Private()
{
    destroyLocationServiceSession();
}
