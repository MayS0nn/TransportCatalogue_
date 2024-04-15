#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include <iostream>

int main() {

    catalogue::TransportCatalogue catalogue;
    svg::Document result_map_render;

    json::jsonreader json_reader(catalogue,result_map_render);

    json_reader.LoadJSON(std::cin);
    

     //json_reader.PrintSvg();

    json_reader.PrintAnswer();

}