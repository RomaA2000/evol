//
// Created by Роман Агеев on 15.10.2022.
//

#include <cassert>
#include "states.hpp"
#include "strategy.hpp"

using namespace Market;

MarketState::MarketState(MarketStrategy &strategy) : _strategy(strategy) {}

MarketState *MarketState::activate(const PointType &prev_close_price) {
    close_price_before_enter = prev_close_price;
    return this;
}

void MarketState::exit_by_stop_loss(const CandleType &candle, Action action) {
    _strategy.escape_deal_by_stop_loss(candle.time_stamp, stop_loss_threshold);

    switch (action) {
        case Action::NONE:
            _strategy.to_start();
            break;
        case Action::SHORT:
            _strategy.to_short(candle.close);
            break;
        case Action::LONG:
            _strategy.to_long(candle.close);
            break;
    }
}

bool MarketState::check_enter_on_next_candle() const {
    return close_price_before_enter.has_value();
}

ShortState::ShortState(MarketStrategy &strategy) : MarketState(strategy) {}

void ShortState::process(const CandleType &candle, Action action) {
    check_enter(candle);

    if (is_exit_by_stop_loss(candle)) {
        exit_by_stop_loss(candle, action);
        return;
    }

    switch (action) {
        case Action::NONE:
        case Action::SHORT:
            stop_loss_threshold = _strategy.move_stop_loss_short(candle, stop_loss_threshold);
            break;
        case Action::LONG:
            _strategy.to_long(candle.close);
            break;
    }
}

bool ShortState::is_exit_by_stop_loss(const CandleType &candle) const {
    return candle.high >= stop_loss_threshold;
}

void ShortState::check_enter(const CandleType &candle) {
    if (check_enter_on_next_candle()) {
        assert(close_price_before_enter.has_value());
        stop_loss_threshold = *close_price_before_enter + _strategy.get_stop_loss(*close_price_before_enter);
        _strategy.enter_short_by_market(candle.time_stamp, candle.open);
        close_price_before_enter = {};
    }
}

LongState::LongState(MarketStrategy &strategy) : MarketState(strategy) {}

void LongState::process(const CandleType &candle, Action action) {
    check_enter(candle);

    if (is_exit_by_stop_loss(candle)) {
        exit_by_stop_loss(candle, action);
        return;
    }

    switch (action) {
        case Action::NONE:
        case Action::LONG:
            stop_loss_threshold = _strategy.move_stop_loss_long(candle, stop_loss_threshold);
            break;
        case Action::SHORT:
            _strategy.to_short(candle.close);
            break;
    }
}

bool LongState::is_exit_by_stop_loss(const CandleType &candle) const {
    return candle.low <= stop_loss_threshold;
}

void LongState::check_enter(const CandleType &candle) {
    if (check_enter_on_next_candle()) {
        assert(close_price_before_enter.has_value());
        stop_loss_threshold = *close_price_before_enter - _strategy.get_stop_loss(*close_price_before_enter);
        _strategy.enter_long_by_market(candle.time_stamp, candle.open);
        close_price_before_enter = {};
    }
}

void StartState::process(const CandleType &candle, Action action) {
    switch (action) {
        case Action::LONG:
            _strategy.to_long(candle.close);
            break;
        case Action::SHORT:
            _strategy.to_short(candle.close);
            break;
        case Action::NONE:
            break;
    }
}

StartState::StartState(MarketStrategy &strategy) : MarketState(strategy) {}
