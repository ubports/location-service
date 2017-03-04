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

#include <location/dbus/codec.h>

#include <location/events/all.h>
#include <location/events/registry.h>

#include <boost/lexical_cast.hpp>

location::Optional<location::Credentials>
location::dbus::Codec<location::Credentials>::decode(GVariant* variant)
{
    GVariantDict dict; g_variant_dict_init(&dict, variant);

    std::uint32_t uid, pid;
    if (!g_variant_dict_lookup(&dict, "UnixUserID", "u", &uid))
        return location::Optional<location::Credentials>{};
    if (!g_variant_dict_lookup(&dict, "ProcessID", "u", &pid))
        return location::Optional<location::Credentials>{};

    return location::Credentials{pid, uid};
}

GVariant* location::dbus::Codec<location::Criteria>::encode(const Criteria& value)
{
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));
    if ((value.requirements() & Features::position) == Features::position)
        g_variant_builder_add(&builder, "{sv}", "wants-position", g_variant_new_boolean(true));
    if ((value.requirements() & Features::heading) == Features::heading)
        g_variant_builder_add(&builder, "{sv}", "wants-heading", g_variant_new_boolean(true));
    if ((value.requirements() & Features::velocity) == Features::velocity)
        g_variant_builder_add(&builder, "{sv}", "wants-velocity", g_variant_new_boolean(true));
    if (value.accuracy().horizontal())
        g_variant_builder_add(&builder, "{sv}", "horizontal-accuracy",
                              g_variant_new_double(value.accuracy().horizontal()->value()));
    if (value.accuracy().vertical())
        g_variant_builder_add(&builder, "{sv}", "vertical-accuracy",
                              g_variant_new_double(value.accuracy().vertical()->value()));
    if (value.accuracy().heading())
        g_variant_builder_add(&builder, "{sv}", "heading-accuracy",
                              g_variant_new_double(value.accuracy().heading()->value()));
    if (value.accuracy().velocity())
        g_variant_builder_add(&builder, "{sv}", "velocity-accuracy",
                              g_variant_new_double(value.accuracy().velocity()->value()));
    return g_variant_builder_end(&builder);
}

location::Optional<location::Criteria> location::dbus::Codec<location::Criteria>::decode(GVariant* variant)
{
    GVariantDict dict; g_variant_dict_init(&dict, variant);

    bool boolean_value; double double_value;
    location::Features features{location::Features::none};

    if (g_variant_dict_lookup(&dict, "wants-position", "b", &boolean_value))
        features = features | location::Features::position;
    if (g_variant_dict_lookup(&dict, "wants-heading", "b", &boolean_value))
        features = features | location::Features::heading;
    if (g_variant_dict_lookup(&dict, "wants-velocity", "b", &boolean_value))
        features = features | location::Features::velocity;

    location::Criteria criteria{features};

    if (g_variant_dict_lookup(&dict, "horizontal-accuracy", "d", &double_value))
        criteria.accuracy().horizontal(units::Meters::from_value(double_value));
    if (g_variant_dict_lookup(&dict, "vertical-accuracy", "d", &double_value))
        criteria.accuracy().vertical(units::Meters::from_value(double_value));
    if (g_variant_dict_lookup(&dict, "heading-accuracy", "d", &double_value))
        criteria.accuracy().heading(units::Degrees::from_value(double_value));
    if (g_variant_dict_lookup(&dict, "velocity-accuracy", "d", &double_value))
        criteria.accuracy().velocity(units::MetersPerSecond::from_value(double_value));

    g_variant_dict_end(&dict);

    return criteria;
}

GVariant* location::dbus::Codec<location::Features>::encode(const Features& value)
{
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));

    if ((value & Features::position) == Features::position)
        g_variant_builder_add(&builder, "{sv}", "wants-position", g_variant_new_boolean(true));
    if ((value & Features::heading) == Features::heading)
        g_variant_builder_add(&builder, "{sv}", "wants-heading", g_variant_new_boolean(true));
    if ((value & Features::velocity) == Features::velocity)
        g_variant_builder_add(&builder, "{sv}", "wants-velocity", g_variant_new_boolean(true));

    return g_variant_builder_end(&builder);
}

location::Optional<location::Features> location::dbus::Codec<location::Features>::decode(GVariant* variant)
{
    GVariantDict dict; g_variant_dict_init(&dict, variant);

    Features features{Features::none}; bool boolean_value;

    if (g_variant_dict_lookup(&dict, "wants-position", "b", &boolean_value) && boolean_value)
        features = features | location::Features::position;
    if (g_variant_dict_lookup(&dict, "wants-heading", "b", &boolean_value) && boolean_value)
        features = features | location::Features::heading;
    if (g_variant_dict_lookup(&dict, "wants-velocity", "b", &boolean_value) && boolean_value)
        features = features | location::Features::velocity;

    return features;
}

GVariant* location::dbus::Codec<location::Provider::Requirements>::encode(const Provider::Requirements& value)
{
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));

    if ((value & Provider::Requirements::cell_network) == Provider::Requirements::cell_network)
        g_variant_builder_add(&builder, "{sv}", "needs-cell-network", g_variant_new_boolean(true));
    if ((value & Provider::Requirements::data_network) == Provider::Requirements::data_network)
        g_variant_builder_add(&builder, "{sv}", "needs-data-network", g_variant_new_boolean(true));
    if ((value & Provider::Requirements::monetary_spending) == Provider::Requirements::monetary_spending)
        g_variant_builder_add(&builder, "{sv}", "needs-monetary-spending", g_variant_new_boolean(true));
    if ((value & Provider::Requirements::satellites) == Provider::Requirements::satellites)
        g_variant_builder_add(&builder, "{sv}", "needs-satellites", g_variant_new_boolean(true));

    return g_variant_builder_end(&builder);
}

location::Optional<location::Provider::Requirements> location::dbus::Codec<location::Provider::Requirements>::decode(GVariant* variant)
{
    GVariantDict dict; g_variant_dict_init(&dict, variant);

    Provider::Requirements requirements{Provider::Requirements::none}; bool boolean_value;

    if (g_variant_dict_lookup(&dict, "needs-cell-network", "b", &boolean_value) && boolean_value)
        requirements = requirements | Provider::Requirements::cell_network;
    if (g_variant_dict_lookup(&dict, "needs-data-network", "b", &boolean_value) && boolean_value)
        requirements = requirements | Provider::Requirements::data_network;
    if (g_variant_dict_lookup(&dict, "needs-monetary-spending", "b", &boolean_value) && boolean_value)
        requirements = requirements | Provider::Requirements::monetary_spending;
    if (g_variant_dict_lookup(&dict, "needs-satellites", "b", &boolean_value) && boolean_value)
        requirements = requirements | Provider::Requirements::satellites;

    g_variant_dict_end(&dict);

    return requirements;
}

GVariant* location::dbus::Codec<location::Update<location::Position>>::encode(const location::Update<location::Position>& value)
{
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("(ta{sv})"));
    g_variant_builder_add(&builder, "t", value.when.time_since_epoch().count());
    g_variant_builder_open(&builder, G_VARIANT_TYPE("a{sv}"));
    g_variant_builder_add(&builder, "{sv}", "latitude", g_variant_new_double(value.value.latitude().value()));
    g_variant_builder_add(&builder, "{sv}", "longitude", g_variant_new_double(value.value.longitude().value()));

    if (value.value.accuracy().horizontal())
    {
        g_variant_builder_add(&builder, "{sv}", "horizontal-accuracy",
                              g_variant_new_double(value.value.accuracy().horizontal()->value()));
    }

    if (value.value.altitude())
    {
        g_variant_builder_add(&builder, "{sv}", "altitude",
                              g_variant_new_double(value.value.altitude()->value()));
        if (value.value.accuracy().vertical())
        {
            g_variant_builder_add(&builder, "{sv}", "vertical-accuracy",
                                  g_variant_new_double(value.value.accuracy().vertical()->value()));
        }
    }
    g_variant_builder_close(&builder);

    return g_variant_builder_end(&builder);
}

location::Optional<location::Update<location::Position>> location::dbus::Codec<location::Update<location::Position>>::decode(GVariant* variant)
{
    GVariantIter iter; g_variant_iter_init(&iter, variant);
    std::uint64_t timestamp{0}; double double_value{0};

    if (!g_variant_iter_next(&iter, "t", &timestamp))
        return location::Optional<location::Update<location::Position>>{};

    auto value = g_variant_iter_next_value(&iter);

    if (!value)
        return location::Optional<location::Update<location::Position>>{};

    double latitude{0}, longitude{0};

    GVariantDict dict; g_variant_dict_init(&dict, value);
    if (!g_variant_dict_lookup(&dict, "latitude", "d", &latitude))
        return location::Optional<location::Update<location::Position>>{};
    if (!g_variant_dict_lookup(&dict, "longitude", "d", &longitude))
        return location::Optional<location::Update<location::Position>>{};

    location::Position position;
    position.latitude(location::units::Degrees::from_value(latitude))
            .longitude(location::units::Degrees::from_value(longitude));

    if (g_variant_dict_lookup(&dict, "horizontal-accuracy", "d", &double_value))
        position.accuracy().horizontal(location::units::Meters::from_value(double_value));
    if (g_variant_dict_lookup(&dict, "altitude", "d", &double_value))
    {
        position.altitude(location::units::Meters::from_value(double_value));
        if (g_variant_dict_lookup(&dict, "vertical-accuracy", "d", &double_value))
            position.accuracy().vertical(location::units::Meters::from_value(double_value));
    }

    return location::Update<location::Position>
    {
        position,
        location::Clock::Timestamp{location::Clock::Duration{timestamp}}
    };
}

GVariant* location::dbus::Codec<location::Update<location::units::Degrees>>::encode(const location::Update<location::units::Degrees>& value)
{
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("(t(d))"));
    g_variant_builder_add(&builder, "t", value.when.time_since_epoch().count());
    g_variant_builder_open(&builder, G_VARIANT_TYPE("(d)"));
    g_variant_builder_add(&builder, "d", value.value.value());
    g_variant_builder_close(&builder);
    return g_variant_builder_end(&builder);
}

location::Optional<location::Update<location::units::Degrees>>
location::dbus::Codec<location::Update<location::units::Degrees>>::decode(GVariant* variant)
{
    GVariantIter iter; g_variant_iter_init(&iter, variant);
    std::uint64_t timestamp{0}; double value{0};
    if (!g_variant_iter_next(&iter, "t", &timestamp))
        return location::Optional<location::Update<location::units::Degrees>>{};
    if (!g_variant_iter_next(&iter, "(d)", &value))
        return location::Optional<location::Update<location::units::Degrees>>{};

    return location::Update<location::units::Degrees>{
        location::units::Degrees::from_value(value),
        location::Clock::Timestamp{location::Clock::Duration{timestamp}}
    };
}

GVariant* location::dbus::Codec<location::Update<location::units::Meters>>::encode(const location::Update<location::units::Meters>& value)
{
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("(t(d))"));
    g_variant_builder_add(&builder, "t", value.when.time_since_epoch().count());
    g_variant_builder_open(&builder, G_VARIANT_TYPE("(d)"));
    g_variant_builder_add(&builder, "d", value.value.value());
    g_variant_builder_close(&builder);
    return g_variant_builder_end(&builder);
}

location::Optional<location::Update<location::units::Meters>>
location::dbus::Codec<location::Update<location::units::Meters>>::decode(GVariant* variant)
{
    GVariantIter iter; g_variant_iter_init(&iter, variant);
    std::uint64_t timestamp{0}; double value{0};
    if (!g_variant_iter_next(&iter, "t", &timestamp))
        return location::Optional<location::Update<location::units::Meters>>{};
    if (!g_variant_iter_next(&iter, "(d)", &value))
        return location::Optional<location::Update<location::units::Meters>>{};

    return location::Update<location::units::Meters>{
        location::units::Meters::from_value(value),
        location::Clock::Timestamp{location::Clock::Duration{timestamp}}
    };
}

GVariant* location::dbus::Codec<location::Update<location::units::MetersPerSecond>>::encode(const location::Update<location::units::MetersPerSecond>& value)
{
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("(t(d))"));
    g_variant_builder_add(&builder, "t", value.when.time_since_epoch().count());
    g_variant_builder_open(&builder, G_VARIANT_TYPE("(d)"));
    g_variant_builder_add(&builder, "d", value.value.value());
    g_variant_builder_close(&builder);
    return g_variant_builder_end(&builder);
}

location::Optional<location::Update<location::units::MetersPerSecond>>
location::dbus::Codec<location::Update<location::units::MetersPerSecond>>::decode(GVariant* variant)
{
    GVariantIter iter; g_variant_iter_init(&iter, variant);
    std::uint64_t timestamp{0}; double value{0};
    if (!g_variant_iter_next(&iter, "t", &timestamp))
        return location::Optional<location::Update<location::units::MetersPerSecond>>{};
    if (!g_variant_iter_next(&iter, "(d)", &value))
        return location::Optional<location::Update<location::units::MetersPerSecond>>{};

    return location::Update<location::units::MetersPerSecond>{
        location::units::MetersPerSecond::from_value(value),
        location::Clock::Timestamp{location::Clock::Duration{timestamp}}
    };
}

GVariant* location::dbus::Codec<location::Event>::encode(const location::Event& value)
{
    if (TypeOf<events::ReferencePositionUpdated>::query() == value.type())
    {
        const auto& update = dynamic_cast<const location::events::ReferencePositionUpdated&>(value).update();

        GVariantBuilder builder;
        g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));
        g_variant_builder_add(&builder, "{sv}", "type", g_variant_new_string(events::Registry::instance().find(value.type()).c_str()));
        g_variant_builder_add(&builder, "{sv}", "when", g_variant_new_uint64(update.when.time_since_epoch().count()));
        g_variant_builder_add(&builder, "{sv}", "latitude", g_variant_new_double(update.value.latitude().value()));
        g_variant_builder_add(&builder, "{sv}", "longitude", g_variant_new_double(update.value.longitude().value()));

        if (update.value.accuracy().horizontal())
        {
            g_variant_builder_add(&builder, "{sv}", "horizontal-accuracy",
                                  g_variant_new_double(update.value.accuracy().horizontal()->value()));
        }

        if (update.value.altitude())
        {
            g_variant_builder_add(&builder, "{sv}", "altitude",
                                  g_variant_new_double(update.value.altitude()->value()));
            if (update.value.accuracy().vertical())
            {
                g_variant_builder_add(&builder, "{sv}", "vertical-accuracy",
                                      g_variant_new_double(update.value.accuracy().vertical()->value()));
            }
        }
        return g_variant_builder_end(&builder);
    }
    else if (TypeOf<events::WifiAndCellIdReportingStateChanged>::query() == value.type())
    {
        auto state = dynamic_cast<const location::events::WifiAndCellIdReportingStateChanged&>(value).new_state();

        GVariantBuilder builder;
        g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));
        g_variant_builder_add(&builder, "{sv}", "type", g_variant_new_string(events::Registry::instance().find(value.type()).c_str()));
        g_variant_builder_add(&builder, "{sv}", "new-state", g_variant_new_string(boost::lexical_cast<std::string>(state).c_str()));
        return g_variant_builder_end(&builder);
    }

    return nullptr;
}

GVariant* location::dbus::Codec<location::Event::Ptr>::encode(const location::Event::Ptr& value)
{
    return Codec<location::Event>::encode(*value);
}

location::Optional<location::Event::Ptr> location::dbus::Codec<location::Event::Ptr>::decode(GVariant* variant)
{
    GVariantDict dict; g_variant_dict_init(&dict, variant);

    char* type_name{nullptr};
    if (!g_variant_dict_lookup(&dict, "type", "s", &type_name))
        return location::Optional<location::Event::Ptr>{};

    auto type = events::Registry::instance().find(type_name);

    if (TypeOf<events::ReferencePositionUpdated>::query() == type)
    {
        std::uint64_t timestamp{0}; double double_value{0};

        double latitude{0}, longitude{0};

        if (!g_variant_dict_lookup(&dict, "when", "t", &timestamp))
            return location::Optional<location::Event::Ptr>{};
        if (!g_variant_dict_lookup(&dict, "latitude", "d", &latitude))
            return location::Optional<location::Event::Ptr>{};
        if (!g_variant_dict_lookup(&dict, "longitude", "d", &longitude))
            return location::Optional<location::Event::Ptr>{};

        location::Position position;
        position.latitude(location::units::Degrees::from_value(latitude))
                .longitude(location::units::Degrees::from_value(longitude));

        if (g_variant_dict_lookup(&dict, "horizontal-accuracy", "d", &double_value))
            position.accuracy().horizontal(location::units::Meters::from_value(double_value));
        if (g_variant_dict_lookup(&dict, "altitude", "d", &double_value))
        {
            position.altitude(location::units::Meters::from_value(double_value));
            if (g_variant_dict_lookup(&dict, "vertical-accuracy", "d", &double_value))
                position.accuracy().vertical(location::units::Meters::from_value(double_value));
        }

        return location::Event::Ptr{
                new events::ReferencePositionUpdated(
                        location::Update<location::Position>(position, location::Clock::Timestamp{location::Clock::Duration{timestamp}}))};
    }
    else if(TypeOf<events::WifiAndCellIdReportingStateChanged>::query() == type)
    {
        char* state{nullptr};
        if (!g_variant_dict_lookup(&dict, "new-state", "s", &state))
            return location::Optional<location::Event::Ptr>{};

        return location::Event::Ptr{
                new events::WifiAndCellIdReportingStateChanged(
                        boost::lexical_cast<WifiAndCellIdReportingState>(state))};
    }

    return location::Optional<location::Event::Ptr>{};
}
