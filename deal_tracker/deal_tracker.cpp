//
// Created by Роман Агеев on 15.10.2022.
//
#include "deal_tracker.hpp"

bool DealTracker::is_in_deal() const {
  return current_deal.type != DealType::NONE;
}

Deal DealTracker::escape_deal(Time time, PointType price, PointType commission) {
  assert(is_in_deal());
  Deal deal = current_deal;

  change_sum(time, deal, price);
  current_deal = {};
  sum -= commission;

  return deal;
}

void DealTracker::enter_long(Time time, PointType price, PointType commission) {
  check_not_in_deal();

  current_deal = {
      .type = DealType::LONG,
      .enter_value = price
  };

  sum -= commission;
}

void DealTracker::enter_short(Time time, PointType price, PointType commission) {
  check_not_in_deal();

  current_deal = {
      .type = DealType::SHORT,
      .enter_value = price
  };

  sum -= commission;
}

void DealTracker::check_not_in_deal() const {
  assert(!is_in_deal());
}

void DealTracker::change_sum(Time time, Deal deal, PointType price) {
  assert(deal.type != DealType::NONE);
  switch (deal.type) {
    case DealType::LONG: sum += price - deal.enter_value;
      break;
    case DealType::SHORT: sum += deal.enter_value - price;
      break;
    case DealType::NONE: break;
  }
}

void DealTracker::reset() {
  sum = 0;
  current_deal = {};
}

Deal StatisticsDealTracker::escape_deal(Time time, PointType price, PointType commission) {
  ++count;
  PointType sum_before = sum;

  Deal old_deal = DealTracker::escape_deal(time, price, commission);

  PointType delta = sum - sum_before;

  update_stats(delta, old_deal.type);
  return old_deal;
}

void StatisticsDealTracker::enter_long(Time time, PointType price, PointType commission) {
  ++count;
  DealTracker::enter_long(time, price, commission);
}

void StatisticsDealTracker::enter_short(Time time, PointType price, PointType commission) {
  ++count;
  DealTracker::enter_short(time, price, commission);
}

void StatisticsDealTracker::update_stats(PointType delta, DealType old_deal) {
  if (delta > 0) {
    ++positive_count;
    switch (old_deal) {
      case DealType::LONG: ++long_positive_count;
        break;
      case DealType::SHORT: ++short_positive_count;
        break;
      case DealType::NONE: break;
    }
  }
}

void StatisticsDealTracker::reset() {
  count = 0;
  positive_count = 0;
  long_positive_count = 0;
  short_positive_count = 0;
  DealTracker::reset();
}

void BinanceDealTracker::escape_deal_by_market(Time time, PointType price) {
  StatisticsDealTracker::escape_deal(time, price, price * TAKER_COMISSION);
}

void BinanceDealTracker::enter_long_by_market(Time time, PointType price) {
  StatisticsDealTracker::enter_long(time, price, price * TAKER_COMISSION);
}

void BinanceDealTracker::enter_short_by_market(Time time, PointType price) {
  StatisticsDealTracker::enter_short(time, price, price * TAKER_COMISSION);
}

void BinanceDealTracker::reset() {
  StatisticsDealTracker::reset();
}

void BinanceDealTracker::escape_deal_by_limit(Time time, PointType price) {
    StatisticsDealTracker::escape_deal(time, price, price * MAKER_COMISSION);
}

void BinanceDealTracker::enter_long_by_limit(Time time, PointType price) {
    StatisticsDealTracker::enter_long(time, price, price * MAKER_COMISSION);
}

void BinanceDealTracker::enter_short_by_limit(Time time, PointType price) {
    StatisticsDealTracker::enter_short(time, price, price * MAKER_COMISSION);
}
