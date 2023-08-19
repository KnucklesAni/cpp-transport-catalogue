#include <istream>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_set>

#include "input_reader.h"

namespace stat_reader {

void ProvideBusInfo(std::ostream &out,
                    const transport_catalogue::TransportCatalogue &catalogue,
                    std::string_view &line) {
  out << "Bus " << line << ": ";
  auto bus_infо = catalogue.GetBuses().At(line);
  if (!bus_infо.has_value()) {
    out << "not found\n";
    return;
  }
  auto &bus_info = *bus_infо;
  std::unordered_set<std::string_view> unique_stops;
  const transport_catalogue::Stop *prev_stop = nullptr;
  double route_length = 0, real_route_length = 0;
  for (auto &stop : bus_info->route) {
    unique_stops.insert(stop->name);
    if (&stop != &bus_info->route.front()) {
      route_length +=
          ComputeDistance(prev_stop->coordinates, stop->coordinates);
      if (catalogue.GetStopToStopDistance().count({prev_stop, stop}) > 0) {
        real_route_length +=
            catalogue.GetStopToStopDistance().at({prev_stop, stop});
      } else if (catalogue.GetStopToStopDistance().count({stop, prev_stop}) >
                 0) {
        real_route_length +=
            catalogue.GetStopToStopDistance().at({stop, prev_stop});
      } else {
        real_route_length +=
            ComputeDistance(prev_stop->coordinates, stop->coordinates);
      }
      if (bus_info->route_type ==
          transport_catalogue::RouteType::BackAndForth) {
        if (catalogue.GetStopToStopDistance().count({stop, prev_stop}) > 0) {
          real_route_length +=
              catalogue.GetStopToStopDistance().at({stop, prev_stop});
        } else if (catalogue.GetStopToStopDistance().count({prev_stop, stop}) >
                   0) {
          real_route_length +=
              catalogue.GetStopToStopDistance().at({prev_stop, stop});
        } else {
          real_route_length +=
              ComputeDistance(prev_stop->coordinates, stop->coordinates);
        }
      }
    }

    prev_stop = stop;
  }
  if (bus_info->route_type == transport_catalogue::RouteType::BackAndForth) {

    if (catalogue.GetStopToStopDistance().count(
            {bus_info->route.back(), bus_info->route.back()}) > 0) {
      real_route_length += catalogue.GetStopToStopDistance().at(
          {bus_info->route.back(), bus_info->route.back()});
    }
    out << (bus_info->route.size() * 2 - 1) << " stops on route, "
        << unique_stops.size() << " unique stops, " << real_route_length
        << " route length, " << real_route_length / route_length / 2
        << " curvature\n";
  } else {
    out << bus_info->route.size() << " stops on route, " << unique_stops.size()
        << " unique stops, " << real_route_length << " route length, "
        << real_route_length / route_length << " curvature\n";
  }
}

void ProvideStopInfo(std::ostream &out,
                     const transport_catalogue::TransportCatalogue &catalogue,
                     std::string_view &line) {
  out << "Stop " << line << ": ";
  auto stop_infо = catalogue.GetStopsToBuses().find(line);
  if (stop_infо == catalogue.GetStopsToBuses().end()) {
    out << "not found\n";
    return;
  }
  auto &stop_info = stop_infо->second;
  if (stop_info.size() == 0) {
    out << "no buses\n";
    return;
  }
  for (auto &bus_name : stop_info) {
    if (&bus_name == &*stop_info.begin()) {
      out << "buses ";
    } else {
      out << ' ';
    }
    out << bus_name;
  }
  out << '\n';
}

} // namespace stat_reader
