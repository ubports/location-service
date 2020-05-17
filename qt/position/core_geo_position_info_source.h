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

#ifndef CORE_GEO_POSITION_INFO_SOURCE_H
#define CORE_GEO_POSITION_INFO_SOURCE_H

#include <qgeopositioninfosource.h>

namespace core
{
class GeoPositionInfoSource : public QGeoPositionInfoSource
{
    Q_OBJECT
public:
    GeoPositionInfoSource(QObject *parent = 0);
    ~GeoPositionInfoSource();

    // From QGeoPositionInfoSource
    void setUpdateInterval(int msec);
    QGeoPositionInfo lastKnownPosition(bool fromSatellitePositioningMethodsOnly = false) const;
    PositioningMethods supportedPositioningMethods() const;
    void setPreferredPositioningMethods(PositioningMethods methods);
    int minimumUpdateInterval() const;
    Error error() const;

    // Updates request state
    enum State {
        stopped = 0,
        running,
        one_shot,
        suspended,
    };

public slots:
    void applicationStateChanged();
    virtual void startUpdates();
    virtual void stopUpdates();
    virtual void requestUpdate(int timeout = 5000);
    void timeout();

private:
    bool m_applicationActive;
    int m_lastReqTimeout;
    int m_state;
    struct Private;
    QScopedPointer<Private> d;
};
}

#endif // CORE_GEO_POSITION_INFO_SOURCE_H
