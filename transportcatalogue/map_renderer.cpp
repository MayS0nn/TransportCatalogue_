#include "map_renderer.h"

#include <algorithm>
#include <sstream>

namespace sphere {
	bool IsZero(double value) {
		return std::abs(value) < EPSILON;
	}

	svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
		return {
			(coords.lng - min_lon_) * zoom_coeff_ + padding_,
			(max_lat_ - coords.lat) * zoom_coeff_ + padding_
		};
	}
}

namespace render {



	MapSettings::MapSettings(const json::Dict& render_settings) {
		width = render_settings.at("width").AsDouble();
		height = render_settings.at("height").AsDouble();
		padding = render_settings.at("padding").AsDouble();
		line_width = render_settings.at("line_width").AsDouble();
		stop_radius = render_settings.at("stop_radius").AsDouble();
		bus_label_font_size = render_settings.at("bus_label_font_size").AsDouble();
		bus_label_offset.lat = render_settings.at("bus_label_offset").AsArray().front().AsDouble();
		bus_label_offset.lng = render_settings.at("bus_label_offset").AsArray().back().AsDouble();
		stop_label_font_size = render_settings.at("stop_label_font_size").AsDouble();
		stop_label_offset.lat = render_settings.at("stop_label_offset").AsArray().front().AsDouble();
		stop_label_offset.lng = render_settings.at("stop_label_offset").AsArray().back().AsDouble();
		underlayer_width = render_settings.at("underlayer_width").AsDouble();
		underlayer_color = RenderColor(render_settings.at("underlayer_color"));
		for (const auto& color : render_settings.at("color_palette").AsArray())
		{
			color_palette.push_back(RenderColor(color));
		}
	}


	MapRenderer::MapRenderer(const MapSettings& settings, catalogue::TransportCatalogue& t_c)
		: render_settings_(settings)
	{
		AddBusSVG(t_c);
	}
	svg::Color MapRenderer::ColorSetting(uint32_t index) {
		return svg::Color{ render_settings_.color_palette[index % render_settings_.color_palette.size()] };
	}

	svg::Color MapSettings::RenderColor(const json::Node& node)
	{
		{
			svg::Color color;
			if (node.IsArray())
			{
				if (node.AsArray().size() == 3)
				{
					svg::Rgb rgb;
					rgb.red = node.AsArray()[0].AsInt();
					rgb.green = node.AsArray()[1].AsInt();
					rgb.blue = node.AsArray()[2].AsInt();
					color = rgb;
				}
				if (node.AsArray().size() == 4)
				{
					svg::Rgba rgba;
					rgba.red = node.AsArray()[0].AsInt();
					rgba.green = node.AsArray()[1].AsInt();
					rgba.blue = node.AsArray()[2].AsInt();
					rgba.opacity = node.AsArray()[3].AsDouble();
					color = rgba;
				}
			}
			if (node.IsString())
			{
				color = node.AsString();
			}
			return color;
		}
	}

	svg::Polyline MapRenderer::AddRoute(const domain::Bus& bus, const svg::Color& color)
	{
		svg::Polyline route_bus;
		route_bus.SetStrokeColor(color);
		route_bus.SetFillColor("none");
		route_bus.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		route_bus.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		route_bus.SetStrokeWidth(render_settings_.line_width);
		for (auto stop : bus.stops_)
		{
			svg::Point point = sphere_({ stop->coordinates_.lat, stop->coordinates_.lng });
			route_bus.AddPoint(point);
		}
		return route_bus;
	}
	svg::Text MapRenderer::TextSvgForBus(const svg::Point& pos, const std::string& data) const
	{
		return svg::Text().SetPosition(pos)
			.SetOffset({ render_settings_.bus_label_offset.lat, render_settings_.bus_label_offset.lng })
			.SetFontSize(render_settings_.bus_label_font_size)
			.SetFontFamily("Verdana")
			.SetFontWeight("bold")
			.SetData(data);
	}

	svg::Text MapRenderer::CreateTextForBusWithColor(const svg::Point& pos, const svg::Color& color, const std::string& data)
	{
		return TextSvgForBus(pos, data).SetFillColor(color);
	}

	svg::Text MapRenderer::CreateTextForBus(const svg::Point& pos, const std::string& data) const
	{
		return TextSvgForBus(pos, data)
			.SetFillColor(render_settings_.underlayer_color)
			.SetStrokeColor(render_settings_.underlayer_color)
			.SetStrokeWidth(render_settings_.underlayer_width)
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
	}

	std::vector<svg::Text> MapRenderer::AddNameBus(const domain::Bus& bus, const svg::Color& color)
	{
		std::vector<svg::Text>result;

		result.reserve(bus.is_roundtrip_ ? bus.stops_.size() : 2);

		const auto& cord_no_cicle = sphere_({ 
			bus.stops_.front()->coordinates_.lat,
			bus.stops_.front()->coordinates_.lng });
		result.push_back(CreateTextForBus(cord_no_cicle, bus.name_));
		result.push_back(CreateTextForBusWithColor(cord_no_cicle, color, bus.name_));

		if (!bus.is_roundtrip_ && bus.stops_[(bus.stops_.size() + 1) / 2 - 1] != bus.stops_[0])
		{
			const auto& cord = sphere_({ 
				bus.stops_[(bus.stops_.size() + 1) / 2 - 1]->coordinates_.lat,
				bus.stops_[(bus.stops_.size() + 1) / 2 - 1]->coordinates_.lng });
			result.push_back(CreateTextForBus(cord, bus.name_));
			result.push_back(CreateTextForBusWithColor(cord, color, bus.name_));
		}
		return result;
	}

	svg::Text MapRenderer::TextSvgForStop(const svg::Point& pos, const std::string& data) const
	{
		return svg::Text().SetPosition(pos)
			.SetOffset({ render_settings_.stop_label_offset.lat, render_settings_.stop_label_offset.lng })
			.SetFontSize(render_settings_.stop_label_font_size)
			.SetFontFamily("Verdana")
			.SetData(data);
	}

	svg::Text MapRenderer::CreateTextForStopWithColor(const svg::Point& pos, const svg::Color& color, const std::string& data)
	{
		return TextSvgForStop(pos, data).SetFillColor(color);
	}

	svg::Text MapRenderer::CreateTextForStop(const svg::Point& pos, const std::string& data)
	{
		return TextSvgForStop(pos, data)
			.SetFillColor(render_settings_.underlayer_color)
			.SetStrokeColor(render_settings_.underlayer_color)
			.SetStrokeWidth(render_settings_.underlayer_width)
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
	}

	std::vector<ShapeNameStop> MapRenderer::AddNameStops(const domain::Bus& bus)
	{
		std::vector<ShapeNameStop> result;
		for (size_t i = 0; i < bus.stops_.size() - 1; ++i)
		{
			result.push_back({ bus.stops_[i]->name_
				, CreateTextForStopWithColor(sphere_({ bus.stops_[i]->coordinates_.lat, bus.stops_[i]->coordinates_.lng }), "black", bus.stops_[i]->name_)
				, CreateTextForStop(sphere_({ bus.stops_[i]->coordinates_.lat, bus.stops_[i]->coordinates_.lng }),bus.stops_[i]->name_) });
		}
		return result;
	}

	std::vector<ShapeCircleStop> MapRenderer::AddCircleStops(const domain::Bus& bus)
	{
		std::vector<ShapeCircleStop> result;
		for (size_t i = 0; i < bus.stops_.size() - 1; ++i)
		{
			svg::Circle circle;
			circle.SetCenter(sphere_({ bus.stops_[i]->coordinates_.lat, bus.stops_[i]->coordinates_.lng }));
			circle.SetRadius(render_settings_.stop_radius);
			circle.SetFillColor("white");
			result.push_back({ bus.stops_[i]->name_ ,circle });
		}
		return result;
	}

	void MapRenderer::AddBusSVG(catalogue::TransportCatalogue& t_c)
	{
		SetSphereProjector(t_c);
		std::vector<domain::Bus>buses;
		for (const auto& bus : t_c.GetAllBus())
		{
			buses.push_back(bus);
		}
		sort(buses.begin(), buses.end(), [](const domain::Bus& lhs, const domain::Bus& rhs)
			{
				return lhs.name_ < rhs.name_;
			});
		uint32_t index = 0;
		for (size_t i = 0; i < buses.size(); ++i)
		{
			if (buses[i].stops_.size())
			{
				PushBusSVG(buses[i], ColorSetting(index));
				++index;
			}
		}
	}

	void MapRenderer::PushBusSVG(const domain::Bus& bus, const svg::Color& color)
	{
		shape_bus_route.push_back({ AddRoute(bus,color) ,AddNameBus(bus, color) ,AddCircleStops(bus) ,AddNameStops(bus) });
	}

	void MapRenderer::SetSphereProjector(catalogue::TransportCatalogue& t_c)
	{
		std::vector<geo::Coordinates> min_max;
		for (const auto& bus : t_c.GetAllBus())
		{
			std::transform(bus.stops_.begin(), bus.stops_.end(), std::back_inserter(min_max), [](const auto& stop) {
				return stop->coordinates_;
				});
		}
		sphere_ = sphere::SphereProjector(min_max.begin(), min_max.end(), render_settings_.width, render_settings_.height, render_settings_.padding);
	}

	inline Doc MapRenderer::PrepareForOut(const std::vector<BusSVG>& buses) const
	{
		Doc temp;
		for (const auto& bus : buses)
		{
			if (bus.shape_name_stops.size())
			{
				temp.shape_buses.push_back(bus.shape_route_bus);
				for (const auto& n_bus : bus.shape_name_bus)
				{
					temp.shape_name_buses.push_back(n_bus);
				}
				for (const auto& circle_stop : bus.shape_circle_stops)
				{
					temp.shape_circle_stops.insert({ circle_stop.name_stop, circle_stop.shape_stop });
				}
				for (const auto& name_stop : bus.shape_name_stops)
				{
					temp.shape_name_stops.insert({ name_stop.name_stop ,{name_stop.shape_name_stop, name_stop.shape_name_stop_substr } });
				}
			}
		}
		return temp;
	}

	inline svg::Document MapRenderer::DocumentPrint() const
	{
		auto preparedDoc = PrepareForOut(shape_bus_route);
		svg::Document doc;

		for (const auto& line : preparedDoc.shape_buses)
		{
			doc.Add(line);
		}
		for (const auto& text : preparedDoc.shape_name_buses)
		{
			doc.Add(text);
		}
		for (const auto& circle : preparedDoc.shape_circle_stops)
		{
			doc.Add(circle.second);
		}
		for (const auto& stop : preparedDoc.shape_name_stops)
		{
			doc.Add(stop.second.second);
			doc.Add(stop.second.first);
		}
		return doc;
		
	}

	std::string MapRenderer::DocumentPrintJSON() const
	{
		svg::Document doc = DocumentPrint();
		std::stringstream map;
		doc.Render(map);
		return map.str();
	}

}