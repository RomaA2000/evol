//
// Created by Роман Агеев on 15.10.2022.
//

#ifndef EVOL_STRATEGY_HPP
#define EVOL_STRATEGY_HPP

#include <cstddef>
#include <array>
#include <iostream>
#include <optional>
#include "candle.hpp"
#include "states.hpp"
#include "deal_tracker/deal_tracker.hpp"

enum class ParameterCalculationStrategy {
  RELATIVE,
  ABSOLUTE
};

struct Stats {
  uint64_t deal_count;
  PointType sum;
};

class ParameterCalculator {
 public:

  ParameterCalculator(PointType value, ParameterCalculationStrategy strategy);

  PointType calculate(PointType price) const;

 private:
  PointType _value;
  ParameterCalculationStrategy _strategy;
};

class MarketStrategy {
 public:
  explicit MarketStrategy(ParameterCalculator parameter_calculator);

  void process(const CandleType &candle, Action action);

  void exit(const CandleType &last_candle);

  void reset();

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

  Stats stats() const;

  void reset_stats();

 private:
  LongState long_state;
  ShortState short_state;
  StartState start_state;

  BinanceDealTracker deal_tracker;

  MarketState *current_state;
  ParameterCalculator stop_loss_calculator;
};

#endif //EVOL_STRATEGY_HPP
