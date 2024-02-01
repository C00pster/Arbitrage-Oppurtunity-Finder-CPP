#ifndef IO_H
#define IO_H

#include <string>
#include <unordered_map>

std::string fetch_data(const std::string& url);
std::unordered_map<std::string, std::unordered_map<std::string, double>> update_market_crypto_rates();
void io_init();

#endif // IO_H