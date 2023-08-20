#include <iostream>
#include <stdexcept>

#include "transport_catalogue.h"

namespace transport_catalogue {

const Bus *TransportCatalogue::Emplace(Bus &&bus) {
  if (buses_.At(bus.name).has_value()) {
    throw std::out_of_range("Bus already exist");
  }
  const Bus *placed_bus = buses_.Emplace(std::move(bus));
  for (const Stop *stop : placed_bus->route) {
    stop_to_buses_[stop].insert(placed_bus);
  }
  return placed_bus;
}

const Stop *TransportCatalogue::Emplace(Stop &&stop) {
  if (auto old_stop = stops_.At(stop.name); old_stop.has_value()) {
    auto оld_stop = *old_stop;
    if (оld_stop->coordinates_present()) {
      throw std::out_of_range("Stop already exist");
    }
    оld_stop->coordinates.lat = stop.coordinates.lat;
    оld_stop->coordinates.lng = stop.coordinates.lng;
    return *old_stop;
  }
  const Stop *placed_stop = stops_.Emplace(std::move(stop));
  // We only need to insert empty set which operator[] does, we don't need to do
  // anything else.
  stop_to_buses_[placed_stop];
  return placed_stop;
}
void TransportCatalogue::AddDistance(const Stop *from, const Stop *to,
                                     double distance) {
  stop_to_stop_distance_[{from, to}] = distance;
}
double TransportCatalogue::GetDistance(const Stop *from, const Stop *to) const {
  if (stop_to_stop_distance_.count({from, to}) > 0) {
    return stop_to_stop_distance_.at({from, to});
  } else if (stop_to_stop_distance_.count({to, from}) > 0) {
    return stop_to_stop_distance_.at({to, from});
  } else {
    return ComputeDistance(from->coordinates, to->coordinates);
  }
}
} // namespace transport_catalogue
