#include <iostream>

#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

int main() {
  transport_catalogue::TransportCatalogue catalogue;
    input_reader::ReadBusAndStopsInfo(catalogue,std::cin);
    stat_reader::ProvideRequestedInfo(std::cout,catalogue,std::cin);
}
