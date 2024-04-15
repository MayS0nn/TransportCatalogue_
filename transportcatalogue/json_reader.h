#pragma once

#include "json.h"
#include "map_renderer.h"
#include <iostream>
#include <string>

namespace json {
	class jsonreader {

	public:
		explicit jsonreader(
			catalogue::TransportCatalogue& t_c,
			svg::Document& result_map_render);

		void LoadJSON(std::istream& input);

		void PrintSvg();
		void PrintAnswer();

	private:

		void FillSettingsAndTakeMap();
		void FillStop();
		void FillBus();
		json::Dict PrintSvgToJson(std::string result_map_render, int id);
		json::Dict PrintStop(const json::Node& node_map, int id);
		json::Dict PrintBus(const json::Node& node_map, int id);

		catalogue::TransportCatalogue& catalogue;
		svg::Document result_map_render_;
		std::string result_map_renderJSON_;
		json::Dict render_set_;
		json::Array base_requests_;
		json::Array stat_requests_;
	};
}
