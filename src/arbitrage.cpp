#include "io.hpp"

#include <iostream>
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <thread>
#include <fstream>

struct Edge {
    std::string from_currency;
    std::string to_currency;
    double weight;
};

std::vector<Edge> build_edges(const std::unordered_map<std::string, std::unordered_map<std::string, double>>& rates) {
    std::vector<Edge> edges;
    for (const auto& from_currency : rates) {
        for (const auto& to_currency : from_currency.second) {
            Edge e;
            e.from_currency = from_currency.first;
            e.to_currency = to_currency.first;
            e.weight = -std::log(to_currency.second);
            edges.push_back(e);
        }
    }

    return edges;
}

bool find_arbitrage(const std::unordered_map<std::string, std::unordered_map<std::string, double>>& rates) {
    std::vector<Edge> edges = build_edges(rates);
    std::unordered_map<std::string, double> distances;
    for (const auto& currency : rates) {
        distances[currency.first] = std::numeric_limits<double>::infinity();
    }
    distances["BTC"] = 0.0;

    std::unordered_map<std::string, std::string> previous;

    std::string cycle_start;
    for (size_t i = 0; i < rates.size() - 1; i++) {
        for (const auto& edge : edges) {
            if (distances[edge.from_currency] + edge.weight < distances[edge.to_currency]) {
                distances[edge.to_currency] = distances[edge.from_currency] + edge.weight;
                previous[edge.to_currency] = edge.from_currency;
                if (i == rates.size() - 1 - 1) {
                    cycle_start = edge.to_currency;
                    break;
                }
            }
        }
        if (!cycle_start.empty()) {
            break;
        }
    }

    if (!cycle_start.empty()) {
        std::vector<std::string> cycle;
        std::string current = cycle_start;
        do {
            cycle.push_back(current);
            current = previous[current];
        } while (current != cycle_start);
        cycle.push_back(current);

        for (const auto& currency : cycle) {
            std::cout << currency << " -> ";
        }
        std::cout << std::endl;

        return true;
    } else std::cout << "No arbitrage found" << std::endl;
    return false;
}
        
int main(int argc, char* argv[]) {
    bool silent = false;
    if (argc > 2) {
        std::cout << "Usage: " << argv[0] << " [-s|--silent]" << std::endl;
        exit(1);
    } else if (argc == 2) {
        std::string arg = argv[1];
        if (arg == "-s" || arg == "--silent") {
            silent = true;
        } else {
            std::cout << "Usage: " << argv[0] << " [-s|--silent]" << std::endl;
            exit(1);
        }
    }

    try {
        io_init();
    } catch (...) {
        std::cout << "Failed to initialize IO" << std::endl;
        return 1;
    }

    while (true) {
        auto start = std::chrono::high_resolution_clock::now();

        try {
            if (!silent) std::cout << "Updating market rates" << std::endl;
            std::unordered_map<std::string, std::unordered_map<std::string, double>> market = update_market_crypto_rates();
            // nlohmann::json json_market = market;
            // std::ofstream file("market.json");
            // if (file.is_open()) {
            //     file << json_market.dump(4);
            //     file.close();
            // } else std::cout << "Failed to open file" << std::endl;
            // exit(0);

            if (!silent) std::cout << "Finding arbitrage" << std::endl;
            find_arbitrage(market);
        } catch (...) {
            continue;
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        if (duration.count() < 300) {
            if (!silent) std::cout << "Sleeping for " << 300 - duration.count() << " milliseconds" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100 - duration.count()));
        }
    }

    return 0;
}