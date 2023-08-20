#include <algorithm>
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
      real_route_length += catalogue.GetDistance(prev_stop, stop);
      if (bus_info->route_type ==
          transport_catalogue::RouteType::BackAndForth) {
        real_route_length += catalogue.GetDistance(stop, prev_stop);
      }
    }

    prev_stop = stop;
  }
  if (bus_info->route_type == transport_catalogue::RouteType::BackAndForth) {
    real_route_length +=
        catalogue.GetDistance(bus_info->route.back(), bus_info->route.back());
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
  auto stop_infо = catalogue.GetStops().At(line);
  if (!stop_infо.has_value()) {
    out << "not found\n";
    return;
  }
  auto &stop_info = catalogue.GetStopsToBuses().at(*stop_infо);
  if (stop_info.size() == 0) {
    out << "no buses\n";
    return;
  }
  std::vector<std::string_view> bus_names;
  bus_names.resize(stop_info.size());
  std::transform(stop_info.begin(), stop_info.end(), bus_names.begin(),
                 [](auto stop) { return std::string_view{stop->name}; });
  std::sort(bus_names.begin(), bus_names.end());
  for (auto &bus_name : bus_names) {
    if (&bus_name == &*bus_names.begin()) {
      out << "buses ";
    } else {
      out << ' ';
    }
    out << bus_name;
  }
  out << '\n';
}
void ProvideRequestedInfo(
    std::ostream &out, const transport_catalogue::TransportCatalogue &catalogue,
    std::istream &is) {
  size_t Q;
  is >> Q;
  is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  for (size_t line_no = 0; line_no < Q; ++line_no) {
    std::string linе;
    std::getline(is, linе);
    std::string_view line = linе;
    if (line.substr(0, 4) == "Bus ") {
      line = line.substr(4);
      stat_reader::ProvideBusInfo(out, catalogue, line);
    } else if (line.substr(0, 5) == "Stop ") {
      line = line.substr(5);
      stat_reader::ProvideStopInfo(out, catalogue, line);
    } else {
      throw std::logic_error{"Line must start with Bus!\nLine" +
                             std::to_string(line_no)};
    }
  }
}
} // namespace stat_reader
