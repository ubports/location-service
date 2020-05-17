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

#ifndef CORE_GEO_POSITION_INFO_SOURCE_FACTORY_H
#define CORE_GEO_POSITION_INFO_SOURCE_FACTORY_H

#include <QObject>
#include "qgeopositioninfosource.h"
#include "qgeopositioninfosourcefactory.h"

namespace core
{
class GeoPositionInfoSourceFactory
        : public QObject,
          public QGeoPositionInfoSourceFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.qt.position.sourcefactory/5.0"
                      FILE "plugin.json")
    Q_INTERFACES(QGeoPositionInfoSourceFactory)

public:
    QGeoPositionInfoSource *positionInfoSource(QObject *parent);
    QGeoSatelliteInfoSource *satelliteInfoSource(QObject *parent);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    QGeoAreaMonitorSource *areaMonitor(QObject *parent);
#endif
};
}

#endif // CORE_GEO_POSITION_INFO_SOURCE_FACTORY_H
