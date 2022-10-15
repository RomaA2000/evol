//
// Created by Роман Агеев on 15.10.2022.
//

#ifndef EVOL_STATES_HPP
#define EVOL_STATES_HPP

#include <cstddef>
#include <array>
#include <iostream>
#include <optional>
#include "candle.hpp"

class MarketStrategy;

class MarketState {
 public:
  explicit MarketState(MarketStrategy &strategy);

  MarketState *activate(const PointType &prev_close_price);

  void exit_by_stop_loss(const CandleType &candle, Action action);

  bool check_enter_on_next_candle() const;

  virtual void process(const CandleType &candle, Action action) = 0;

  virtual ~MarketState() = default;

  PointType stop_loss_threshold = 0.0;

  std::optional<PointType> close_price_before_enter = {};

  MarketStrategy &_strategy;
};

class ShortState : public MarketState {
 public:

  explicit ShortState(MarketStrategy &strategy);

  void process(const CandleType &candle, Action action) override;

  bool is_exit_by_stop_loss(const CandleType &candle) const;

 private:
  void check_enter(const CandleType &candle);
};

class LongState : public MarketState {
 public:

  explicit LongState(MarketStrategy &strategy);

  void process(const CandleType &candle, Action action) override;

  bool is_exit_by_stop_loss(const CandleType &candle) const;

 private:
  void check_enter(const CandleType &candle);
};

class StartState : public MarketState {
 public:

  void process(const CandleType &candle, Action action) override;

  explicit StartState(MarketStrategy &strategy);
};

#endif //EVOL_STATES_HPP