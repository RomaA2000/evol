#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <chrono>
#include <cassert>
#include <future>
#include "market_strategy/strategy.hpp"

constexpr PointType DELTA = 3.8000000000000035E-4;
constexpr PointType STOP_LOSS = 5.0E-4;

Action mapper(PointType value) {
    if (std::abs(value) <= DELTA) {
        return Action::NONE;
    } else if (value <= 0) {
        return Action::SHORT;
    } else if (value >= 0) {
        return Action::LONG;
    }
    assert(false);
}

std::vector<std::vector<std::string>> get_lines(std::string f_name) {
    std::ifstream file(f_name);

    std::vector<std::vector<std::string>> tokenized_lines;

    std::vector<std::string> row;

    std::string line, word;
    assert(file.is_open());
    while (getline(file, line)) {
        row.clear();

        std::stringstream str(line);

        while (getline(str, word, ','))
            row.push_back(word);
        tokenized_lines.push_back(row);
    }
    return tokenized_lines;
}

std::vector<std::pair<CandleType, Action>> read_from_csv(std::string f_name) {
    std::vector<std::pair<CandleType, Action>> candles_with_actions;

    auto tokenized_lines = get_lines(f_name);
    for (auto &line: tokenized_lines) {
        assert(line.size() == 8);

        CandleType candle;
        candle.time_stamp = stoi(line[0]);
        candle.close = stod(line[2]);
        candle.open = stod(line[3]);
        candle.high = stod(line[4]);
        candle.low = stod(line[5]);
        candle.volume = stod(line[6]);
        auto action = mapper(stod(line[7]));
        candles_with_actions.emplace_back(candle, action);
    }
    return candles_with_actions;
}

int main() {
    auto candles_with_actions = read_from_csv("../predicts.csv");
    std::cout << candles_with_actions.size() << std::endl;
    auto stop_loss_calculator = ParameterCalculator(STOP_LOSS, ParameterCalculationStrategy::RELATIVE);
    auto market_strategy = MarketStrategy(stop_loss_calculator);

    size_t parts_sizes = 2 * 60 * 24 * 7;

    std::cout << parts_sizes << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t steps_counter = 0; steps_counter < 1000; ++steps_counter) {
        for (size_t i = 0; i < candles_with_actions.size(); ++i) {

            market_strategy.process(candles_with_actions[i].first, candles_with_actions[i].second);

            if ((i % parts_sizes) == (parts_sizes - 1)) {

                market_strategy.exit(candles_with_actions[i].first);

                auto [positive_count, count, sum] = market_strategy.stats();

                market_strategy.reset();
                market_strategy.reset_stats();
            }
        }
    }
    auto stop = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << "Time taken by simulation: " << duration.count() << " microseconds" << std::endl;

    return 0;
}
