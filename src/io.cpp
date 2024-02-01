#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "io.hpp"

using json = nlohmann::json;

struct SymbolPair {
    std::string start_symbol;
    std::string end_symbol;
};

std::unordered_map<std::string, SymbolPair> symbol_map;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string fetch_data(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        // curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 4);

        int max_attempts = 5;
        for (int attempt = 1; attempt <= max_attempts; ++attempt) {
            res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                break;
            }
        }
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

std::unordered_map<std::string, std::unordered_map<std::string, double>> update_market_crypto_rates() {
    std::string data = fetch_data("https://api.binance.us/api/v3/ticker/price");
    static std::unordered_map<std::string, std::unordered_map<std::string, double>> reformatted_data;
    json json_data = json::parse(data);
    
    for (const auto& item : json_data) {
        std::string symbol = item["symbol"].get<std::string>();
        if (symbol_map.count(symbol) == 0) continue; //only known coins can be included
        SymbolPair pair = symbol_map[symbol];

        reformatted_data[pair.end_symbol][pair.start_symbol] = std::stof(item["price"].get<std::string>());
        reformatted_data[pair.start_symbol][pair.end_symbol] = 1 / std::stof(item["price"].get<std::string>());
    }

    return reformatted_data;
}

void io_init() {
    std::string symbol_list = fetch_data("https://api.binance.us/api/v3/exchangeInfo?permissions=SPOT");
    
    json json_data = json::parse(symbol_list);

    for (const auto& symbol : json_data["symbols"]) {
        std::string symbol_str = symbol["symbol"].get<std::string>();
        if (symbol_str == "" || symbol_str == "MXC") {
            continue;
        }
        if (symbol["baseAsset"].get<std::string>() == "MXC" || symbol["quoteAsset"].get<std::string>() == "MXC") {
            continue;
        }
        symbol_map[symbol["symbol"].get<std::string>()] = SymbolPair{symbol["baseAsset"].get<std::string>(), symbol["quoteAsset"].get<std::string>()};
    }
}