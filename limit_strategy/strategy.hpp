//
// Created by Роман Агеев on 20/11/22.
//

#ifndef EVOL_LIMIT_STRATEGY_HPP
#define EVOL_LIMIT_STRATEGY_HPP

#include <cstddef>
#include <array>
#include <iostream>
#include "states.hpp"

namespace Limit {

    class LimitStrategy {
    public:
        explicit LimitStrategy(ParameterCalculator stop_loss_calculator, ParameterCalculator gap_calculator);

        void process(const CandleType &candle, Action action);

        void exit(const CandleType &last_candle);

        void reset();

        Stats stats() const;

        void reset_stats();

    protected:

        void to_start();

        void to_wait_long(PointType high);

        void to_wait_short(PointType low);

        void to_long(PointType loss_threshold);

        void to_short(PointType loss_threshold);

        void to_short_wait_long(PointType high, PointType loss_threshold);

        void to_long_wait_short(PointType low, PointType loss_threshold);

        bool is_exit_by_stop_loss_on_enter_long(const CandleType &candle, PointType enter_value,
                                                PointType stop_loss_value) const;

        bool is_enter_long_before_stop_loss_short(const CandleType &candle, PointType enter_value,
                                                  PointType stop_loss_value) const;

        bool is_exit_by_stop_loss_on_enter_short(const CandleType &candle, PointType enter_value,
                                                 PointType stop_loss_value) const;

        bool is_enter_short_before_stop_loss_long(const CandleType &candle, PointType enter_value,
                                                  PointType stop_loss_value) const;

        PointType stop_loss_long(PointType enter) const;

        PointType stop_loss_short(PointType enter) const;

        PointType gap(PointType price) const;

        bool is_exit_by_stop_loss_long(const CandleType &candle, PointType loss_threshold) const;

        bool is_exit_by_stop_loss_short(const CandleType &candle, PointType loss_threshold) const;

        void enter_long(Time time, PointType price);

        void enter_short(Time time, PointType price);

        void escape_deal_by_stop_loss(Time time, PointType price);

        PointType move_stop_loss_long(const CandleType &candle, PointType current_stop_loss) const;

        PointType move_stop_loss_short(const CandleType &candle, PointType current_stop_loss) const;

        virtual bool should_move_gap_long(PointType old_value, PointType new_value) const;

        virtual bool should_move_gap_short(PointType old_value, PointType new_value) const;

    private:
        Limit::StartState start_state;
        Limit::LongState long_state;
        Limit::ShortState short_state;
        Limit::WaitLongState wait_long_state;
        Limit::WaitShortState wait_short_state;
        Limit::LongWaitShortState long_wait_short_state;
        Limit::ShortWaitLongState short_wait_long_state;

        BinanceDealTracker deal_tracker;

        Limit::LimitState *state;
        ParameterCalculator stop_loss_calculator;
        ParameterCalculator gap_calculator;

        friend class StartState;

        friend class LongState;

        friend class ShortState;

        friend class WaitLongState;

        friend class WaitShortState;

        friend class LongWaitShortState;

        friend class ShortWaitLongState;
    };
}

#endif //EVOL_LIMIT_STRATEGY_HPP
