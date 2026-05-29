#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #ifndef UNIX_PATH_MAX
    #define UNIX_PATH_MAX 108
    struct sockaddr_un {
        ADDRESS_FAMILY sun_family;
        char sun_path[UNIX_PATH_MAX];
    };
    #endif
#endif

#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <mutex>

#include "httplib.h"    
#include "json.hpp"     
#include "cafe_inventario.h"  // Tu nueva estructura

using namespace std;
using namespace httplib;

vector<CafeInventario> inventario_db;
int proximo_id = 1;
mutex db_mutex; 
const string archivo_datos = "inventario.dat"; // Archivo de persistencia

// Escribir en el archivo
void guardar_inventario_con_lock_adquirido() {
    json j_array = json::array(); 
    for (const auto& c : inventario_db) {
        j_array.push_back(c); 
    }

    ofstream archivo(archivo_datos);
    if (archivo.is_open()) {
        archivo << j_array.dump(4);
        archivo.close();
    } else {
        cerr << "Error al abrir " << archivo_datos << " para escritura." << endl;
    }
}

// Leer del archivo al arrancar
void cargar_inventario() {
    lock_guard<mutex> lock(db_mutex); 
    ifstream archivo(archivo_datos);
    
    if (archivo.is_open()) {
        try {
            json j_array_leido; 
            archivo >> j_array_leido;
            if (j_array_leido.is_array()) { 
                for (const auto& j_cafe : j_array_leido) {
                    CafeInventario c = j_cafe.get<CafeInventario>(); 
                    inventario_db.push_back(c);
                    if (c.id >= proximo_id) {
                        proximo_id = c.id + 1;
                    }
                }
            }
            cout << "Inventario cargado desde " << archivo_datos << endl;
        } catch (const exception& e) {
            cerr << "Error procesando " << archivo_datos << ": " << e.what() << endl;
        }
        archivo.close();
    } else {
        cout << "Iniciando con base de datos de inventario vacia." << endl;
    }
}

int main(void) {
    Server svr;
    cargar_inventario(); 

    // POST: Crear nuevo café
    svr.Post("/cafe", [&](const Request &req, Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");

        try {
            json j_body = json::parse(req.body); 
            CafeInventario c_nuevo;
            from_json(j_body, c_nuevo); 

            lock_guard<mutex> lock(db_mutex); 
            c_nuevo.id = proximo_id++; 
            inventario_db.push_back(c_nuevo);
            guardar_inventario_con_lock_adquirido(); 

            json j_respuesta = c_nuevo; 
            res.set_content(j_respuesta.dump(4), "application/json");
            res.status = 201; 
        } catch (const exception& e) {
            res.status = 500; 
            res.set_content("Error: " + string(e.what()), "text/plain");
        }
    });

    // GET: Obtener todos
    svr.Get("/cafes", [&](const Request &req, Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        
        lock_guard<mutex> lock(db_mutex);
        json j_array_respuesta = inventario_db;
        res.set_content(j_array_respuesta.dump(4), "application/json");
    });

    // GET: Obtener uno solo por ID
    svr.Get(R"(/cafe/(\d+))", [&](const Request &req, Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        int id_buscado = stoi(req.matches[1].str()); 
        
        lock_guard<mutex> lock(db_mutex);
        auto it = find_if(inventario_db.begin(), inventario_db.end(), 
                          [id_buscado](const CafeInventario& c){ return c.id == id_buscado; });

        if (it != inventario_db.end()) { 
            json j_respuesta = *it; 
            res.set_content(j_respuesta.dump(4), "application/json");
        } else {
            res.status = 404; 
            res.set_content("No encontrado", "text/plain");
        }
    });

    // PUT: Actualizar por ID
    svr.Put(R"(/cafe/(\d+))", [&](const Request &req, Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        int id_buscado = stoi(req.matches[1].str());
        
        try {
            json j_actualizacion = json::parse(req.body);
            
            lock_guard<mutex> lock(db_mutex);
            auto it = find_if(inventario_db.begin(), inventario_db.end(), 
                              [id_buscado](const CafeInventario& c){ return c.id == id_buscado; });

            if (it != inventario_db.end()) {
                if (j_actualizacion.contains("nombre")) it->nombre = j_actualizacion["nombre"].get<string>();
                if (j_actualizacion.contains("variedad")) it->variedad = j_actualizacion["variedad"].get<string>();
                if (j_actualizacion.contains("tueste")) it->tueste = j_actualizacion["tueste"].get<string>();
                if (j_actualizacion.contains("precio")) it->precio = j_actualizacion["precio"].get<double>();
                if (j_actualizacion.contains("gramos")) it->gramos = j_actualizacion["gramos"].get<int>();
                
                guardar_inventario_con_lock_adquirido();
                json j_respuesta = *it; 
                res.set_content(j_respuesta.dump(4), "application/json");
            } else {
                res.status = 404;
                res.set_content("No encontrado", "text/plain");
            }
        } catch (const exception& e) {
            res.status = 500; 
            res.set_content("Error: " + string(e.what()), "text/plain");
        }
    });

    // DELETE: Eliminar por ID
    svr.Delete(R"(/cafe/(\d+))", [&](const Request& req, Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        int id_buscado = stoi(req.matches[1].str());

        lock_guard<mutex> lock(db_mutex);
        auto it = find_if(inventario_db.begin(), inventario_db.end(),
                          [id_buscado](const CafeInventario& c) { return c.id == id_buscado; });

        if (it != inventario_db.end()) {
            inventario_db.erase(it); 
            guardar_inventario_con_lock_adquirido();
            res.status = 200; 
            res.set_content("{\"status\":\"ok\"}", "application/json");
        } else {
            res.status = 404;
        }
    });
    
    svr.Options(R"(.*)", [](const Request& req, Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.status = 200; 
    });
    
    cout << "Sistema de Inventario en linea: http://127.0.0.1:8080" << endl;
    svr.listen("127.0.0.1", 8080); 

    return 0;
}