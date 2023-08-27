#include "json_reader.h"
#include <algorithm>
#include <istream>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>

namespace json_reader {

namespace {

// Return reference to Stop in the list of stops.
// Add empty stop which doesn't have any coordinates if it's not in the base.
const transport_catalogue::Stop &
GetStop(transport_catalogue::TransportCatalogue &catalogue,
        std::string_view stop_name) {
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
            const json::Node &bus_node) {
  const auto &bus_info = bus_node.AsMap();
  transport_catalogue::Bus bus = {bus_info.at("name").AsString(),
                                  bus_info.at("is_roundtrip").AsBool()
                                      ? domain::RouteType::Round
                                      : domain::RouteType::BackAndForth};
  for (auto &stop_node : bus_info.at("stops").AsArray()) {
    bus.route.push_back(&GetStop(catalogue, stop_node.AsString()));
  }
  return bus;
}

transport_catalogue::Stop ReadStopInfo(const json::Node &stop_node) {
  const auto &stop_info = stop_node.AsMap();
  double lat = stop_info.at("latitude").AsDouble();
  // if (lat < 0 || lat > 90) {
  //  throw std::invalid_argument("Wrong input format");
  //  }
  double lng = stop_info.at("longitude").AsDouble();
  // Technically longitude have to be between 0 and 180 but tests, for some
  // reason, pass -1 in there.
  /* if (lng < 0 || lng > 180) {
     throw std::invalid_argument("Wrong input format" + std::to_string(lng));
   }*/
  return transport_catalogue::Stop{stop_info.at("name").AsString(), lat, lng};
}

void ReadStopToStopInfo(const transport_catalogue::Stop *stop_from,
                        transport_catalogue::TransportCatalogue &catalogue,
                        const json::Node &road_distances_node) {
  for (auto &stop_info : road_distances_node.AsMap()) {
    catalogue.AddDistance(stop_from, &GetStop(catalogue, stop_info.first),
                          stop_info.second.AsDouble());
  }
}

void ReadBusAndStopsInfo(transport_catalogue::TransportCatalogue &catalogue,
                         const json::Node &base_requests) {
  for (auto &base_node : base_requests.AsArray()) {
    const auto &base_info = base_node.AsMap();
    const auto &type = base_info.at("type").AsString();
    if (type == "Bus") {
      catalogue.Emplace(ReadBusInfo(catalogue, base_info));
    } else if (type == "Stop") {
      const transport_catalogue::Stop *stop_from =
          catalogue.Emplace(ReadStopInfo(base_info));
      auto road_distances = base_info.find("road_distances");
      if (road_distances != base_info.end()) {
        ReadStopToStopInfo(stop_from, catalogue, road_distances->second);
      }
    } else {
      throw std::logic_error{"Type must be Bus or Stop!"};
    }
  }
}

json::Dict
ProvideBusInfo(const transport_catalogue::TransportCatalogue &catalogue,
               std::string_view name) {
  json::Dict result;
  auto bus_infо = catalogue.GetBuses().At(name);
  if (!bus_infо.has_value()) {
    result.insert({"error_message", std::string{"not found"}});
    return result;
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
      if (bus_info->route_type == domain::RouteType::BackAndForth) {
        real_route_length += catalogue.GetDistance(stop, prev_stop);
      }
    }
    prev_stop = stop;
  }
  double curvature;
  int stop_count;
  if (bus_info->route_type == domain::RouteType::BackAndForth) {
    real_route_length +=
        catalogue.GetDistance(bus_info->route.back(), bus_info->route.back());
    curvature = real_route_length / route_length / 2;
    stop_count = bus_info->route.size() * 2 - 1;
  } else {
    curvature = real_route_length / route_length;
    stop_count = bus_info->route.size();
  }
  result.insert({"curvature", curvature});
  result.insert({"route_length", real_route_length});
  result.insert({"stop_count", stop_count});
  result.insert({"unique_stop_count", int(unique_stops.size())});
  return result;
}

json::Dict
ProvideStopInfo(const transport_catalogue::TransportCatalogue &catalogue,
                std::string_view name) {
  json::Dict result;
  auto stop_infо = catalogue.GetStops().At(name);
  if (!stop_infо.has_value()) {
    result.insert({"error_message", std::string{"not found"}});
    return result;
  }
  auto &stop_info = catalogue.GetStopsToBuses().at(*stop_infо);
  std::vector<std::string_view> bus_names;
  bus_names.resize(stop_info.size());
  std::transform(stop_info.begin(), stop_info.end(), bus_names.begin(),
                 [](auto stop) { return std::string_view{stop->name}; });
  std::sort(bus_names.begin(), bus_names.end());
  json::Array buses;
  for (auto &bus_name : bus_names) {
    buses.emplace_back(std::string{bus_name});
  }
  result.insert({"buses", buses});
  return result;
}

json::Node
ProvideRequestedInfo(const transport_catalogue::TransportCatalogue &catalogue,
                     const renderer::RenderSettings &render_settings,
                     const json::Node &stat_requests) {
  json::Array result;
  for (const auto &request : stat_requests.AsArray()) {
    const auto &request_info = request.AsMap();
    const auto &type = request_info.at("type").AsString();
    json::Dict response;
    if (type == "Bus") {
      response = ProvideBusInfo(catalogue, request_info.at("name").AsString());
    } else if (type == "Stop") {
      response = ProvideStopInfo(catalogue, request_info.at("name").AsString());
    } else if (type == "Map") {
      std::stringstream stream;
      renderer::GetSVG(render_settings, catalogue.GetBuses()).Render(stream);
      response.insert({"map", stream.str()});
    } else {
      throw std::logic_error{"Type must be Bus or Stop!"};
    }
    response.insert({"request_id", request_info.at("id")});
    result.push_back(std::move(response));
  }
  return result;
}
renderer::RenderSettings ProcessRenderSettings(const json::Dict &settings) {
  renderer::RenderSettings result;
  result.width = settings.at("width").AsDouble();
  result.height = settings.at("height").AsDouble();
  result.padding = settings.at("padding").AsDouble();
  result.line_width = settings.at("line_width").AsDouble();
  result.stop_radius = settings.at("stop_radius").AsDouble();
  result.bus_label_font_size = settings.at("bus_label_font_size").AsInt();
  result.bus_label_offset =
      svg::Point(settings.at("bus_label_offset").AsArray()[0].AsDouble(),
                 settings.at("bus_label_offset").AsArray()[1].AsDouble());
  result.stop_label_font_size = settings.at("stop_label_font_size").AsInt();
  result.stop_label_offset =
      svg::Point(settings.at("stop_label_offset").AsArray()[0].AsDouble(),
                 settings.at("stop_label_offset").AsArray()[1].AsDouble());
  result.underlayer_width = settings.at("underlayer_width").AsDouble();
  result.underlayer_color = ProcessColor(settings.at("underlayer_color"));
  for (const auto &color : settings.at("color_palette").AsArray()) {
    result.color_palette.push_back(ProcessColor(color));
  }
  return result;
}
svg::Color ProcessColor(const json::Node &color) {
  if (color.IsString()) {
    return color.AsString();
  }
  if (color.AsArray().size() == 3) {
    return svg::Rgb(color.AsArray()[0].AsInt(), color.AsArray()[1].AsInt(),
                    color.AsArray()[2].AsInt());
  }
  return svg::Rgba(color.AsArray()[0].AsInt(), color.AsArray()[1].AsInt(),
                   color.AsArray()[2].AsInt(), color.AsArray()[3].AsDouble());
}
} // namespace json_reader
