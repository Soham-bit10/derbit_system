# derbit_system
cd path/to/deribit_trading_system git init


// deribit_trading_system.cpp
// High-performance order execution & management system for Deribit Test

#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

//----------------------------------------
// Helper functions for libcurl
//----------------------------------------
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

//----------------------------------------
// REST API Client using libcurl
//----------------------------------------
class DeribitClient {
    std::string client_id;
    std::string client_secret;
    std::string access_token;
    std::mutex token_mutex;
    const std::string base_url = "https://test.deribit.com/api/v2";

public:
    DeribitClient(const std::string& id, const std::string& secret)
        : client_id(id), client_secret(secret) {
        authenticate();
    }

    void authenticate() {
        std::string url = base_url + "/public/auth?grant_type=client_credentials&client_id=" + client_id + "&client_secret=" + client_secret;

        CURL* curl = curl_easy_init();
        std::string response;
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            json result = json::parse(response);
            std::lock_guard<std::mutex> lock(token_mutex);
            access_token = result["result"]["access_token"];
        }
    }

    json request(const std::string& endpoint, const std::string& method = "GET", const json& data = {}) {
        std::string url = base_url + endpoint;
        CURL* curl = curl_easy_init();
        std::string response;

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + access_token).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            if (method == "POST") {
                std::string json_data = data.dump();
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
            }

            CURLcode res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
        }

        return json::parse(response);
    }

    json place_order(const std::string& instrument, const std::string& type, double price, double quantity) {
        std::string endpoint = "/private/" + std::string(type == "buy" ? "buy" : "sell");
        json data = {
            {"instrument_name", instrument},
            {"amount", quantity},
            {"type", "limit"},
            {"price", price}
        };
        return request(endpoint, "POST", data);
    }

    json cancel_order(const std::string& order_id) {
        return request("/private/cancel", "POST", {{"order_id", order_id}});
    }

    json get_orderbook(const std::string& instrument) {
        return request("/public/get_order_book?instrument_name=" + instrument);
    }

    json get_positions() {
        return request("/private/get_positions?currency=BTC");
    }
};

//----------------------------------------
// WebSocket Server & Market Data (Stub)
//----------------------------------------
class WebSocketServer {
public:
    void start() {
        std::cout << "[WebSocket] Server running (stub) — implement with uWebSockets or Boost.Asio." << std::endl;
        // Accept clients, manage subscriptions, broadcast updates
    }
};

//----------------------------------------
// Main Execution
//----------------------------------------
int main() {
    DeribitClient client("your_client_id", "your_client_secret");
    WebSocketServer ws_server;

    std::thread ws_thread([&]() {
        ws_server.start();
    });

    // --- Demo ---
    auto order_response = client.place_order("BTC-PERPETUAL", "buy", 65000, 10);
    std::cout << "Order placed: " << order_response.dump(4) << std::endl;

    auto book = client.get_orderbook("BTC-PERPETUAL");
    std::cout << "Order Book: " << book.dump(4) << std::endl;

    ws_thread.join();
    return 0;
}
