#pragma once

#include <algorithm>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <deque>
#include <string_view>
#include <iostream>
#include <vector>

#include "domain.h"



namespace catalogue
{
	using namespace domain;

	class TransportCatalogue 
	{
	public:



		void AddBus(const std::string& name, const std::vector<Stop*>& stops, bool is_roundtrip);

		void AddStop(const std::string& stop_name, geo::Coordinates coordinates);

		Bus* FindBus(const std::string_view& name);

		const Bus* FindBus(const std::string_view& name)const;

		Stop* FindStop(const std::string_view& stop_name);

		const Stop* FindStop(const std::string_view& stop_name)const;
		
		BusInfo GetBusInfo(const std::string_view route);

		const Bus* GetRouteInfo(const std::string_view)const;

		const std::map<std::string_view, Bus*> GetBusesInStop(const std::string_view) const;

		std::deque<Bus> GetAllBus();

		void SetDistance(Stop* from, Stop* to, size_t distance);

		size_t GetDistance(Stop* from, Stop* to) const;

		class DistanceHasher
		{
		public:
			std::size_t operator()(std::pair<const Stop*, const Stop*> pair_of_stops_)const noexcept
			{
				return hasher_(static_cast<const void*>(pair_of_stops_.first)) * 37 + hasher_(static_cast<const void*>(pair_of_stops_.second));
			}

			std::size_t operator()(const Stop* stop)const noexcept
			{
				return hasher_(static_cast<const void*>(stop)) * 41;
			}

		private:
			std::hash<const void*> hasher_;
		};

		std::unordered_map<std::pair<const Stop*, const Stop*>, size_t, DistanceHasher> GetStopsFromTo();

	private:
		std::unordered_map<std::pair<const Stop*, const Stop*>, size_t, DistanceHasher> distance_between_stops_;
		std::unordered_map<std::string_view, Bus*> buses_base_;
		std::unordered_map<std::string_view, Stop*> stops_base_;
		std::map<std::string_view, std::map<std::string_view, Bus*>> stops_bus;
		std::deque<Stop> all_stops;
		std::deque<Bus> all_buses;

	};

}