#pragma once
#include "geo.h"

#include <string>
#include<vector>
#include <unordered_map>
#include <string_view>

namespace domain
{

	struct Stop 
	{
		Stop(const std::string name, const geo::Coordinates coordinates) :name_(name), coordinates_(coordinates) {}

		std::string name_;
		geo::Coordinates coordinates_;
		std::unordered_map <std::string_view, int> road_distance_;
	};

	struct Bus 
	{
		Bus() = default;
		Bus(const std::string name, const std::vector<Stop*> stops, bool is_round) :name_(name), stops_(stops), is_roundtrip_(is_round) {}

		std::string name_;
		std::vector <Stop*> stops_;
		bool is_roundtrip_{};
	};


	struct BusInfo
	{
		std::string bus_number_{};
		int stops_count_ = 0;
		int unique_stops_ = 0;
		double geo_route_length_ = 0.0;
		double meters_route_length_ = 0;
		double curvature_ = 0.0;
	};
}


/*не знаю насколько это правильно */

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */