#include "json_reader.h"
#include "domain.h"
#include "map_renderer.h"

#include <sstream>
#include <optional>
#include <cstdint>
#include <utility>
#include <vector>
#include <string>

namespace json
{
    using namespace catalogue;
    jsonreader::jsonreader(
        TransportCatalogue& t_c,
        svg::Document& result_map_render)
        : catalogue(t_c), result_map_render_(std::move(result_map_render)) {}

    void jsonreader::LoadJSON(std::istream& input) {
        json::Document doc = json::Load(input);
        stat_requests_ = doc.GetRoot().AsMap().at("stat_requests").AsArray();
        base_requests_ = doc.GetRoot().AsMap().at("base_requests").AsArray();
        render_set_ = doc.GetRoot().AsMap().at("render_settings").AsMap();
        FillStop();
        FillBus();
        FillSettingsAndTakeMap();
        
    }

    void jsonreader::FillSettingsAndTakeMap() {

        render::MapSettings renset(render_set_);
        render::MapRenderer ren(renset, catalogue);
        result_map_renderJSON_ = ren.DocumentPrintJSON();
        //result_map_render_ = ren.DocumentPrint();
    }

    void jsonreader::FillStop() {
        for (const auto& node_map : base_requests_) {
            if (node_map.AsMap().at("type").AsString() == "Stop") {
                catalogue.AddStop(node_map.AsMap().at("name").AsString(), {node_map.AsMap().at("latitude").AsDouble(), node_map.AsMap().at("longitude").AsDouble()});;
            }
        }
        for (const auto& node_map : base_requests_) {
            const auto& map = node_map.AsMap();
            if (map.at("type").AsString() == "Stop") {
                for (const auto& [key, val] : map.at("road_distances").AsMap()) {
                    catalogue.SetDistance(catalogue.FindStop(map.at("name").AsString()), catalogue.FindStop(key), val.AsInt());
                }
            }
        }
    }

    void jsonreader::FillBus() {
        for (const auto& node_map : base_requests_) {
            const auto& map = node_map.AsMap();
            if (map.at("type").AsString() == "Bus") {
                domain::Bus bs;
                bs.name_ = map.at("name").AsString();
                bs.is_roundtrip_ = map.at("is_roundtrip").AsBool();
                auto& buses = map.at("stops").AsArray();
                for (auto& stop_n : buses) {
                    bs.stops_.push_back(catalogue.FindStop(stop_n.AsString()));
                }
                if (!map.at("is_roundtrip").AsBool()) {
                    auto& tmp = bs.stops_;
                    bs.stops_.insert(bs.stops_.end(), std::next(tmp.rbegin()), tmp.rend());
                }
                catalogue.AddBus(bs.name_, bs.stops_, bs.is_roundtrip_);
            }
        }
    }

    json::Dict jsonreader::PrintBus(const json::Node& node_map, int id) {
        using namespace std::literals;
        std::string tmp = node_map.AsMap().at("name").AsString();
        if (catalogue.FindBus(tmp) != nullptr) {
            return
                json::Dict{
                    {{"route_length"},{catalogue.GetBusInfo(tmp).meters_route_length_}},
                    {{"unique_stop_count"},{catalogue.GetBusInfo(tmp).unique_stops_}},
                    {{"stop_count"},{catalogue.GetBusInfo(tmp).stops_count_}},
                    {{"curvature"},{catalogue.GetBusInfo(tmp).curvature_}},
                    {{"request_id"},{id}}
                };
        }
        else {
            return
                json::Dict{
                    { { "request_id"}, { id } },
                    { {"error_message"}, {"not found"s} }
                };
        }
    }


    json::Dict jsonreader::PrintStop(const json::Node& node_map, int id) {
        using namespace std::literals;
        std::string tmp = node_map.AsMap().at("name").AsString();
        if (catalogue.FindStop(tmp) != nullptr) {
            json::Array arr_bus{};
            for (const auto& [elem, _] : catalogue.GetBusesInStop(tmp)) {
                arr_bus.push_back(std::string(elem));
            }
            
            return
                json::Dict{ 
                    {{"buses"},{arr_bus} },
                    {{"request_id"}, {id}} 
                };
        }
        else {
            return 
                json::Dict{
                    {{"error_message"},{"not found"s}},
                    {{"request_id"}, {id}}
            };
        }
    }

    void jsonreader::PrintSvg() {
        result_map_render_.Render(std::cout);
    }

    json::Dict jsonreader::PrintSvgToJson(std::string result_map_render, int id) {
        return json::Dict{
            {{"map"},{result_map_render}},
            {{"request_id"},{id}}
        };
    }

    void jsonreader::PrintAnswer()
    {
        using namespace std::literals;

        json::Array arr{};
        for (const auto& node_map : stat_requests_) {
            const auto& type = node_map.AsMap().at("type").AsString();
            int id_q = node_map.AsMap().at("id").AsInt();
            switch (type[0]) {
            case 'B':
                arr.emplace_back(PrintBus(node_map, id_q));
                break;
            case 'S':
                arr.emplace_back(PrintStop(node_map, id_q));
                break;
            case 'M':
                arr.emplace_back(PrintSvgToJson(result_map_renderJSON_, id_q));
                break;
            default:
                // Обработка недопустимого типа
                std::cerr << "Unsupported type: " << type << std::endl;
                break;
            }
        }

        json::Print(json::Document{ json::Node{arr} }, std::cout);
    }

}
