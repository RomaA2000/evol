//
// Created by Роман Агеев on 15.10.2022.
//

#ifndef EVOL_MARKET_STRATEGY_HPP
#define EVOL_MARKET_STRATEGY_HPP

#include <cstddef>
#include <array>
#include <iostream>
#include "states.hpp"

namespace Market {

    class MarketStrategy {
    public:
        explicit MarketStrategy(ParameterCalculator parameter_calculator);

        void process(const CandleType &candle, Action action);

        void exit(const CandleType &last_candle);

        void reset();

        Stats stats() const;

        void reset_stats();

    protected:

        void to_start();

        void to_short(PointType prev_close_price);

        void to_long(PointType prev_close_price);

        /**
         * Входит в сделку Long. Выходит из предыдущей, если такая была
        */
        void enter_long_by_market(Time time, PointType price);

        /**
         * Входит в сделку Short. Выходит из предыдущей, если такая была
        */
        void enter_short_by_market(Time time, PointType price);

        void escape_deal_by_stop_loss(Time time, PointType price);

        PointType move_stop_loss_long(const CandleType &candle, PointType old_value) const;

        PointType move_stop_loss_short(const CandleType &candle, PointType old_value) const;

        PointType get_stop_loss(PointType price);

    private:
        Market::LongState long_state;
        Market::ShortState short_state;
        Market::StartState start_state;

        BinanceDealTracker deal_tracker;

        Market::MarketState *current_state;
        ParameterCalculator stop_loss_calculator;

        friend class LongState;

        friend class ShortState;

        friend class StartState;

        friend class MarketState;
    };
}

#endif //EVOL_MARKET_STRATEGY_HPP
