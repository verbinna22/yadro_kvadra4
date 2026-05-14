
#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>

#include <crow.h>
#include <crow/common.h>
#include <crow/middlewares/cors.h>
#include "json.hpp"

#include "constellation_finder/converter.h"
#include "robinson_cruise/converter.h"
#include "star_visibility/converter.h"

int main(int /*argc*/, const char** /*argv*/) {
    crow::App<crow::CORSHandler> app;
    const std::string bad_response = "{\n  \"status\": \"incorrect_input\"\n}";
    CROW_ROUTE(app, "/api/v1/robinson_cruise")
        .methods(crow::HTTPMethod::Post)([&bad_response](const crow::request& request) {
            if (request.body.size() > static_cast<std::size_t>(5e7)) {
                return crow::response(crow::status::BAD_REQUEST, bad_response);
            }
            try {
                nlohmann::json input = nlohmann::json::parse(request.body);
                nlohmann::json result = process_robinson_cruise(input);
                std::stringstream s;
                s << result;
                return crow::response(s.str());
            } catch (const nlohmann::json::parse_error& error) {
                return crow::response(crow::status::BAD_REQUEST, bad_response);
            } catch (const nlohmann::json::out_of_range& error) {
                return crow::response(crow::status::BAD_REQUEST,
                                      bad_response);
            } catch (const nlohmann::json::type_error& error) {
                return crow::response(crow::status::BAD_REQUEST,
                                      bad_response);
            } catch (const std::invalid_argument& error) {
                return crow::response(crow::status::BAD_REQUEST,
                                      bad_response);
            } catch (const std::runtime_error& error) {
                return crow::response(crow::status::BAD_REQUEST,
                                      bad_response);
            } catch (const std::logic_error& error) {
                return crow::response(crow::status::BAD_REQUEST,
                                      bad_response);
            }
        });
    
    CROW_ROUTE(app, "/api/v1/star_visibility")
        .methods(crow::HTTPMethod::Post)([&bad_response](const crow::request &request) {
            if (request.body.size() > static_cast<std::size_t>(1e7)) {
                return crow::response(crow::status::BAD_REQUEST, bad_response);
            }
            try {
                nlohmann::json input = nlohmann::json::parse(request.body);
                nlohmann::json result = process_star_visibility(input);
                std::stringstream s;
                s << result;
                return crow::response(s.str());
            } catch (const nlohmann::json::parse_error& error) {
                return crow::response(crow::status::BAD_REQUEST, bad_response);
            } catch (const nlohmann::json::out_of_range& error) {
                return crow::response(crow::status::BAD_REQUEST,
                                      bad_response);
            } catch (const nlohmann::json::type_error& error) {
                return crow::response(crow::status::BAD_REQUEST,
                                      bad_response);
            } catch (const std::invalid_argument& error) {
                return crow::response(crow::status::BAD_REQUEST,
                                      bad_response);
            } catch (const std::runtime_error& error) {
                return crow::response(crow::status::BAD_REQUEST,
                                      bad_response);
            }
        });
    
    CROW_ROUTE(app, "/api/v1/constellation_finder")
        .methods(crow::HTTPMethod::Post)([&bad_response](const crow::request& request) {
            if (request.body.size() > static_cast<std::size_t>(1e7)) {
                return crow::response(crow::status::BAD_REQUEST, bad_response);
            }
            try {
                nlohmann::json input = nlohmann::json::parse(request.body);
                nlohmann::json result = process_constellation_finder(input);
                std::stringstream s;
                s << result;
                return crow::response(s.str());
            } catch (const nlohmann::json::parse_error& error) {
                return crow::response(crow::status::BAD_REQUEST, bad_response);
            } catch (const nlohmann::json::out_of_range& error) {
                return crow::response(crow::status::BAD_REQUEST,
                                      bad_response);
            } catch (const nlohmann::json::type_error& error) {
                return crow::response(crow::status::BAD_REQUEST,
                                      bad_response);
            } catch (const std::invalid_argument& error) {
                return crow::response(crow::status::BAD_REQUEST,
                                      bad_response);
            } catch (const std::runtime_error& error) {
                return crow::response(crow::status::BAD_REQUEST,
                                      bad_response);
            } catch (const std::logic_error& error) {
                return crow::response(crow::status::BAD_REQUEST,
                                      bad_response);
            }
        });

    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors.global().origin("*");
    app.bindaddr("0.0.0.0").port(8000).multithreaded().run();
    return 0;
}
