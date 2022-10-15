//
// Created by Роман Агеев on 15.10.2022.
//
#include "strategy.hpp"

ParameterCalculator::ParameterCalculator(PointType value, ParameterCalculationStrategy strategy)
    : _value(value), _strategy(strategy) {}

PointType ParameterCalculator::calculate(PointType price) const {
  switch (_strategy) {
    case ParameterCalculationStrategy::RELATIVE: return price * _value;
    case ParameterCalculationStrategy::ABSOLUTE: return _value;
  }
}

MarketStrategy::MarketStrategy(ParameterCalculator stop_loss_calculator)
    : long_state(*this),
      short_state(*this),
      start_state(*this),
      deal_tracker(),
      current_state(&start_state),
      stop_loss_calculator(stop_loss_calculator) {}

void MarketStrategy::process(const CandleType &candle, Action action) {
  current_state->process(candle, action);
}

void MarketStrategy::exit(const CandleType &last_candle) {
  if (deal_tracker.is_in_deal()) {
    deal_tracker.escape_deal_by_market(last_candle.time_stamp, last_candle.close);
  }
}

void MarketStrategy::reset() {
  to_start();
}

void MarketStrategy::to_start() {
  current_state = &start_state;
}

void MarketStrategy::to_short(PointType prev_close_price) {
  current_state = short_state.activate(prev_close_price);
}

void MarketStrategy::to_long(PointType prev_close_price) {
  current_state = long_state.activate(prev_close_price);
}

/**
 * Входит в сделку Long. Выходит из предыдущей, если такая была
*/
void MarketStrategy::enter_long_by_market(Time time, PointType price) {
  if (deal_tracker.is_in_deal()) {
    deal_tracker.escape_deal_by_market(time, price);
  }

  deal_tracker.enter_long_by_market(time, price);
}

/**
* Входит в сделку Short. Выходит из предыдущей, если такая была
*/
void MarketStrategy::enter_short_by_market(Time time, PointType price) {
  if (deal_tracker.is_in_deal()) {
    deal_tracker.escape_deal_by_market(time, price);
  }

  deal_tracker.enter_short_by_market(time, price);
}

void MarketStrategy::escape_deal_by_stop_loss(Time time, PointType price) {
  deal_tracker.escape_deal_by_market(time, price);
}

PointType MarketStrategy::move_stop_loss_long(const CandleType &candle, PointType old_value) const {
  // DEFAULT STOP LOSS MANAGER
  return old_value;
}

PointType MarketStrategy::move_stop_loss_short(const CandleType &candle, PointType old_value) const {
  // DEFAULT STOP LOSS MANAGER
  return old_value;
}

PointType MarketStrategy::get_stop_loss(PointType price) {
  return stop_loss_calculator.calculate(price);
}
Stats MarketStrategy::stats() const {
  return {
      .deal_count = deal_tracker.count,
      .sum = deal_tracker.sum
  };
}

void MarketStrategy::reset_stats() {
  deal_tracker.reset();
}
