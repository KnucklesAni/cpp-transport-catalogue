#pragma once

#include <deque>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "geo.h"

namespace domain {

struct pair_hash {
  template <class T1, class T2>
  std::size_t operator()(const std::pair<T1, T2> &p) const {
    auto h1 = std::hash<T1>{}(p.first);
    auto h2 = std::hash<T2>{}(p.second);

    // Mainly for demonstration purposes, i.e. works but is overly simple
    // In the real world, use sth. like boost.hash_combine
    return h1 ^ h2 * 997;
  }
};

struct Stop {
  std::string name;
  // 1024, 1024 is impossible lattitude/longitude.
  // We use it for Stop which doesn't have coordinates yet.
  geo::Coordinates coordinates = {1024, 1024};
  bool coordinates_present() const {
    return coordinates.lat != 1024 || coordinates.lng != 1024;
  }
};

enum class RouteType { BackAndForth, Round };

struct Bus {
  std::string name;
  RouteType route_type;
  std::vector<const Stop *> route = {};
};

// Catalogue with stable addres of elements.
// Elements are inserted into deque and then string_view for name field is used
// as key in the unordered_map.
template <typename Type> class Catalogue {
public:
  Catalogue() = default;

  const Type *Emplace(Type &&data) {
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

} // namespace domain
