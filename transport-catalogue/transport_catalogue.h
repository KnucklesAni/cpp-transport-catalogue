#pragma once

#include <deque>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "geo.h"

namespace transport_catalogue {
struct pair_hash {
  template <class T1, class T2>
  std::size_t operator()(const std::pair<T1, T2> &p) const {
    auto h1 = std::hash<T1>{}(p.first);
    auto h2 = std::hash<T2>{}(p.second);

    // Mainly for demonstration purposes, i.e. works but is overly simple
    // In the real world, use sth. like boost.hash_combine
    return h1 ^ h2;
  }
};
struct Stop {
  std::string name;
  // 1024, 1024 is impossible lattitude/longitude.
  // We use it for Stop which doesn't have coordinates yet.
  Coordinates coordinates = {1024, 1024};
  bool coordinates_present() const {
    return coordinates.lat != 1024 || coordinates.lng != 1024;
  }
  friend bool operator==(const Stop &lhs, const Stop &rhs) {
    return lhs.name == rhs.name;
  }
  template <typename OtherType>
  friend bool operator==(const Stop &lhs, const OtherType &rhs) {
    return lhs.name == rhs;
  }
  template <typename OtherType>
  friend bool operator==(const OtherType &lhs, const Stop &rhs) {
    return lhs == rhs.name;
  }
  friend bool operator!=(const Stop &lhs, const Stop &rhs) {
    return lhs.name != rhs.name;
  }
  template <typename OtherType>
  friend bool operator!=(const Stop &lhs, const OtherType &rhs) {
    return lhs.name != rhs;
  }
  template <typename OtherType>
  friend bool operator!=(const OtherType &lhs, const Stop &rhs) {
    return lhs != rhs.name;
  }
};

enum class RouteType { BackAndForth, Round };

struct Bus {
  std::string name;
  RouteType route_type;
  std::vector<const Stop *> route = {};
  friend bool operator==(const Bus &lhs, const Bus &rhs) {
    return lhs.name == rhs.name;
  }
  template <typename OtherType>
  friend bool operator==(const Bus &lhs, const OtherType &rhs) {
    return lhs.name == rhs;
  }
  template <typename OtherType>
  friend bool operator==(const OtherType &lhs, const Bus &rhs) {
    return lhs == rhs.name;
  }
  friend bool operator!=(const Bus &lhs, const Bus &rhs) {
    return lhs.name != rhs.name;
  }
  template <typename OtherType>
  friend bool operator!=(const Bus &lhs, const OtherType &rhs) {
    return lhs.name != rhs;
  }
  template <typename OtherType>
  friend bool operator!=(const OtherType &lhs, const Bus &rhs) {
    return lhs != rhs.name;
  }
};

template <typename Type> class Catalogue {
public:
  Catalogue() = default;

  const Type *Add(Type &&data) {
    Type &emplaced = data_.emplace_back(data);
    name_to_data_.emplace(emplaced.name, &emplaced);
    return &emplaced;
  }

  std::optional<Type *> At(std::string_view name) {
    if (name_to_data_.count(name) > 0) {
      return name_to_data_.at(name);
    }
    return std::nullopt;
  }

  std::optional<const Type *> At(std::string_view name) const {
    if (name_to_data_.count(name) > 0) {
      return name_to_data_.at(name);
    }
    return std::nullopt;
  }

  auto begin() const { return name_to_data_.begin(); }

  auto end() const { return name_to_data_.end(); }

  size_t size() const { return data_.size(); }

private:
  std::deque<Type> data_;
  std::unordered_map<std::string_view, Type *> name_to_data_;
};

class TransportCatalogue {
public:
  TransportCatalogue() = default;
  const Bus *Add(Bus &&bus);
  const Stop *Add(Stop &&stop);
  void AddDistance(const Stop *from, const Stop *to, double distance);
  const Catalogue<Bus> &GetBuses() const { return buses_; }
  const Catalogue<Stop> &GetStops() const { return stops_; }
  const std::unordered_map<std::string_view, std::set<std::string_view>> &
  GetStopsToBuses() const {
    return stop_to_buses_;
  }
  const std::unordered_map<std::pair<const Stop *, const Stop *>, double,
                           pair_hash> &
  GetStopToStopDistance() const {
    return stop_to_stop_distance_;
  }

private:
  Catalogue<Bus> buses_;
  Catalogue<Stop> stops_;
  std::unordered_map<std::string_view, std::set<std::string_view>>
      stop_to_buses_;
  std::unordered_map<std::pair<const Stop *, const Stop *>, double, pair_hash>
      stop_to_stop_distance_;
};

} // namespace transport_catalogue