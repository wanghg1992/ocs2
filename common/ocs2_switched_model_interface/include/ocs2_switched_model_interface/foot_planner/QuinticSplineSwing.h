//
// Created by rgrandia on 21.01.21.
//

#pragma once

#include "ocs2_switched_model_interface/core/SwitchedModel.h"

namespace switched_model {

struct SwingNode {
  scalar_t time;
  scalar_t position;
  scalar_t velocity;
};

/**
 * Implements a quintic polynomial:
 *  y = c_5 * tau^5 + ... + c_1 * tau + c_0,
 *  where tau = (t - t0) / dt
 */
class QuinticSpline {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  /** Default constructor creates a flat spline, y(t) = 0.0 */
  QuinticSpline() : c_(vector6_t::Zero()), t0_(0.0), dt_(0.0) {}

  /**
   * Constructs a quintic spline. See class description for the definition.
   *
   * @param coefficients : spline coefficients [c_5, ..., c_0]
   * @param t0 : start time of the spline.
   * @param dt : time normalization factor. The normalized time is 1.0 at t = t0 + dt
   */
  QuinticSpline(const vector6_t& coefficients, scalar_t t0, scalar_t dt) : c_(coefficients), t0_(t0), dt_(dt) {}

  /** returns y(t) */
  scalar_t position(scalar_t time) const;

  /** returns dy/dt(t) */
  scalar_t velocity(scalar_t time) const;

  /** returns d2y/dt2(t) */
  scalar_t acceleration(scalar_t time) const;

  /** returns d3y/dt3(t) */
  scalar_t jerk(scalar_t time) const;

 private:
  scalar_t normalizedTime(scalar_t t) const;

  /// Coefficients [c_5, ..., c_0] for normalized time
  vector6_t c_;

  /// Start time of the spline
  scalar_t t0_;

  /// Spline time normalization
  scalar_t dt_;
};

/**
 * Implements a swing trajectory based on two quintic splines
 * The trajectory is made with the following 12 conditions
 *  start:
 *      position = given
 *      velocity = given
 *      acceleration = 0.0
 *  Mid:
 *      position = given & continuous
 *      velocity = given & continuous
 *      acceleration = continuous
 *      jerk = continuous
 *  End:
 *      position = given
 *      velocity = given
 *      acceleration = 0.0
 */
class QuinticSwing {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  /**
   * Construct a swing trajectory with given boundary conditions
   *
   * @param start : starting time and boundary conditions
   * @param midHeight : desired height at the middle of the trajectory. Assumes zero velocity.
   * @param end : ending time and boundary conditions
   */
  QuinticSwing(const SwingNode& start, scalar_t midHeight, const SwingNode& end);

  /**
   * Construct a swing trajectory with given boundary conditions
   *
   * @param start : starting time and boundary conditions
   * @param mid : waypoint time, position, and velocity. Time must be between start and end.
   * @param end : ending time and boundary conditions
   */
  QuinticSwing(const SwingNode& start, const SwingNode& mid, const SwingNode& end);

  /** returns z(t) */
  scalar_t position(scalar_t time) const;

  /** returns dz/dt(t) */
  scalar_t velocity(scalar_t time) const;

  /** returns d2z/dt2(t) */
  scalar_t acceleration(scalar_t time) const;

  /** returns d3z/dt3(t) */
  scalar_t jerk(scalar_t time) const;

 private:
  scalar_t midTime_;
  QuinticSpline leftSpline_;
  QuinticSpline rightSpline_;
};

}  // namespace switched_model