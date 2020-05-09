#ifndef ICHNAEA_CODEC_H_
#define ICHNAEA_CODEC_H_

#include <ichnaea/error.h>

#include <ichnaea/bluetooth_beacon.h>
#include <ichnaea/radio_cell.h>
#include <ichnaea/wifi_access_point.h>

#include <ichnaea/geolocate/fallback.h>
#include <ichnaea/geolocate/parameters.h>
#include <ichnaea/geolocate/result.h>
#include <ichnaea/geosubmit/parameters.h>
#include <ichnaea/geosubmit/report.h>
#include <ichnaea/geosubmit/result.h>
#include <ichnaea/region/result.h>

#include <ichnaea/util/json.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

#include <chrono>
#include <map>
#include <set>
#include <vector>

namespace json = nlohmann;

namespace ichnaea
{

template<typename T>
struct NVP
{
    std::string name;
    T value;
};

template<typename T>
inline NVP<T> make_nvp(const std::string& name, const T& value)
{
    return NVP<T>{name, value};
}

template<typename T>
struct Codec
{
    static void encode(json::json& out, const T& in)
    {
        out = in;
    }

    static void decode(json::json& in, T& out)
    {
        out = in;
    }
};

template<typename T>
inline json::json& operator&(json::json& out, const T& in)
{
    Codec<T>::encode(out, in);
    return out;
}

template<typename T>
inline json::json& operator&(json::json& out, const std::reference_wrapper<T>& in)
{
    Codec<T>::decode(out, in.get());
    return out;
}

template<typename T>
struct Codec<NVP<T>>
{
    static void encode(json::json& out, const NVP<T>& in)
    {
        Codec<T>::encode(out[in.name], in.value);
    }

    static void decode(json::json& in, NVP<T>& out)
    {
        Codec<T>::decode(in[out.name], out.value);
    }
};

template<typename T>
struct Codec<NVP<boost::optional<T>>>
{
    static void encode(json::json& out, const NVP<boost::optional<T>>& in)
    {
        if (in.value) out[in.name] & in.value;
    }

    static void decode(json::json& in, NVP<boost::optional<T>>& out)
    {
        if (in) in[out.name] & out.value;
    }
};

template<typename T>
struct Codec<NVP<std::set<T>>>
{
    static void encode(json::json& out, const NVP<std::set<T>>& in)
    {
        if (not in.value.empty()) out[in.name] & in.value;
    }

    static void decode(json::json& in, NVP<std::set<T>>& out)
    {
        if (in) in[out.name] & out.value;
    }
};

template<>
struct Codec<std::chrono::milliseconds>
{
    static void encode(json::json& out, const std::chrono::milliseconds in)
    {
        out & in.count();
    }

    static void decode(json::json& out, std::chrono::milliseconds& in);
};

template<typename T>
struct Codec<boost::optional<T>>
{
    static void encode(json::json& out, const boost::optional<T>& in)
    {
        if (in) Codec<T>::encode(out, *in);
    }

    static void decode(json::json& out, boost::optional<T>& in)
    {
        if (out) Codec<T>::encode(out, *(in = T{}));
    }
};

template<typename T>
struct Codec<std::set<T>>
{
    static void encode(json::json& out, const std::set<T>& in)
    {
        if (in.empty()) return;
        for (const auto& element : in)
        {
            json::json j; Codec<T>::encode(j, element); out.push_back(j);
        }
    }

    static void decode(json::json& out, std::set<T>& in)
    {
        for (const auto& element: in)
        {
            T t; Codec<T>::decode(element, t); in.insert(t);
        }
    }
};

template<typename T>
struct Codec<std::vector<T>>
{
    static void encode(json::json& out, const std::vector<T>& in)
    {
        if (in.empty()) return;
        for (const auto& element : in)
        {
            json::json j; Codec<T>::encode(j, element); out.push_back(j);
        }
    }

    static void decode(json::json& out, std::vector<T>& in)
    {
        for (const auto& element: in)
        {
            T t; Codec<T>::decode(element, t); in.push_back(t);
        }
    }
};

template<>
struct Codec<Error>
{
    static void encode(json::json&, const Error&)
    {
    }

    static void decode(json::json& in, Error& error)
    {
        Error e{static_cast<core::net::http::Status>(in["error"]["code"].get<unsigned int>()), in["error"]["message"]};
        for (const auto& element : in["error"]["errors"])
            e.errors.push_back({element["domain"], element["reason"], element["message"]});
        error = e;
    }
};

template<>
struct Codec<BluetoothBeacon>
{
    static void encode(json::json& out, const BluetoothBeacon& in)
    {
        out & make_nvp("macAddress", in.mac_address)
            & make_nvp("name", in.name)
            & make_nvp("age", in.age.count())
            & make_nvp("signalStrength", in.signal_strength);
    }

    static void decode(json::json& out, BluetoothBeacon& in);
};

template<>
struct Codec<WifiAccessPoint::Type>
{
    static void encode(json::json& out, WifiAccessPoint::Type in)
    {
        out = boost::lexical_cast<std::string>(in);
    }
};

template<>
struct Codec<WifiAccessPoint>
{
    static void encode(json::json& out, const WifiAccessPoint& in)
    {
        out & make_nvp("macAddress", in.bssid)
            & make_nvp("age", in.age)
            & make_nvp("channel", in.channel)
            & make_nvp("frequency", in.frequency)
            & make_nvp("signalStrength", in.signal_strength)
            & make_nvp("signalToNoiseRatio", in.signal_to_noise_ratio)
            & make_nvp("ssid", in.ssid)
            & make_nvp("radioType", in.type);
    }

    static void decode(json::json& out, BluetoothBeacon& in);
};

template<>
struct Codec<RadioCell>
{
    static void encode(json::json& out, const RadioCell& in)
    {
        out & make_nvp("radioType", boost::lexical_cast<std::string>(in.radio_type))
            & make_nvp("mobileCountryCode", in.mcc)
            & make_nvp("mobileNetworkCode", in.mnc)
            & make_nvp("locationAreaCode", in.lac)
            & make_nvp("cellId", in.id)
            & make_nvp("age", in.age)
            & make_nvp("psc", in.psc)
            & make_nvp("asu", in.asu)
            & make_nvp("signalStrength", in.signal_strength)
            & make_nvp("timingAdvance", in.timing_advance);
    }

    static void decode(json::json& out, RadioCell& in);
};

template<>
struct Codec<geolocate::Parameters>
{
    static void encode(json::json& out, const geolocate::Parameters& parameters)
    {
        out & make_nvp("carrier", parameters.carrier)
            & make_nvp("considerIp", parameters.consider_ip)
            & make_nvp("homeMobileCountryCode", parameters.mcc)
            & make_nvp("homeMobileNetworkCode", parameters.mnc)
            & make_nvp("bluetoothBeacons", parameters.bluetooth_beacons)
            & make_nvp("wifiAccessPoints", parameters.wifi_access_points)
            & make_nvp("cellTowers", parameters.radio_cells)
            & make_nvp("fallbacks", parameters.fallback);
    }
};

template<>
struct Codec<geolocate::Fallback>
{
    static void encode(json::json& out, geolocate::Fallback fb)
    {
        if ((fb & geolocate::Fallback::lac) == geolocate::Fallback::lac)
            out & make_nvp("lacf", true);
        if ((fb & geolocate::Fallback::ip) == geolocate::Fallback::ip)
            out & make_nvp("ip", true);
    }
};

template<>
struct Codec<geolocate::Result>
{
    static void decode(json::json& in, geolocate::Result& r)
    {
        r.location.lat = in["location"]["lat"];
        r.location.lon = in["location"]["lng"];
        r.accuracy = in["accuracy"];

        if (in["fallback"] == "ipf")
            r.fallback = geolocate::Fallback::ip;
        if (in["fallback"] == "lacf")
            r.fallback = geolocate::Fallback::lac;
    }
};

template<>
struct Codec<geosubmit::Report::Position::Source>
{
    static void encode(json::json& out, const geosubmit::Report::Position::Source& source)
    {
        out = boost::lexical_cast<std::string>(source);
    }

    static void decode(json::json&, geosubmit::Report&);
};

template<>
struct Codec<geosubmit::Report>
{
    static void encode(json::json& out, const geosubmit::Report& report)
    {
        out & make_nvp("timestamp", std::chrono::system_clock::to_time_t(report.timestamp) * 1000);

        out["position"] & make_nvp("latitude", report.position.latitude)
                        & make_nvp("longitude", report.position.longitude)
                        & make_nvp("accuracy", report.position.accuracy)
                        & make_nvp("altitude", report.position.altitude)
                        & make_nvp("altitudeAccuracy", report.position.altitude_accuracy)
                        & make_nvp("age", report.position.age)
                        & make_nvp("heading", report.position.heading)
                        & make_nvp("pressure", report.position.pressure)
                        & make_nvp("speed", report.position.speed)
                        & make_nvp("source", report.position.source);
        out & make_nvp("bluetoothBeacons", report.bluetooth_beacons)
            & make_nvp("wifiAccessPoints", report.wifi_access_points)
            & make_nvp("radioCells", report.radio_cells);
    }

    static void decode(json::json&, geosubmit::Report&);
};

template<>
struct Codec<geosubmit::Parameters>
{
    static void encode(json::json& out, const geosubmit::Parameters& params)
    {
        out & make_nvp("items", params.reports);
    }

    static void decode(json::json&, geosubmit::Parameters&);
};

template<>
struct Codec<geosubmit::Result>
{
    static void decode(json::json&, geosubmit::Result&)
    {
    }
};

template<>
struct Codec<region::Result>
{
    static void decode(json::json& in, region::Result& result)
    {
        result.country_code = in["country_code"];
        result.country_name = in["country_name"];

    }
};
}

#endif // ICHNAEA_CODEC_H_
