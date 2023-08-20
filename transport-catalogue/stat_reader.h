#pragma once
#include <string_view>

#include "transport_catalogue.h"

namespace stat_reader {

void ProvideBusInfo(std::ostream& out,
                    const transport_catalogue::TransportCatalogue& catalogue,
                    std::string_view& line);
 void ProvideStopInfo(std::ostream& out,
                    const transport_catalogue::TransportCatalogue& catalogue,
                    std::string_view& line);
    void ProvideRequestedInfo(std::ostream& out,const transport_catalogue::TransportCatalogue& catalogue,std::istream &is);
}  // namespace stat_reader
