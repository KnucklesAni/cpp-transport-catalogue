#include "transport_router.h"

#include <algorithm>
#include <deque>
#include <queue>

namespace transport_router {

Router::Router(const RoutingSettings &routing_settings,
               const TransportCatalogue &transport_catalogue)
    : routing_settings_(routing_settings),
      graph_(std::distance(transport_catalogue.GetStops().begin(),
                           transport_catalogue.GetStops().end())),
      router_(BuildRouter(transport_catalogue)) {}

graph::Router<double>
Router::BuildRouter(const TransportCatalogue &transport_catalogue) {
  graph::VertexId stop_id = 0;
  for (auto &[stop_name, stop] : transport_catalogue.GetStops()) {
    id_to_stop_.insert({stop_id, stop});
    stop_to_id_.insert({stop, stop_id++});
  }
  for (auto &[bus_name, bus] : transport_catalogue.GetBuses()) {
    for (auto start_stop = bus->route.begin(); start_stop != bus->route.end();
         start_stop = std::next(start_stop)) {
      double time_forward = routing_settings_.bus_wait_time;
      double time_backward = routing_settings_.bus_wait_time;
      std::size_t stops = 0;
      for (auto prev_stop = start_stop, end_stop = std::next(start_stop);
           end_stop != bus->route.end();
           prev_stop = end_stop, end_stop = next(end_stop)) {
        time_forward += transport_catalogue.GetDistance(*prev_stop, *end_stop) *
                        60.0 / (1000.0 * routing_settings_.bus_velocity);
        subroutes_.insert(
            {graph_.AddEdge({stop_to_id_.at(*start_stop),
                             stop_to_id_.at(*end_stop), time_forward}),
             {*start_stop, bus, ++stops, time_forward}});
        if (bus->route_type == domain::RouteType::BackAndForth) {
          time_backward +=
              transport_catalogue.GetDistance(*end_stop, *prev_stop) * 60.0 /
              (1000.0 * routing_settings_.bus_velocity);
          subroutes_.insert(
              {graph_.AddEdge({stop_to_id_.at(*end_stop),
                               stop_to_id_.at(*start_stop), time_backward}),
               {*end_stop, bus, stops, time_backward}});
        }
      }
    }
  }
  return graph::Router<double>(graph_);
}

std::optional<Router::RouteInfo> Router::BuildRoute(const Stop *from,
                                                    const Stop *to) const {
  if (from == to) {
    return RouteInfo{};
  }
  auto raw_infо = router_.BuildRoute(stop_to_id_.at(from), stop_to_id_.at(to));
  if (!raw_infо.has_value()) {
    return {};
  }
  const graph::Router<double>::RouteInfo &raw_info = *raw_infо;
  RouteInfo info = {raw_info.weight, {}};
  for (const graph::EdgeId id : raw_info.edges) {
    auto &[stop, bus, stops, time] = subroutes_.at(id);
    info.path.push_back(
        {{stop->name, routing_settings_.bus_wait_time},
         {bus->name, stops, time - routing_settings_.bus_wait_time}});
  }
  return info;
}

} // namespace transport_router
