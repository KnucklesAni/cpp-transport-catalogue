#pragma once
#include <istream>
#include <string_view>
#include <tuple>

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

namespace json_reader {

transport_catalogue::Bus
ReadBusInfo(transport_catalogue::TransportCatalogue &catalogue,
            const json::Node &bus_node);
transport_catalogue::Stop ReadStopInfo(const json::Node &stop_node);
void ReadStopToStopInfo(const transport_catalogue::Stop *stop_from,
                        transport_catalogue::TransportCatalogue &catalogue,
                        const json::Node &road_distances_node);
void ReadBusAndStopsInfo(transport_catalogue::TransportCatalogue &catalogue,
                         const json::Node &base_requests);

json::Dict
ProvideBusInfo(const transport_catalogue::TransportCatalogue &catalogue,
               std::string_view name);
json::Dict
ProvideStopInfo(const transport_catalogue::TransportCatalogue &catalogue,
                std::string_view name);
json::Node
ProvideRequestedInfo(const transport_catalogue::TransportCatalogue &catalogue,
                     const renderer::RenderSettings &render_settings,
                     const json::Node &stat_requests);
renderer::RenderSettings ProcessRenderSettings(const json::Dict &settings);
svg::Color ProcessColor(const json::Node &color);
} // namespace json_reader
