/*
 * Copyright © 2012-2013 Canonical Ltd.
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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_CRITERIA_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_CRITERIA_H_

#include <com/ubuntu/location/accuracy.h>
#include <com/ubuntu/location/heading.h>
#include <com/ubuntu/location/velocity.h>
#include <com/ubuntu/location/wgs84/altitude.h>
#include <com/ubuntu/location/wgs84/latitude.h>
#include <com/ubuntu/location/wgs84/longitude.h>

#include <limits>
#include <ostream>
#include <stdexcept>

namespace com
{
namespace ubuntu
{
namespace location
{
struct Criteria
{
    Criteria() : latitude_accuracy(),
                 longitude_accuracy(),
                 altitude_accuracy(),
                 velocity_accuracy(),
                 heading_accuracy()
    {
    }
    
    Accuracy<wgs84::Latitude> latitude_accuracy;
    Accuracy<wgs84::Longitude> longitude_accuracy;
    Accuracy<wgs84::Altitude> altitude_accuracy;
    Accuracy<Velocity> velocity_accuracy;
    Accuracy<Heading> heading_accuracy;
};
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_CRITERIA_H_
