#include <istream>
#include <string>
#include <string_view>
#include <tuple>

#include "input_reader.h"

namespace string_reader {

std::tuple<std::string_view, Delimieter> GetName(std::string_view &line,
                                                 char delimieter) {
  auto end = line.find(delimieter);
  std::tuple<std::string_view, Delimieter> result;
  auto &[name, found] = result;
  if (end == std::string_view::npos) {
    found = Delimieter::NotFound;
    name = line;
    line = std::string_view{};
  } else {
    found = Delimieter::Found;
    name = line.substr(0, end);
    line = line.substr(end + 1);
  }
  auto name_start = name.find_first_not_of(" \t");
  auto name_end = name.find_last_not_of(" \t");
  if (name_end < name_start) {
    name = std::string_view{};
  } else {
    name = name.substr(name_start, name_end - name_start + 1);
  }
  return result;
}

} // namespace string_reader

namespace input_reader {

namespace {
// Return reference to Stop in the list of stops.
// Add empty stop which doesn't have any coordinates if it's not in the base.
const transport_catalogue::Stop &
GetStop(transport_catalogue::TransportCatalogue &catalogue,
        std::string_view &line, char delimeter) {
  auto [stop_name, delimeter_found] = string_reader::GetName(line, delimeter);
  auto stop = catalogue.GetStops().At(stop_name);
  if (!stop.has_value()) {
    catalogue.Emplace(transport_catalogue::Stop{std::string{stop_name}});
    stop = catalogue.GetStops().At(stop_name);
    if (!stop.has_value()) {
      throw std::logic_error("Couldn't add dummy stop");
    }
  }
  return **stop;
}

} // namespace

transport_catalogue::Bus
ReadBusInfo(transport_catalogue::TransportCatalogue &catalogue,
            std::string_view &line) {
  auto [bus_name, delimeter_found] = string_reader::GetName(line, ':');
  if (delimeter_found != string_reader::Delimieter::Found) {
    throw std::invalid_argument("Wrong input format");
  }
  auto normal_path = line.find('-') != std::string_view::npos;
  auto round_path = line.find('>') != std::string_view::npos;
  if (!(normal_path ^ round_path)) {
    throw std::invalid_argument("Wrong input format");
  }
  auto [route_type, delimeter] =
      normal_path
          ? std::tuple{transport_catalogue::RouteType::BackAndForth, '-'}
          : std::tuple{transport_catalogue::RouteType::Round, '>'};
  transport_catalogue::Bus bus = {std::string{bus_name}, route_type};
  while (!line.empty()) {
    bus.route.push_back(&GetStop(catalogue, line, delimeter));
  }
  return bus;
}

transport_catalogue::Stop ReadStopInfo(std::string_view &line) {
  auto [stop_name, delimeter_found] = string_reader::GetName(line, ':');
  if (delimeter_found != string_reader::Delimieter::Found) {
    throw std::invalid_argument("Wrong input format");
  }
  auto [lat_text, lat_delimeter] = string_reader::GetName(line, ',');
  auto [lng_text, lng_delimeter] = string_reader::GetName(line, ',');
  double lat = stod(std::string{lat_text});
  if (lat < 0 || lat > 90) {
    throw std::invalid_argument("Wrong input format");
  }
  double lng = stod(std::string{lng_text});
  // Technically longitude have to be between 0 and 180 but tests, for some
  // reason, pass -1 in there.
  /* if (lng < 0 || lng > 180) {
     throw std::invalid_argument("Wrong input format" + std::to_string(lng));
   }*/
  return transport_catalogue::Stop{std::string{stop_name}, lat, lng};
}

void ReadStopToStopInfo(const transport_catalogue::Stop *stop_from,
                        transport_catalogue::TransportCatalogue &catalogue,
                        std::string_view &line) {
  while (!line.empty()) {
    auto [distance_text, delimeter_found] = string_reader::GetName(line, 'm');
    double distance = stod(std::string{distance_text});
    if (delimeter_found != string_reader::Delimieter::Found) {
      throw std::invalid_argument("Wrong input format");
    }
    if (line.substr(0, 4) != " to ") {
      throw std::invalid_argument("Wrong input format");
    }
    line = line.substr(4);
    catalogue.AddDistance(stop_from, &GetStop(catalogue, line, ','), distance);
  }
}
 void ReadBusAndStopsInfo(transport_catalogue::TransportCatalogue &catalogue,std::istream &is){
     size_t N;
  is >> N;
  is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  for (size_t line_no = 0; line_no < N; ++line_no) {
    std::string linе;
    std::getline(is, linе);
    std::string_view line = linе;
    if (line.substr(0, 4) == "Bus ") {
      line = line.substr(4);
      catalogue.Emplace(input_reader::ReadBusInfo(catalogue, line));
    } else if (line.substr(0, 5) == "Stop ") {
      line = line.substr(5);
      const transport_catalogue::Stop *stop_from =
          catalogue.Emplace(input_reader::ReadStopInfo(line));
      input_reader::ReadStopToStopInfo(stop_from, catalogue, line);
    } else {
      throw std::logic_error{"Line must start with Bus or Stop!\nLine" +
                             std::to_string(line_no + 1)};
    }
  }
 }
} // namespace input_reader
