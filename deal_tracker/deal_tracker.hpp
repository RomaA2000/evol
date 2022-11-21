//
// Created by Роман Агеев on 15.10.2022.
//

#ifndef EVOL_DEAL_TRACKER_HPP
#define EVOL_DEAL_TRACKER_HPP

#include "../utils/candle.hpp"

constexpr PointType MAKER_COMISSION = 0.000120;
constexpr PointType TAKER_COMISSION = 0.000300;

enum class DealType {
  LONG,
  SHORT,
  NONE,
};

struct Deal {
  DealType type = DealType::NONE;
  PointType enter_value = 0;
};

class DealTracker {
 public:
  DealTracker() = default;

  bool is_in_deal() const;

  Deal escape_deal(Time time, PointType price, PointType commission);
  void enter_long(Time time, PointType price, PointType commission);
  void enter_short(Time time, PointType price, PointType commission);

  PointType sum = 0;
  Deal current_deal = {};

 private:
  void check_not_in_deal() const;
  void change_sum(Time time, Deal deal, PointType price);
 protected:
  void reset();
};

class StatisticsDealTracker : public DealTracker {
 public:
  uint64_t count = 0;

  uint64_t positive_count = 0;

  uint64_t long_positive_count = 0;

  uint64_t short_positive_count = 0;

  StatisticsDealTracker() = default;

  Deal escape_deal(Time time, PointType price, PointType commission);
  void enter_long(Time time, PointType price, PointType commission);
  void enter_short(Time time, PointType price, PointType commission);

 private:
  void update_stats(PointType delta, DealType old_deal);
 protected:
  void reset();
};

class BinanceDealTracker : public StatisticsDealTracker {
 public:
  BinanceDealTracker() = default;

  void escape_deal_by_market(Time time, PointType price);

  void enter_long_by_market(Time time, PointType price);

  void enter_short_by_market(Time time, PointType price);

  void reset();

  void escape_deal_by_limit(Time time, PointType price);

  void enter_long_by_limit(Time time, PointType price);

  void enter_short_by_limit(Time time, PointType price);
};

#endif //EVOL_DEAL_TRACKER_HPP
