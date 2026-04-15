// =============================================================================
// pid.cpp - PID with conditional integration anti-windup
// =============================================================================
#include "pid.h"

PID::PID(float kp, float ki, float kd, float out_min, float out_max)
    : kp_(kp), ki_(ki), kd_(kd),
      out_min_(out_min), out_max_(out_max),
      integ_(0.0f), prev_err_(0.0f), have_prev_(false) {}

void PID::set_gains(float kp, float ki, float kd) {
    kp_ = kp; ki_ = ki; kd_ = kd;
}

void PID::reset() {
    integ_     = 0.0f;
    prev_err_  = 0.0f;
    have_prev_ = false;
}

float PID::update(float setpoint, float measurement, float dt) {
    if (dt <= 0.0f) return 0.0f;

    float err    = setpoint - measurement;
    float deriv  = have_prev_ ? (err - prev_err_) / dt : 0.0f;
    prev_err_    = err;
    have_prev_   = true;

    // Tentative output with current integrator
    float p = kp_ * err;
    float i = ki_ * (integ_ + err * dt);
    float d = kd_ * deriv;
    float u = p + i + d;

    // Anti-windup: only integrate if we're not already saturated
    // in the direction the integrator would push us.
    bool saturating_hi = (u > out_max_) && (err > 0.0f);
    bool saturating_lo = (u < out_min_) && (err < 0.0f);
    if (!saturating_hi && !saturating_lo) {
        integ_ += err * dt;
    }

    // Clamp output
    if (u > out_max_) u = out_max_;
    if (u < out_min_) u = out_min_;
    return u;
}
