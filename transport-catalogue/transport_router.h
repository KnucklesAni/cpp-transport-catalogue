#pragma once

#include <vector>

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

namespace transport_router {

struct RoutingSettings {
  double bus_velocity = 0.0;
  double bus_wait_time = 0.0;
};

class Router {
public:
  using Bus = transport_catalogue::Bus;
  using Stop = transport_catalogue::Stop;
  using TransportCatalogue = transport_catalogue::TransportCatalogue;

  explicit Router(const RoutingSettings &routing_settings,
                  const TransportCatalogue &transport_catalogue);

  struct ActionWait {
    std::string_view stop_name;
    double time;
  };
  struct ActionBus {
    std::string_view bus_name;
    std::size_t span_count;
    double time;
  };
  struct RouteInfo {
    double weight;
    std::vector<std::pair<ActionWait, ActionBus>> path;
  };
  std::optional<RouteInfo> BuildRoute(const Stop *from, const Stop *to) const;

private:
  const RoutingSettings &routing_settings_;
  std::unordered_map<graph::VertexId, const Stop *> id_to_stop_;
  std::unordered_map<const Stop *, graph::VertexId> stop_to_id_;
  std::unordered_map<graph::EdgeId,
                     std::tuple<const Stop *, const Bus *, std::size_t, double>>
      subroutes_;
  graph::DirectedWeightedGraph<double> graph_;
  graph::Router<double> router_;

  graph::Router<double>
  BuildRouter(const TransportCatalogue &transport_catalogue);
};

} // namespace transport_router
