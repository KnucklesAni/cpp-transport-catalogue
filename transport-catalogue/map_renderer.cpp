#include "map_renderer.h"

using namespace std::literals;

namespace renderer {

bool IsZero(double value) { return std::abs(value) < EPSILON; }

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
  return {(coords.lng - min_lon_) * zoom_coeff_ + padding_,
          (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
}

svg::Document GetSVG(const RenderSettings &render_settings,
                     const domain::Catalogue<domain::Bus> &buses) {
  svg::Document result;
  std::map<std::string_view, const domain::Bus *> sorted_buses;
  std::map<std::string_view, const domain::Stop *> unique_stops;
  for (const auto &bus_info : buses) {
    sorted_buses.insert(bus_info);
    for (const auto stop : bus_info.second->route) {
      unique_stops[stop->name] = stop;
    }
  }

  std::vector<geo::Coordinates> coordinates;
  for (const auto [_, stop_ptr] : unique_stops) {
    coordinates.push_back(stop_ptr->coordinates);
  }

  SphereProjector projector(coordinates.begin(), coordinates.end(),
                            render_settings.width, render_settings.height,
                            render_settings.padding);

  size_t counter = 0;

  for (const auto &[_, bus] : sorted_buses) {
    if (bus->route.empty()) {
      continue;
    }
    std::vector<const domain::Stop *> bus_stops{bus->route.begin(),
                                                bus->route.end()};
    if (bus->route_type == domain::RouteType::BackAndForth) {
      bus_stops.insert(bus_stops.end(), next(bus->route.rbegin()),
                       bus->route.rend());
    }

    size_t color_index = counter % render_settings.color_palette.size();

    svg::Polyline polyline;
    polyline.SetFillColor(svg::NoneColor)
        .SetStrokeColor(render_settings.color_palette[color_index])
        .SetStrokeWidth(render_settings.line_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    for (const auto stop : bus_stops) {
      polyline.AddPoint(projector(stop->coordinates));
    }
    result.Add(polyline);
    ++counter;
  }
  counter = 0;
  for (const auto &[_, bus] : sorted_buses) {
    if (bus->route.empty()) {
      continue;
    }
    size_t color_index = counter % render_settings.color_palette.size();
    svg::Text underlayer;
    underlayer.SetData(bus->name)
        .SetPosition(projector(bus->route.front()->coordinates))
        .SetOffset(render_settings.bus_label_offset)
        .SetFillColor(render_settings.underlayer_color)
        .SetStrokeColor(render_settings.underlayer_color)
        .SetFontFamily("Verdana")
        .SetFontSize(render_settings.bus_label_font_size)
        .SetFontWeight("bold")
        .SetStrokeWidth(render_settings.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    svg::Text text;
    text.SetData(bus->name)
        .SetPosition(projector(bus->route.front()->coordinates))
        .SetOffset(render_settings.bus_label_offset)
        .SetFontSize(render_settings.bus_label_font_size)
        .SetFontFamily("Verdana")
        .SetFontWeight("bold")
        .SetFillColor(render_settings.color_palette[color_index]);

    result.Add(underlayer);
    result.Add(text);

    if (bus->route_type == domain::RouteType::BackAndForth &&
        bus->route.front() != bus->route.back()) {
      result.Add(
          underlayer.SetPosition(projector(bus->route.back()->coordinates)));
      result.Add(text.SetPosition(projector(bus->route.back()->coordinates)));
    }
    ++counter;
  }
  for (const auto &[_, stop] : unique_stops) {
    svg::Circle stop_symbol;
    stop_symbol.SetCenter(projector(stop->coordinates))
        .SetRadius(render_settings.stop_radius)
        .SetFillColor("white");
    result.Add(stop_symbol);
  }

  for (const auto &[_, stop] : unique_stops) {
    svg::Text underlayer;
    underlayer.SetData(stop->name)
        .SetPosition(projector(stop->coordinates))
        .SetOffset(render_settings.stop_label_offset)
        .SetFontSize(render_settings.stop_label_font_size)
        .SetFontFamily("Verdana")
        .SetFillColor(render_settings.underlayer_color)
        .SetStrokeColor(render_settings.underlayer_color)
        .SetStrokeWidth(render_settings.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    svg::Text text;
    text.SetData(stop->name)
        .SetPosition(projector(stop->coordinates))
        .SetOffset(render_settings.stop_label_offset)
        .SetFontSize(render_settings.stop_label_font_size)
        .SetFontFamily("Verdana")
        .SetFillColor("black");

    result.Add(underlayer);
    result.Add(text);
  }

  return result;
}

} // namespace renderer