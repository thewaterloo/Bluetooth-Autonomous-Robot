// =============================================================================
// pid.h - small PID controller with anti-windup
// =============================================================================
#pragma once
#include <stdint.h>

class PID {
public:
    PID(float kp, float ki, float kd, float out_min, float out_max);

    void  set_gains(float kp, float ki, float kd);
    void  reset();
    // dt in seconds. Returns a control output clamped to [out_min, out_max].
    float update(float setpoint, float measurement, float dt);

private:
    float kp_, ki_, kd_;
    float out_min_, out_max_;
    float integ_;
    float prev_err_;
    bool  have_prev_;
};
