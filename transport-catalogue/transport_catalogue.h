#pragma once

#include <deque>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "domain.h"

namespace transport_catalogue {

using Bus = domain::Bus;
using Stop = domain::Stop;

class TransportCatalogue {
public:
  TransportCatalogue() = default;
  const Bus *Emplace(Bus &&bus);
  const Stop *Emplace(Stop &&stop);
  void AddDistance(const Stop *from, const Stop *to, double distance);
  double GetDistance(const Stop *from, const Stop *to) const;
  const domain::Catalogue<Bus> &GetBuses() const { return buses_; }
  const domain::Catalogue<Stop> &GetStops() const { return stops_; }
  const std::unordered_map<const Stop *, std::unordered_set<const Bus *>> &
  GetStopsToBuses() const {
    return stop_to_buses_;
  }
  const std::unordered_map<std::pair<const Stop *, const Stop *>, double,
                           domain::pair_hash> &
  GetStopToStopDistance() const {
    return stop_to_stop_distance_;
  }

private:
  domain::Catalogue<Bus> buses_;
  domain::Catalogue<Stop> stops_;
  std::unordered_map<const Stop *, std::unordered_set<const Bus *>>
      stop_to_buses_;
  std::unordered_map<std::pair<const Stop *, const Stop *>, double,
                     domain::pair_hash>
      stop_to_stop_distance_;
};

} // namespace transport_catalogue
