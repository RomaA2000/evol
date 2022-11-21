//
// Created by Роман Агеев on 15.10.2022.
//

#ifndef EVOL_CANDLE_HPP
#define EVOL_CANDLE_HPP

#include <cstddef>
#include <array>
#include <iostream>

enum class Action : int {
    LONG = 1,
    SHORT = -1,
    NONE = 0
};

enum class ParameterCalculationStrategy {
    RELATIVE,
    ABSOLUTE
};

using PointType = double;

template<typename T, size_t Frame>
struct MergedCandle;

template<typename T>
using Candle = MergedCandle<T, 1>;

template<typename T, size_t Frame>
struct MergedCandle : Candle<T> {
    std::array<Candle<T>, Frame> inner_candles;

    MergedCandle(
            T open,
            T high,
            T low,
            T close,
            T volume,
            std::array<Candle<T>, Frame> inner_candles
    ) :
            Candle<T>(open, high, low, close, volume),
            inner_candles(inner_candles) {}

    bool is_exit_on_enter_long(PointType enter_value, PointType stop_loss_value) const {
        auto index = inner_candles.cbegin();
        for (;index != inner_candles.cend(); ++index) {
            if (index->low <= enter_value) {
                break;
            }
        }

        if (index == inner_candles.end()) {
            return false;
        }

        for (;index != inner_candles.cend(); ++index) {
            if (index->low <= stop_loss_value) {
                return true;
            }
        }

        return false;
    }

    bool is_exit_on_enter_short(PointType enter_value, PointType stop_loss_value) const {
        auto index = inner_candles.cbegin();
        for (;index != inner_candles.cend(); ++index) {
            if (index->high >= enter_value) {
                break;
            }
        }

        if (index == inner_candles.end()) {
            return false;
        }

        for (;index != inner_candles.cend(); ++index) {
            if (index->high >= stop_loss_value) {
                return true;
            }
        }

        return false;
    }

    bool is_enter_long_before_stop_loss_short(PointType enter_value, PointType stop_loss_value) const {
        auto index = inner_candles.cbegin();
        for (;index != inner_candles.cend(); ++index) {
            if (index->high >= stop_loss_value) {
                return false;
            }

            if (index->low <= enter_value) {
                return true;
            }
        }

        assert(false);
    }

    bool is_enter_short_before_stop_loss_long(PointType enter_value, PointType stop_loss_value) const {
        auto index = inner_candles.cbegin();
        for (;index != inner_candles.cend(); ++index) {
            if (index->low <= stop_loss_value) {
                return false;
            }

            if (index->high >= enter_value) {
                return true;
            }
        }

        assert(false);
    }

    MergedCandle() = default;
};

using Time = size_t;

template<typename T>
struct MergedCandle<T, 1> {
    Time time_stamp = 0;
    T open = 0;
    T close = 0;
    T high = 0;
    T low = 0;
    T volume = 0;

    MergedCandle() = default;

    MergedCandle(
            T open,
            T close,
            T high,
            T low,
            T volume
    ) :
            open(open),
            close(close),
            high(high),
            low(low),
            volume(volume) {}
};

template<size_t Frame>
using FloatCandle = MergedCandle<float, Frame>;

using CandleType = FloatCandle<5>;

class ParameterCalculator {
public:

    ParameterCalculator(PointType value, ParameterCalculationStrategy strategy);

    PointType calculate(PointType price) const;

private:
    PointType _value;
    ParameterCalculationStrategy _strategy;
};


struct Stats {
    uint64_t positive_count;
    uint64_t deal_count;
    PointType sum;
};

#endif //EVOL_CANDLE_HPP
