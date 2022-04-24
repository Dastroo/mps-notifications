//
// Created by dawid on 03.04.2022.
//

#include <csignal>

#include <httplib.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/reader.h>

#include <log_helper/Log.h>
#include <mps_utils/SvrDir.h>

#include "../include/Firebase.h"
#include "../include/API.h"
#include "../include/Services.h"

void API::run() {
    Log::init(mps::SvrDir::var().append("logs/notifications/log"));
    DBHelper::set_default_path(mps::SvrDir::var().append("database.db3"));
    Log::i("DBHelper", DBHelper().get_db_full_path());

    static httplib::Server api;
    if (!api.is_valid()) {
        Log::e(TAG, "failed to initialize server");
        throw std::exception();
    }

    Firebase firebase;
    Services services;
    Json::FastWriter writer;

    api.Post("/connect", [&services](const httplib::Request &req, httplib::Response &res) {
        Log::i(TAG, req.path, req.body);

        //  PARSE JSON
        Json::Value value;
        Json::Reader reader;
        if (!reader.parse(req.body, value, false))
            Log::e(TAG, req.path, reader.getFormattedErrorMessages());

        //  GET VALUES
        std::string service;
        if (value.isMember("service"))
            service = value["service"].asString();

        services.connect_service(service);

        Json::Value jor;
        jor["error"] = false; // false means no error

        std::string response = Json::FastWriter().write(jor);
        res.set_content(response, "application/json");
        response.pop_back(); // to prevent newline char
        Log::i(TAG, req.path, response);
    });

    api.Post("/disconnect", [&services, &writer](const httplib::Request &req, httplib::Response &res) {
        Log::i(TAG, req.path, req.body);

        //  PARSE JSON
        Json::Value value;
        Json::Reader reader;
        if (!reader.parse(req.body, value, false))
            Log::e(TAG, req.path, reader.getFormattedErrorMessages());

        //  GET VALUES
        std::string service;
        if (value.isMember("service"))
            service = value["service"].asString();

        services.disconnect_service(service);

        Json::Value jor;
        jor["error"] = false; // false means no error

        std::string response = writer.write(jor);
        res.set_content(response, "application/json");
        response.pop_back(); // to prevent newline char
        Log::i(TAG, req.path, response);
    });

    api.Post("/notify_clients", [&firebase, &services](const httplib::Request &req, httplib::Response &res) {
        Log::i(TAG, req.path, req.body);

        //  PARSE JSON
        Json::Value value;
        Json::Reader reader;
        if (!reader.parse(req.body, value, false))
            Log::e(TAG, req.path, reader.getFormattedErrorMessages());

        //  EXTRACT VALUES
        std::string response;
        std::string service;
        std::string title;
        std::string body;
        if (value.isMember("service") && value.isMember("title") && value.isMember("body")) {
            service = value["service"].asString();
            title = value["title"].asString();
            body = value["body"].asString();

            std::vector<std::string> firebase_tokens = services.get_firebase_tokens(service);
            firebase.notify_clients(firebase_tokens, title, body);

            response = R"({"status": "ok"})";
        } else
            response = R"({"error": "INVALID_PARAMETER"})";

        res.set_content(response, "application/json");
        Log::i(TAG, req.path, response);
    });

    api.Post("/notify_client", [](const httplib::Request &req, httplib::Response &res) {
        Log::i(TAG, req.path, req.body);

        //TODO: complete
        res.set_content("services!", "application/json");
    });

    // example: curl --request GET -H "client_id: 2" -v "localhost:1618/services?client_id=1&service_id=0"
    api.Get("/services", [&services, &writer](const httplib::Request &req, httplib::Response &res) {
        //  LOG PARAMETERS
        std::ostringstream params;
        for (const auto &param: req.params)
            params << param.first << ":" << param.second;
        Log::i(TAG, req.path, params.str());

        std::string response;
        if (req.has_param("client_id")) {
            try {
                int client_id = std::stoi(req.get_param_value("client_id"));
                Json::Value json_services(Json::arrayValue);
                for (const auto &service: services.get_services()) {
                    Json::Value json_service;
                    json_service["service"] = "btc_spy";

                    //   if its the first time for a user to request services default to notifications off
                    json_service["subscribed"] = services.subscribed(service, client_id);

                    json_services.append(json_service);
                }

                response = writer.write(json_services);
            } catch (const std::invalid_argument &e) {
                response = R"({"error": "INVALID_PARAMETER"})";
            } catch (const std::out_of_range &e) {
                response = R"({"error": "OUT_OF_RANGE"})";
            }
        } else {
            response = R"({"error": "INVALID_PARAMETER"})";
        }

        res.set_content(response, "application/json");
        response.pop_back();
        Log::i(TAG, req.path, response);
    });

    api.Post("/service_on", [&services](const httplib::Request &req, httplib::Response &res) {
        Log::i(TAG, req.path, req.body);

        //  PARSE JSON
        Json::Value value;
        Json::Reader reader;
        if (!reader.parse(req.body, value, false))
            Log::e(TAG, req.path, reader.getFormattedErrorMessages());

        std::string response;
        if (value.isMember("client_id") && value.isMember("service") && value.isMember("firebase_token")) {
            int client_id = value["client_id"].asInt();
            std::string service = value["service"].asString();
            std::string firebase_token = value["firebase_token"].asString();
            services.add_to_service(service, client_id, firebase_token);
            response = R"({"status": "ok"})";
        } else
            response = R"({"error": "INVALID_PARAMETER"})";

        res.set_content(response, "application/json");
        response.pop_back();
        Log::i(TAG, req.path, response);
    });

    api.Post("/service_off", [&services](const httplib::Request &req, httplib::Response &res) {
        Log::i(TAG, req.path, req.body);

        //  PARSE JSON
        Json::Value value;
        Json::Reader reader;
        if (!reader.parse(req.body, value, false))
            Log::e(TAG, req.path, reader.getFormattedErrorMessages());

        std::string response;
        if (value.isMember("client_id") && value.isMember("service")) {
            int client_id = value["client_id"].asInt();
            std::string service = value["service"].asString();
            services.remove_from_service(service, client_id);
            response = R"({"status": "ok"})";
        } else
            response = R"({"error": "INVALID_PARAMETER"})";

        res.set_content(response, "application/json");
        response.pop_back();
        Log::i(TAG, req.path, response);
    });

    api.Post("/services_off", [&services](const httplib::Request &req, httplib::Response &res) {
        Log::i(TAG, req.path, req.body);

        //  PARSE JSON
        Json::Value value;
        Json::Reader reader;
        if (!reader.parse(req.body, value, false))
            Log::e(TAG, req.path, reader.getFormattedErrorMessages());

        std::string response;
        if (value.isMember("client_id")) {
            int client_id = value["client_id"].asInt();
            services.remove_from_services(client_id);
            response = R"({"status": "ok"})";
        } else
            response = R"({"error": "INVALID_PARAMETER"})";

        res.set_content(response, "application/json");
        response.pop_back();
        Log::i(TAG, req.path, response);
    });

    api.Post("/update_firebase_token", [&services](const httplib::Request &req, httplib::Response &res) {
        Log::i(TAG, req.path, req.body);

        //  PARSE JSON
        Json::Value value;
        Json::Reader reader;
        if (!reader.parse(req.body, value, false))
            Log::e(TAG, req.path, reader.getFormattedErrorMessages());

        std::string response;
        if (value.isMember("client_id") && value.isMember("firebase_token")) {
            int client_id = value["client_id"].asInt();
            std::string firebase_token = value["firebase_token"].asString();
            services.update_firebase_token(client_id, firebase_token);
            response = R"({"status": "ok"})";
        } else
            response = R"({"error": "INVALID_PARAMETER"})";

        res.set_content(response, "application/json");
        response.pop_back();
        Log::i(TAG, req.path, response);
    });

    // example: curl --request GET -H "client_id: 2" -v "localhost:1618/services?client_id=1&service_id=0"
    api.Get("/test", [&](const httplib::Request &req, httplib::Response &res) {
        Log::i(TAG, req.path, "method: " + req.method);
        Log::i(TAG, req.path, "body: " + req.body);
        Log::i(TAG, req.path, "target: " + req.target);

        std::ostringstream params;
        for (const auto &param: req.params)
            params << "\n\t" << param.first << '\t' << param.second;
        Log::i(TAG, req.path, "params: " + params.str());

        std::ostringstream headers;
        for (const auto &header: req.headers)
            headers << "\n\t" << header.first << '\t' << header.second;
        Log::i(TAG, req.path, "headers: " + headers.str());

        res.set_content(R"({"test": "test")", "application/json");
    });

    //  SET A WAY TO GRACEFULLY STOP THIS PROCESS
    std::signal(SIGTERM, [](int signum) {
        api.stop();
        Log::release();
        Services().disconnect_services();

        exit(signum);
    });

    api.listen("localhost", 1618);
}
