//
// Created by Роман Агеев on 20/11/22.
//

#include "strategy.hpp"

using namespace Limit;

LimitStrategy::LimitStrategy(ParameterCalculator stop_loss_calculator, ParameterCalculator gap_calculator) :
        start_state(*this), long_state(*this), short_state(*this), wait_long_state(*this), wait_short_state(*this),
        short_wait_long_state(*this), long_wait_short_state(*this), deal_tracker(), state(&start_state),
        stop_loss_calculator(stop_loss_calculator), gap_calculator(gap_calculator) {}

void LimitStrategy::process(const CandleType &candle, Action action) {
    state->process(candle, action);
}

void LimitStrategy::exit(const CandleType &last_candle) {
    if (deal_tracker.is_in_deal()) {
        deal_tracker.escape_deal_by_market(last_candle.time_stamp, last_candle.close);
    }
}

void LimitStrategy::reset() {
    to_start();
}

Stats LimitStrategy::stats() const {
    return {
            .positive_count = deal_tracker.positive_count,
            .deal_count = deal_tracker.count,
            .sum = deal_tracker.sum
    };
}

void LimitStrategy::reset_stats() {
    deal_tracker.reset();
}

void LimitStrategy::to_start() {
    state = &start_state;
}

void LimitStrategy::to_wait_long(PointType high) {
    wait_long_state.bias = high;
    state = &wait_long_state;
}

void LimitStrategy::to_wait_short(PointType low) {
    wait_short_state.bias = low;
    state = &wait_short_state;
}

void LimitStrategy::to_long(PointType loss_threshold) {
    long_state.loss_threshold = loss_threshold;
    state = &long_state;
}

void LimitStrategy::to_short(PointType loss_threshold) {
    short_state.loss_threshold = loss_threshold;
    state = &short_state;
}

void LimitStrategy::to_short_wait_long(PointType high, PointType loss_threshold) {
    short_wait_long_state.bias = high;
    short_wait_long_state.loss_threshold = loss_threshold;
    state = &short_wait_long_state;
}

void LimitStrategy::to_long_wait_short(PointType low, PointType loss_threshold) {
    long_wait_short_state.bias = low;
    long_wait_short_state.loss_threshold = loss_threshold;
    state = &long_wait_short_state;
}

bool LimitStrategy::is_exit_by_stop_loss_on_enter_long(const CandleType &candle, PointType enter_value,
                                                      PointType stop_loss_value) const {
    return candle.is_exit_on_enter_long(enter_value, stop_loss_value);
}

bool LimitStrategy::is_enter_long_before_stop_loss_short(const CandleType &candle, PointType enter_value,
                                                        PointType stop_loss_value) const {
    return candle.is_enter_long_before_stop_loss_short(enter_value, stop_loss_value);
}

bool LimitStrategy::is_exit_by_stop_loss_on_enter_short(const CandleType &candle, PointType enter_value,
                                                       PointType stop_loss_value) const {
    return candle.is_exit_on_enter_short(enter_value, stop_loss_value);
}

bool LimitStrategy::is_enter_short_before_stop_loss_long(const CandleType &candle, PointType enter_value,
                                                        PointType stop_loss_value) const {
    return candle.is_enter_short_before_stop_loss_long(enter_value, stop_loss_value);
}

PointType LimitStrategy::stop_loss_long(PointType enter) const {
    return enter - stop_loss_calculator.calculate(enter);
}

PointType LimitStrategy::stop_loss_short(PointType enter) const {
    return enter + stop_loss_calculator.calculate(enter);
}

PointType LimitStrategy::gap(PointType price) const {
    return gap_calculator.calculate(price);
}

bool LimitStrategy::is_exit_by_stop_loss_long(const CandleType &candle, PointType loss_threshold) const {
    return candle.low <= loss_threshold;
}

bool LimitStrategy::is_exit_by_stop_loss_short(const CandleType &candle, PointType loss_threshold) const {
    return candle.high >= loss_threshold;
}

void LimitStrategy::enter_long(Time time, PointType price) {
    if (deal_tracker.is_in_deal()) {
        deal_tracker.escape_deal_by_limit(time, price);
    }
    deal_tracker.enter_long_by_limit(time, price);
}

void LimitStrategy::enter_short(Time time, PointType price) {
    if (deal_tracker.is_in_deal()) {
        deal_tracker.escape_deal_by_limit(time, price);
    }
    deal_tracker.enter_short_by_limit(time, price);
}

void LimitStrategy::escape_deal_by_stop_loss(Time time, PointType price) {
    deal_tracker.escape_deal_by_market(time, price);
}

PointType LimitStrategy::move_stop_loss_long(const CandleType &candle, PointType current_stop_loss) {
    // DEFAULT STOP LOSS MANAGER
    return current_stop_loss;
}

PointType LimitStrategy::move_stop_loss_short(const CandleType &candle, PointType current_stop_loss) {
    // DEFAULT STOP LOSS MANAGER
    return current_stop_loss;
}

bool LimitStrategy::should_move_gap_long(PointType old_value, PointType new_value) const {
    return true;
}

bool LimitStrategy::should_move_gap_short(PointType old_value, PointType new_value) const {
    return true;
}
