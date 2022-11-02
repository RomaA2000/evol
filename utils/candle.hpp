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

using PointType = double;
using CandleType = FloatCandle<1>;

#endif //EVOL_CANDLE_HPP
