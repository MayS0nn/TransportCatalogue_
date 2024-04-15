#pragma once

namespace geo {

    struct Coordinates {
        Coordinates(double lati, double lang):lat(lati), lng(lang){}
        double lat; // Широта
        double lng; // Долгота
        bool operator==(const Coordinates& other) const {
            return lat == other.lat && lng == other.lng;
        }
        bool operator!=(const Coordinates& other) const {
            return !(*this == other);
        }
    };

    double ComputeDistance(Coordinates from, Coordinates to);

} // namespace geo