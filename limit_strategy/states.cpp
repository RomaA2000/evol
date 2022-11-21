//
// Created by Роман Агеев on 20/11/22.
//

#include "states.hpp"
#include "strategy.hpp"

using namespace Limit;

LimitState::LimitState(LimitStrategy &strategy) : _strategy(strategy) {}

InState::InState(LimitStrategy &strategy) : LimitState(strategy) {}

InLongState::InLongState(LimitStrategy &strategy) : InState(strategy) {}

bool InLongState::is_exit_by_stop_loss(const CandleType &candle) const {
    return candle.low <= loss_threshold;
}

InShortState::InShortState(LimitStrategy &strategy) : InState(strategy) {}

bool InShortState::is_exit_by_stop_loss(const CandleType &candle) const {
    return candle.low >= loss_threshold;
}

LongState::LongState(LimitStrategy &strategy) : InLongState(strategy) {}

void LongState::process(const CandleType &candle, Action action) {
    if (is_exit_by_stop_loss(candle)) {
        _strategy.escape_deal_by_stop_loss(candle.time_stamp, loss_threshold);

        switch (action) {
            case Action::NONE:
                _strategy.to_start();
                break;
            case Action::SHORT:
                _strategy.to_wait_short(candle.close + _strategy.gap(candle.close));
                break;
            case Action::LONG:
                _strategy.to_wait_long(candle.close - _strategy.gap(candle.close));
                break;
        }

        loss_threshold = _strategy.move_stop_loss_long(candle, loss_threshold);

        if (action == Action::SHORT) {
            _strategy.to_long_wait_short(candle.close + _strategy.gap(candle.close), loss_threshold);
        }
    }
}

ShortState::ShortState(LimitStrategy &strategy) : InShortState(strategy) {}

void ShortState::process(const CandleType &candle, Action action) {
    if (is_exit_by_stop_loss(candle)) {
        _strategy.escape_deal_by_stop_loss(candle.time_stamp, loss_threshold);

        switch (action) {
            case Action::NONE:
                _strategy.to_start();
                break;
            case Action::SHORT:
                _strategy.to_wait_short(candle.close + _strategy.gap(candle.close));
                break;
            case Action::LONG:
                _strategy.to_wait_long(candle.close - _strategy.gap(candle.close));
                break;
        }

        loss_threshold = _strategy.move_stop_loss_short(candle, loss_threshold);

        if (action == Action::LONG) {
            _strategy.to_short_wait_long(candle.close - _strategy.gap(candle.close), loss_threshold);
        }
    }
}

LongWaitShortState::LongWaitShortState(LimitStrategy &strategy) : InShortState(strategy) {}

void LongWaitShortState::process(const CandleType &candle, Action action) {
    if (is_exit_by_stop_loss(candle)) {
        process_stop_loss_cases(candle, action);
        return;
    }

    if (candle.high >= bias) {
        process_enter_short(candle, action);
        return;
    }

    loss_threshold = _strategy.move_stop_loss_long(candle, loss_threshold);

    if (action == Action::LONG) {
        _strategy.to_long(loss_threshold);
        return;
    }

    if (action == Action::SHORT) {
        PointType new_bias = candle.close + _strategy.gap(candle.close);
        if (_strategy.should_move_gap_short(bias, new_bias)) {
            bias = new_bias;
        }
    }
}

void LongWaitShortState::process_enter_short(const CandleType &candle, Action action) {
    _strategy.enter_short(candle.time_stamp, bias);

    PointType threshold = _strategy.stop_loss_short(bias);

    if (is_stop_loss_on_enter(candle, threshold)) {
        _strategy.escape_deal_by_stop_loss(candle.time_stamp, threshold);

        switch (action) {
            case Action::NONE:
                _strategy.to_start();
                break;
            case Action::LONG:
                _strategy.to_wait_long(candle.close - _strategy.gap(candle.close));
                break;
            case Action::SHORT:
                _strategy.to_wait_short(candle.close + _strategy.gap(candle.close));
                break;
        }
        return;
    }

    PointType updated_threshold = _strategy.move_stop_loss_short(candle, threshold);

    if (action == Action::SHORT || action == Action::NONE) {
        _strategy.to_short(updated_threshold);
    } else {
        _strategy.to_short_wait_long(candle.close - _strategy.gap(candle.close), updated_threshold);
    }
}

void LongWaitShortState::process_stop_loss_cases(const CandleType &candle, Action action) {
    if (candle.high >= bias
        && bias >= loss_threshold
        && _strategy.is_enter_short_before_stop_loss_long(candle, bias, loss_threshold)) {
        process_enter_short(candle, action);
        return;
    }

    _strategy.escape_deal_by_stop_loss(candle.time_stamp, loss_threshold);

    if (candle.high >= bias) {
        process_enter_short(candle, action);
        return;
    }

    PointType new_bias;
    PointType next_state_bias;

    switch (action) {

        case Action::NONE:
            _strategy.to_wait_short(bias);
            break;
        case Action::SHORT:
            new_bias = candle.close + _strategy.gap(candle.close);
            next_state_bias = _strategy.should_move_gap_short(bias, new_bias) ? new_bias : bias;
            _strategy.to_wait_short(next_state_bias);
            break;
        case Action::LONG:
            _strategy.to_wait_long(candle.close - _strategy.gap(candle.close));
            break;
    }
}

bool LongWaitShortState::is_stop_loss_on_enter(const CandleType &candle, PointType threshold) const {
    return _strategy.is_exit_by_stop_loss_short(candle, threshold) && _strategy.is_exit_by_stop_loss_on_enter_short(candle, bias, threshold);
}

ShortWaitLongState::ShortWaitLongState(LimitStrategy &strategy) : InLongState(strategy) {}

void ShortWaitLongState::process(const CandleType &candle, Action action) {
if (is_exit_by_stop_loss(candle)) {
        process_stop_loss_cases(candle, action);
        return;
    }

    if (candle.low <= bias) {
        process_enter_long(candle, action);
        return;
    }

    loss_threshold = _strategy.move_stop_loss_short(candle, loss_threshold);

    if (action == Action::SHORT) {
        _strategy.to_short(loss_threshold);
        return;
    }

    if (action == Action::LONG) {
        PointType new_bias = candle.close - _strategy.gap(candle.close);
        if (_strategy.should_move_gap_long(bias, new_bias)) {
            bias = new_bias;
        }
    }
}

void ShortWaitLongState::process_enter_long(const CandleType &candle, Action action) {
    _strategy.enter_long(candle.time_stamp, bias);

    PointType threshold = _strategy.stop_loss_long(bias);

    if (is_stop_loss_on_enter(candle, threshold)) {
        _strategy.escape_deal_by_stop_loss(candle.time_stamp, threshold);

        switch (action) {
            case Action::NONE:
                _strategy.to_start();
                break;
            case Action::SHORT:
                _strategy.to_wait_short(candle.close + _strategy.gap(candle.close));
                break;
            case Action::LONG:
                _strategy.to_wait_long(candle.close - _strategy.gap(candle.close));
                break;
        }
        return;
    }

    PointType updated_threshold = _strategy.move_stop_loss_long(candle, threshold);

    if (action == Action::LONG || action == Action::NONE) {
        _strategy.to_long(updated_threshold);
    } else {
        _strategy.to_long_wait_short(candle.close + _strategy.gap(candle.close), updated_threshold);
    }
}

void ShortWaitLongState::process_stop_loss_cases(const CandleType &candle, Action action) {
    if (candle.low <= bias
        && bias <= loss_threshold
        && _strategy.is_enter_long_before_stop_loss_short(candle, bias, loss_threshold)) {
        process_enter_long(candle, action);
        return;
    }

    _strategy.escape_deal_by_stop_loss(candle.time_stamp, loss_threshold);

    if (candle.low <= bias) {
        process_enter_long(candle, action);
        return;
    }

    PointType new_bias;
    PointType next_state_bias;

    switch (action) {

        case Action::NONE:
            _strategy.to_wait_long(bias);
            break;
        case Action::LONG:
            new_bias = candle.close - _strategy.gap(candle.close);
            next_state_bias = _strategy.should_move_gap_long(bias, new_bias) ? new_bias : bias;
            _strategy.to_wait_long(next_state_bias);
            break;
        case Action::SHORT:
            _strategy.to_wait_short(candle.close + _strategy.gap(candle.close));
            break;
    }
}

bool ShortWaitLongState::is_stop_loss_on_enter(const CandleType &candle, PointType threshold) const {
    return _strategy.is_exit_by_stop_loss_long(candle, threshold) && _strategy.is_exit_by_stop_loss_on_enter_long(candle, bias, threshold);
}

StartState::StartState(LimitStrategy &strategy) : LimitState(strategy) {}

void StartState::process(const CandleType &candle, Action action) {
    switch (action) {
        case Action::NONE:
            break;
        case Action::SHORT:
            _strategy.to_wait_short(candle.close + _strategy.gap(candle.close));
            break;
        case Action::LONG:
            _strategy.to_wait_long(candle.close - _strategy.gap(candle.close));
            break;
    }
}

WaitLongState::WaitLongState(LimitStrategy &strategy) : LimitState(strategy) {}

void WaitLongState::process(const CandleType &candle, Action action) {
    if (candle.low <= bias) {
        _strategy.enter_long(candle.time_stamp, bias);

        PointType threshold = _strategy.stop_loss_long(bias);

        if (is_stop_loss_on_enter(candle, threshold)) {
            _strategy.escape_deal_by_stop_loss(candle.time_stamp, threshold);

            switch (action) {
                case Action::NONE:
                    _strategy.to_start();
                    break;
                case Action::SHORT:
                    _strategy.to_wait_short(candle.close + _strategy.gap(candle.close));
                    break;
                case Action::LONG:
                    _strategy.to_wait_long(candle.close - _strategy.gap(candle.close));
                    break;
            }
            return;
        }

        PointType updated_threshold = _strategy.move_stop_loss_long(candle, threshold);

        if (action == Action::LONG || action == Action::NONE) {
            _strategy.to_long(updated_threshold);
        } else {
            _strategy.to_long_wait_short(candle.close + _strategy.gap(candle.close), updated_threshold);
        }
    }
}

bool WaitLongState::is_stop_loss_on_enter(const CandleType &candle, PointType threshold) const {
    return _strategy.is_exit_by_stop_loss_long(candle, threshold) && _strategy.is_exit_by_stop_loss_on_enter_long(candle, bias, threshold);
}

WaitShortState::WaitShortState(LimitStrategy &strategy) : LimitState(strategy) {}

void WaitShortState::process(const CandleType &candle, Action action) {
    if (candle.high >= bias) {
        _strategy.enter_short(candle.time_stamp, bias);

        PointType threshold = _strategy.stop_loss_short(bias);

        if (is_stop_loss_on_enter(candle, threshold)) {
            _strategy.escape_deal_by_stop_loss(candle.time_stamp, threshold);

            switch (action) {
                case Action::NONE:
                    _strategy.to_start();
                    break;
                case Action::SHORT:
                    _strategy.to_wait_short(candle.close + _strategy.gap(candle.close));
                    break;
                case Action::LONG:
                    _strategy.to_wait_long(candle.close - _strategy.gap(candle.close));
                    break;
            }
            return;
        }

        PointType updated_threshold = _strategy.move_stop_loss_short(candle, threshold);

        if (action == Action::SHORT || action == Action::NONE) {
            _strategy.to_short(updated_threshold);
        } else {
            _strategy.to_short_wait_long(candle.close - _strategy.gap(candle.close), updated_threshold);
        }
    }
}

bool WaitShortState::is_stop_loss_on_enter(const CandleType &candle, PointType threshold) const {
    return _strategy.is_exit_by_stop_loss_short(candle, threshold) && _strategy.is_exit_by_stop_loss_on_enter_short(candle, bias, threshold);
}
