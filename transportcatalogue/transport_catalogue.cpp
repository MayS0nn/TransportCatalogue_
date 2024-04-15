#include "transport_catalogue.h"


namespace catalogue
{

	void TransportCatalogue::AddBus(const std::string& name, const std::vector<Stop*>& stops_input, bool is_roundtrip)
	{
		all_buses.push_back(Bus(name, stops_input, is_roundtrip));

		Bus* add = &all_buses.back();
		for (const Stop* stop : stops_input) {
			stops_bus[stop->name_][add->name_] = add;
		}
		buses_base_[add->name_] = add;
	}

	void TransportCatalogue::AddStop(const std::string& stop_name, geo::Coordinates coordinates)
	{
		all_stops.emplace_back(stop_name, coordinates);
		auto* added_stop = &all_stops.back();
		stops_bus[added_stop->name_]; // Создаем пустой словарь остановок для данной остановки
		stops_base_[added_stop->name_] = added_stop;
	}

	//было сделано через обычный оператор if, но потом переделал на тернарный
	const Bus* TransportCatalogue::FindBus(const std::string_view& name)const
	{
		return buses_base_.count(name) ? buses_base_.at(name) : nullptr;
	}

	const Stop* TransportCatalogue::FindStop(const std::string_view& stop_name)const
	{

		return stops_base_.count(stop_name) ? stops_base_.at(stop_name) : nullptr;
	}

	Bus* TransportCatalogue::FindBus(const std::string_view& name)
	{
		return buses_base_.count(name) ? buses_base_.at(name) : nullptr;
	}

	Stop* TransportCatalogue::FindStop(const std::string_view& stop_name)
	{
		return stops_base_.count(stop_name) ? stops_base_.at(stop_name) : nullptr;
	}

	BusInfo TransportCatalogue::GetBusInfo(const std::string_view route)
	{
		BusInfo bus_info;
		if (buses_base_.count(route))
		{
			bus_info.bus_number_ = route;
			auto& Bus = buses_base_[route];
			std::vector<domain::Stop*> tmp = Bus->stops_;
			bus_info.stops_count_ = Bus->stops_.size();
			std::sort(tmp.begin(), tmp.end());
			auto last = std::unique(tmp.begin(), tmp.end());
			bus_info.unique_stops_ = (last != tmp.end() ? std::distance(tmp.begin(), last) : tmp.size());
			if (bus_info.stops_count_ > 1)
			{
				for (int i = 0; i < bus_info.stops_count_ - 1; ++i)
				{
					bus_info.geo_route_length_ += geo::ComputeDistance(Bus->stops_[i]->coordinates_, Bus->stops_[i + 1]->coordinates_);
					bus_info.meters_route_length_ += GetDistance(Bus->stops_[i], Bus->stops_[i + 1]);
				}
				bus_info.curvature_ = bus_info.meters_route_length_ / bus_info.geo_route_length_;
			}
			return bus_info;
		}
		else {
			return bus_info;
		}
	}

	const Bus* TransportCatalogue::GetRouteInfo(const std::string_view request) const
	{
		return (buses_base_.count(request) > 0) ? buses_base_.at(request) : nullptr;
		
	}

	const std::map<std::string_view, Bus*> TransportCatalogue::GetBusesInStop(const std::string_view request) const
	{
		return (stops_bus.count(request) > 0) ? stops_bus.at(request) : std::map<std::string_view, Bus*>{};		
	}

	std::deque<Bus> TransportCatalogue::GetAllBus() {
		return all_buses;
	}

	void TransportCatalogue::SetDistance(Stop* from, Stop* to, size_t distance) {
		distance_between_stops_[std::make_pair(from, to)] = distance;	
	}

	// Получение дистанции между остановками
	size_t TransportCatalogue::GetDistance(Stop* from, Stop* to) const {
		// Проверка на nullptr для указателей на остановки
		if (from != nullptr || to != nullptr) {
			auto it = distance_between_stops_.find(std::make_pair(from, to));
			if (it != distance_between_stops_.end()) {
				// Найдена дистанция в прямом направлении
				return it->second;
			}
			else {
				// Если дистанция не найдена, попробуем найти в обратном направлении
				it = distance_between_stops_.find(std::make_pair(to, from));
				return (it != distance_between_stops_.end()) ? it->second : 0;

			}
		}
		return 0;
		
	}

	std::unordered_map<std::pair<const Stop*, const Stop*>, size_t, TransportCatalogue::DistanceHasher> TransportCatalogue::GetStopsFromTo()
	{
		return distance_between_stops_;
	}

}