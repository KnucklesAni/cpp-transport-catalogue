#pragma once
#include <istream>
#include <string_view>
#include <tuple>

#include "transport_catalogue.h"

namespace string_reader {

enum class Delimieter { NotFound, Found };

std::tuple<std::string_view, Delimieter> GetName(std::string_view &line,
                                                 char delimieter);

} // namespace string_reader

namespace input_reader {

transport_catalogue::Bus
ReadBusInfo(transport_catalogue::TransportCatalogue &catalogue,
            std::string_view &line);
transport_catalogue::Stop ReadStopInfo(std::string_view &line);
void ReadStopToStopInfo(const transport_catalogue::Stop *stop_from,
                        transport_catalogue::TransportCatalogue &catalogue,
                        std::string_view &line);

} // namespace input_reader
