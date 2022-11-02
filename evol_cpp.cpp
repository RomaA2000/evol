//
// Created by Роман Агеев on 3/11/22.
//
#include <torch/torch.h>
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
constexpr PointType STOP_LOSS = 2.0E-3;

std::vector<CandleType> candles_from_tensor(torch::Tensor const &candles) {
    std::vector<CandleType> candle_vector;
    candle_vector.reserve(candles.size(0));
    assert(candles.size(1) == 5);

    for (int32_t i = 0; i < candles.size(0); ++i) {
        CandleType candle;
        candle.time_stamp = i;
        candle.open = candles[i][0].item<float>();
        candle.close = candles[i][1].item<float>();
        candle.high = candles[i][2].item<float>();
        candle.low = candles[i][3].item<float>();
        candle.volume = candles[i][4].item<float>();
        candle_vector.push_back(candle);
    }
    return candle_vector;
}

torch::Tensor partial_scores(std::vector<CandleType> const &candles_vector,
                             torch::Tensor const &actions,
                             const int32_t parts_number,
                             const int32_t start,
                             const int32_t end
) {
    auto options =  torch::TensorOptions()
            .dtype(torch::kFloat32)
            .device(torch::kCPU)
            .requires_grad(false);

    int32_t candles_number = actions.size(0);
    int32_t models_number = end - start;
    int32_t parts_size = candles_number / parts_number;
    torch::Tensor scores = torch::zeros({models_number, parts_number, 3}, options);

    auto stop_loss_calculator = ParameterCalculator(STOP_LOSS, ParameterCalculationStrategy::RELATIVE);
    auto market_strategy = MarketStrategy(stop_loss_calculator);
    for (int32_t m = start; m < end; ++m) {
        for (int32_t i = 0; i < candles_vector.size(); ++i) {
            market_strategy.process(candles_vector[i], static_cast<Action>(actions[i][m].item<int>()));
            if ((i + 1) % parts_size == 0) {

                market_strategy.exit(candles_vector[i]);

                auto stats = market_strategy.stats();
                scores[m - start][i / parts_size][0] = stats.sum;
                scores[m - start][i / parts_size][1] = float(stats.deal_count);
                scores[m - start][i / parts_size][2] = float(stats.positive_count);

                market_strategy.reset();
                market_strategy.reset_stats();
            }
        }
    }
    return scores;
}

//using stats_python_type = std::tuple<float, int>;
//std::vector<std::vector<stats_python_type>>
torch::Tensor scores(torch::Tensor const &candles, torch::Tensor const &actions, const int parts_number) {
    assert(actions.size(0) == candles.size(0));

    auto const candles_vector = candles_from_tensor(candles);
    int32_t models_number = actions.size(1);


    int32_t THREAD_COUNT = 16;


    std::vector<std::future<torch::Tensor>> future_scores;
    future_scores.reserve(THREAD_COUNT);

    int32_t tasks_size = models_number / THREAD_COUNT;
    int32_t additional = models_number % THREAD_COUNT;
    int32_t start_idx = 0;

    auto score_lambda = [&] (int32_t s, int32_t e) {return partial_scores(candles_vector, actions, parts_number, s, e);};

    while (start_idx < models_number) {
        int32_t end_idx = start_idx + tasks_size + (additional > 0);
        if (additional > 0) {
            --additional;
        }
        auto tensor_future = std::async(std::launch::async, score_lambda, start_idx,  end_idx);
        future_scores.push_back(std::move(tensor_future));
        start_idx = end_idx;
    }
    std::vector<torch::Tensor> scores;
    scores.reserve(THREAD_COUNT);
    for (auto & f : future_scores) {
        scores.push_back(f.get());
    }
    return torch::cat(scores, 0);
}

PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) {
m.def("scores", &scores, "Getting scores");
}
