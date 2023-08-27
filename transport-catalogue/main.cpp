#include <iostream>

#include "json_reader.h"
#include "transport_catalogue.h"

int main() {
  transport_catalogue::TransportCatalogue catalogue;
  json::Document input = json::Load(std::cin);
  const auto &root = input.GetRoot().AsMap();
  json_reader::ReadBusAndStopsInfo(catalogue, root.at("base_requests"));
  const auto &settings =
      json_reader::ProcessRenderSettings(root.at("render_settings").AsMap());
  Print(json::Document{json_reader::ProvideRequestedInfo(
            catalogue, settings, root.at("stat_requests"))},
        std::cout);
}
