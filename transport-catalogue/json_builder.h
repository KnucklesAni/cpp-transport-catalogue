#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "json.h"

namespace json {

// Dummy class with no data, suitable to be used as inline constexpr builder.
class Builder {
public:
  constexpr Builder() {}
  auto StartArray() const { return Array(EMPTY); }
  auto StartDict() const { return Dict(EMPTY); }
  auto Value(Node::Value value) const { return Valuе(std::move(value)); }

private:
  template <typename PreviousNode> class Array;
  template <typename PreviousNode> class Dict;
  template <typename PreviousNode> class Kеy;
  template <typename Vаluе> class Valuе;
    
  template <typename NotArray> static constexpr bool IsArray = false;

  template <typename NotArray> static constexpr bool IsDict = false;

  template <typename NotArray> static constexpr bool IsKey = false;
  
  // EndArray and EndDict are collapsing all collected data into appropriate
  // BuilderNodeClass::Value node.
  //
  // This is done with help of GetArray/GetDict helper functions which call
  // parent functions till Array or Dict would be found.
  //                       1
  // If there are no appropriate Array or Dict that it's, naturally, a
  // compile-time error.
  template <typename PreviousNode> class Array : private PreviousNode {
  public:
    auto EndArray() & {
      if constexpr (IsArray<PreviousNode>) {
        return PreviousNode::Value(array_);
      } else if constexpr (IsKey<PreviousNode>) {
        return PreviousNode::InsertValue(array_);
      } else {
        static_assert(std::is_same_v<const PreviousNode, decltype(EMPTY)>);
        return Valuе{array_};
      }
    }
    auto EndArray() && {
      if constexpr (IsArray<PreviousNode>) {
        return std::move(*static_cast<PreviousNode *>(this))
            .Value(std::move(array_));
      } else if constexpr (IsKey<PreviousNode>) {
        return std::move(*static_cast<PreviousNode *>(this))
            .InsertValue(std::move(array_));
      } else {
        static_assert(std::is_same_v<const PreviousNode, decltype(EMPTY)>);
        return Valuе{std::move(array_)};
      }
    }

    Array &Value(Node::Value value) & {
      array_.push_back(value);
      return *this;
    }
    Array &&Value(Node::Value value) && {
      array_.push_back(value);
      return std::move(*this);
    }

    auto StartArray() & { return Array<Array<PreviousNode>>{*this}; }
    auto StartArray() && {
      return Array<Array<PreviousNode>>{std::move(*this)};
    }

    auto StartDict() & { return Dict{*this}; }
    auto StartDict() && { return Dict{std::move(*this)}; }

  private:
    json::Array array_;

    explicit Array(const PreviousNode &previous_node)
        : PreviousNode(previous_node) {}
    explicit Array(PreviousNode &&previous_node)
        : PreviousNode(std::move(previous_node)) {}

    template <typename Vаluе> friend class Valuе;
    friend Builder;
  };

  template <typename PreviousNode> class Dict : private PreviousNode {
  public:
    auto EndDict() & {
      if constexpr (IsArray<PreviousNode>) {
        return PreviousNode::Value(dict_);
      } else if constexpr (IsKey<PreviousNode>) {
        return PreviousNode::InsertValue(dict_);
      } else {
        static_assert(std::is_same_v<const PreviousNode, decltype(EMPTY)>);
        return Valuе{dict_};
      }
    }
    auto EndDict() && {
      if constexpr (IsArray<PreviousNode>) {
        return std::move(*static_cast<PreviousNode *>(this))
            .Value(std::move(dict_));
      } else if constexpr (IsKey<PreviousNode>) {
        return std::move(*static_cast<PreviousNode *>(this))
            .InsertValue(std::move(dict_));
      } else {
        static_assert(std::is_same_v<const PreviousNode, decltype(EMPTY)>);
        return Valuе{std::move(dict_)};
      }
    }

    auto Key(std::string key) & { return Kеy{*this, std::move(key)}; }
    auto Key(std::string key) && {
      return Kеy{std::move(*this), std::move(key)};
    }

  private:
    json::Dict dict_;

    explicit Dict(const PreviousNode &previous_node)
        : PreviousNode(previous_node) {}
    explicit Dict(PreviousNode &&previous_node)
        : PreviousNode(std::move(previous_node)) {}

    void CheckDuplicates(const std::string &key) {
      if (dict_.count(key) > 0) {
        throw std::logic_error("Duplicated keys");
      }
    }

    Dict &&InsertValue(std::string &&key, Node::Value &&value) && {
      dict_.insert({std::move(key), std::move(value)});
      return std::move(*this);
    }

    template <typename PreviousNodе> friend class Array;
    template <typename PreviousNodе> friend class Kеy;
    friend Builder;
  };

  template <typename PreviousNode> class Kеy : private PreviousNode {
  public:
    auto StartArray() & { return Array{*this}; }
    auto StartArray() && { return Array{std::move(*this)}; }

    auto StartDict() & { return Dict{*this}; }
    auto StartDict() && { return Dict{std::move(*this)}; }

    auto Value(Node::Value value) & { return InsertValue(std::move(value)); }
    auto Value(Node::Value value) && {
      return std::move(*this).InsertValue(std::move(value));
    }

  private:
    std::string key_;

    Kеy(const PreviousNode &previous_node, std::string &&key)
        : PreviousNode(previous_node), key_(std::move(key)) {
      CheckDuplicates(key_);
    }
    Kеy(PreviousNode &&previous_node, std::string &&key)
        : PreviousNode(std::move(previous_node)), key_(std::move(key)) {
      CheckDuplicates(key_);
    }

    using PreviousNode::CheckDuplicates;

    PreviousNode InsertValue(Node::Value &&value) & {
      PreviousNode previous_node = *this;
      return std::move(previous_node)
          .InsertValue(std::string(key_), std::move(value));
    }
    PreviousNode InsertValue(Node::Value &&value) && {
      return std::move(*this).InsertValue(std::move(key_), std::move(value));
    }
    using PreviousNode::InsertValue;

    template <typename PreviousNodе> friend class Array;
    template <typename PreviousNodе> friend class Dict;
  };

  template <typename Vаluе> class Valuе {
  public:
    // It's not clear why this class is even needed and why Build function
    // is needed, but, apparently, that's how we are supposed to use builders.
#if 0
    operator auto() && {
      return value_;
    }
#endif
    auto Build() && { return value_; }

  private:
    Vаluе value_;

    Valuе(Vаluе value) : value_(std::move(value)) {}

    friend Builder;
  };

  static constexpr struct {
  } EMPTY = {};
};
  template <typename PreviousNode>
  constexpr bool Builder::IsArray<Builder::Array<PreviousNode>> = true;

  template <typename PreviousNode>
  constexpr bool Builder::IsDict<Builder::Dict<PreviousNode>> = true;

  template <typename PreviousNode>
  constexpr bool Builder::IsKey<Builder::Kеy<PreviousNode>> = true;

inline constexpr Builder JSON;

} // namespace json
