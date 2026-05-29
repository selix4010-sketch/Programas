#ifndef CAFE_INVENTARIO_H 
#define CAFE_INVENTARIO_H
#include <string>
#include "json.hpp" 

using namespace std;
using json = nlohmann::ordered_json;

struct CafeInventario {
    int id;
    string nombre;
    string variedad;
    string tueste;
    double precio;
    int gramos; // Control de inventario físico

    // Serialización a JSON
    friend void to_json(json& j, const CafeInventario& c) {
        j = json{
            {"id", c.id},
            {"nombre", c.nombre},
            {"variedad", c.variedad},
            {"tueste", c.tueste},
            {"precio", c.precio},
            {"gramos", c.gramos}
        };
    }

    // Deserialización desde JSON
    friend void from_json(const json& j, CafeInventario& c) {
        if (j.contains("id")) j.at("id").get_to(c.id);
        if (j.contains("nombre")) j.at("nombre").get_to(c.nombre);
        if (j.contains("variedad")) j.at("variedad").get_to(c.variedad);
        if (j.contains("tueste")) j.at("tueste").get_to(c.tueste);
        if (j.contains("precio")) j.at("precio").get_to(c.precio);
        if (j.contains("gramos")) j.at("gramos").get_to(c.gramos);
    }
};

#endif