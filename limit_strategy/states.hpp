//
// Created by Роман Агеев on 20/11/22.
//

#ifndef EVOL_LIMIT_STATES_HPP
#define EVOL_LIMIT_STATES_HPP


#include "../deal_tracker/deal_tracker.hpp"
#include "../utils/candle.hpp"

namespace Limit {

    class LimitStrategy;

    class LimitState {
    public:
        explicit LimitState(LimitStrategy &strategy);

        virtual void process(const CandleType &candle, Action action) = 0;

    protected:
        virtual ~LimitState() = default;

        LimitStrategy &_strategy;
    };

    class InState : public LimitState {
    public:
        explicit InState(LimitStrategy &strategy);

        virtual bool is_exit_by_stop_loss(const CandleType &candle) const = 0;

        PointType loss_threshold = 0.0;
    };

    class InLongState : public InState {
    public:
        explicit InLongState(LimitStrategy &strategy);

        bool is_exit_by_stop_loss(const CandleType &candle) const override;
    };

    class InShortState : public InState {
    public:
        explicit InShortState(LimitStrategy &strategy);

        bool is_exit_by_stop_loss(const CandleType &candle) const override;
    };

    class LongState : public InLongState {
    public:
        explicit LongState(LimitStrategy &strategy);

        void process(const CandleType &candle, Action action) override;
    };

    class ShortState : public InShortState {
    public:
        explicit ShortState(LimitStrategy &strategy);

        void process(const CandleType &candle, Action action) override;
    };

    class LongWaitShortState : public InShortState {
    public:
        explicit LongWaitShortState(LimitStrategy &strategy);

        void process(const CandleType &candle, Action action) override;

        PointType bias = 0.0;

    private:
        void process_enter_short(const CandleType &candle, Action action);

        void process_stop_loss_cases(const CandleType &candle, Action action);

        bool is_stop_loss_on_enter(const CandleType &candle, PointType threshold) const;
    };

    class ShortWaitLongState : public InLongState {
    public:
        explicit ShortWaitLongState(LimitStrategy &strategy);

        void process(const CandleType &candle, Action action) override;

        PointType bias = 0.0;

    private:
        void process_enter_long(const CandleType &candle, Action action);

        void process_stop_loss_cases(const CandleType &candle, Action action);

        bool is_stop_loss_on_enter(const CandleType &candle, PointType threshold) const;
    };

    class StartState : public LimitState {
    public:
        explicit StartState(LimitStrategy &strategy);

        void process(const CandleType &candle, Action action) override;
    };

    class WaitLongState : public LimitState {
    public:
        explicit WaitLongState(LimitStrategy &strategy);

        void process(const CandleType &candle, Action action) override;

        PointType bias = 0.0;

    private:
        bool is_stop_loss_on_enter(const CandleType &candle, PointType threshold) const;
    };

    class WaitShortState : public LimitState {
    public:
        explicit WaitShortState(LimitStrategy &strategy);

        void process(const CandleType &candle, Action action) override;

        PointType bias = 0.0;

    private:
        bool is_stop_loss_on_enter(const CandleType &candle, PointType threshold) const;
    };
}

#endif //EVOL_LIMIT_STATES_HPP
