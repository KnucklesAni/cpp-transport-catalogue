#pragma once
#include <istream>
#include <string_view>
#include <tuple>

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include <iostream>

namespace json_reader {
class JSONReader {
public:
  JSONReader(std::istream &json_source);
  void GenerateResponse(std::ostream &);

private:
  json::Document input_;
  transport_catalogue::TransportCatalogue catalogue_;
  renderer::RenderSettings render_settings_;
  transport_catalogue::Bus ReadBusInfo(const json::Node &bus_node);
  transport_catalogue::Stop ReadStopInfo(const json::Node &stop_node);
  void ReadStopToStopInfo(const transport_catalogue::Stop *stop_from,

                          const json::Node &road_distances_node);
  void ReadBusAndStopsInfo(const json::Node &base_requests);

  json::Dict ProvideBusInfo(std::string_view name);
  json::Dict ProvideStopInfo(std::string_view name);
  json::Node ProvideRequestedInfo(const json::Node &stat_requests);
  renderer::RenderSettings ProcessRenderSettings(const json::Dict &settings);
  svg::Color ProcessColor(const json::Node &color);
};
} // namespace json_reader
