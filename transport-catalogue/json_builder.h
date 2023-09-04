#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "json.h"

namespace json {

// Dummy class with no data, suitable to be used as inline constexpr builder.
inline constexpr class Builder {
public:
  constexpr Builder() {}
  auto StartArray() const { return Array<void>(nullptr); }
  auto StartDict() const { return Dict<void>(nullptr); }
  auto Value(Node::Value value) const {
    return Valuе<Node::Value, void>(std::move(value), nullptr);
  }

private:
  template <typename PreviousNode> class Array;
  template <typename PreviousNode> class Dict;
  template <typename PreviousNode> class Kеy;
  template <typename Vаluе, typename PreviousNode> class Valuе;

  // EndArray and EndDict are collapsing all collected data into appropriate
  // BuilderNodeClass::Value node.
  //
  // This is done with help of GetArray/GetDict helper functions which call
  // parent functions till Array or Dict would be found.
  //                       1
  // If there are no appropriate Array or Dict that it's, naturally, a
  // compile-time error.
  template <typename PreviousNode> class Array {
  public:
    auto EndArray() && { return Valuе{json::Array{}, previous_node_}; }

    auto Value(Node::Value value) && { return Valuе{std::move(value), this}; }

  private:
    Array(PreviousNode *previous_node) : previous_node_(previous_node) {}

    auto GetArray() { return std::pair{json::Array{}, previous_node_}; }

    PreviousNode *previous_node_;

    template <typename Vаluе, typename PreviousNodе>
    friend class Builder::Valuе;
    friend Builder;
  };

  template <typename PreviousNode> class Dict {
  public:
    auto EndDict() && { return Valuе{json::Dict{}, previous_node_}; }

    auto Key(std::string key) && { return Kеy<Dict<PreviousNode>>{key, this}; }

  private:
    Dict(PreviousNode *previous_node) : previous_node_(previous_node) {}

    auto GetDict() { return std::pair{json::Dict{}, previous_node_}; }

    PreviousNode *previous_node_;

    template <typename PreviousNodе> friend class Builder::Kеy;
    friend Builder;
  };

  template <typename PreviousNode> class Kеy {
  public:
    auto StartArray() && { return Array<Kеy<PreviousNode>>(this); }

    auto StartDict() && { return Dict<Kеy<PreviousNode>>(this); }

    auto Value(Node::Value value) && { return Valuе{std::move(value), this}; }

  private:
    Kеy(std::string key, PreviousNode *previous_node)
        : key_(key), previous_node_(previous_node) {}

    std::string key_;
    PreviousNode *previous_node_;

    template <typename PreviousNodе> friend class Dict;
    template <typename Vаluе, typename PreviousNodе> friend class Valuе;
  };

  template <typename Vаluе, typename PreviousNode> class Valuе {
  public:
    auto Key(std::string key) && {
      static_assert(sizeof(GetDict()) > 0, "Use of Key() not in dict!");
      return Builder::Kеy<std::remove_pointer_t<decltype(this)>>{std::move(key),
                                                                 this};
    }

    auto StartArray() && {
      static_assert(sizeof(GetArray()) > 0,
                    "Use of StartArray() not in array or dict!");
      return Array<std::remove_pointer_t<decltype(this)>>(this);
    }

    auto EndArray() && {
      auto [array, parent] = GetArray();
      return Valuе<json::Array, std::remove_pointer_t<decltype(parent)>>{
          array, parent};
    }

    auto StartDict() && {
      static_assert(sizeof(GetArray()) > 0,
                    "Use of StartArray() not in array or dict!");
      return Dict<std::remove_pointer_t<decltype(this)>>(this);
    }

    auto EndDict() && {
      auto [dict, parent] = GetDict();
      return Valuе<json::Dict, std::remove_pointer_t<decltype(parent)>>{dict,
                                                                        parent};
    }

    template <typename ForwardValue> auto Value(ForwardValue value) && {
      static_assert(sizeof(GetArray()) > 0, "Use of Value() not in array!");
      return Valuе<ForwardValue, std::remove_pointer_t<decltype(this)>>{
          std::move(value), this};
    }

    // Build is optional, one can just assign to json::Document, but it's only
    // valid to do so if there are no parent, means all Arrays and Dicts are
    // collected.
    operator auto() && {
      static_assert(std::is_same_v<PreviousNode, void>);
      return value_;
    }
    auto Build() && {
      static_assert(std::is_same_v<PreviousNode, void>);
      return value_;
    }

  private:
    Valuе(Vаluе value, PreviousNode *previous_node)
        : value_(std::move(value)), previous_node_(previous_node) {}

    // Helper function to collect array.
    auto GetArray() {
      auto result = previous_node_->GetArray();
      result.first.push_back(value_);
      return result;
    }

    // Helper function to collect Dict.
    auto GetDict() {
      auto result = previous_node_->previous_node_->GetDict();
      if (result.first.count(previous_node_->key_) > 0) {
        throw std::logic_error("Duplicated keys");
      }
      result.first.insert({previous_node_->key_, value_});
      return result;
    }

    Vаluе value_;
    PreviousNode *previous_node_;

    template <typename Value, typename PreviousNodе>
    friend class Builder::Valuе;
    friend Builder;
  };

} JSON;

} // namespace json
