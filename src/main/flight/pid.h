/*
 * This file is part of Cleanflight and Betaflight and EmuFlight.
 *
 * Cleanflight and Betaflight and EmuFlight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight and EmuFlight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdbool.h>
#include "common/time.h"
#include "common/filter.h"
#include "common/axis.h"
#include "common/dynlpf2.h"

#include "flight/gyroanalyse.h"

#include "pg/pg.h"

#define MAX_PID_PROCESS_DENOM       16
#define PID_CONTROLLER_BETAFLIGHT   1
#define PID_MIXER_SCALING           1000.0f
#define PID_SERVO_MIXER_SCALING     0.7f
#define PIDSUM_LIMIT                500
#define PIDSUM_LIMIT_YAW            400
#define PIDSUM_LIMIT_MIN            100
#define PIDSUM_LIMIT_MAX            1000

// Scaling factors for Pids for better tunable range in configurator for betaflight pid controller. The scaling is based on legacy pid controller or previous float
#define PTERM_SCALE 0.032029f
#define ITERM_SCALE 0.244381f
#define DTERM_SCALE 0.000529f

// The constant scale factor to replace the Kd component of the feedforward calculation.
// This value gives the same "feel" as the previous Kd default of 26 (26 * DTERM_SCALE)
#define FEEDFORWARD_SCALE 0.013754f

// Full iterm suppression in setpoint mode at high-passed setpoint rate > 40deg/sec
#define ITERM_RELAX_SETPOINT_THRESHOLD 40.0f
#define ITERM_RELAX_CUTOFF_DEFAULT 15

// Anti gravity I constant
#define AG_KI 21.586988f;

#define ITERM_ACCELERATOR_GAIN_OFF 0
#define ITERM_ACCELERATOR_GAIN_MAX 30000
typedef enum {
    PID_ROLL,
    PID_PITCH,
    PID_YAW,
    PID_LEVEL_LOW, //pid controller for low errorAngle
    PID_LEVEL_HIGH, //pid controller for high errorAngle
    PID_MAG,
    PID_ITEM_COUNT
} pidIndex_e;

typedef enum {
    SUPEREXPO_YAW_OFF = 0,
    SUPEREXPO_YAW_ON,
    SUPEREXPO_YAW_ALWAYS
} pidSuperExpoYaw_e;

typedef enum {
    PID_STABILISATION_OFF = 0,
    PID_STABILISATION_ON
} pidStabilisationState_e;

typedef enum {
    PID_CRASH_RECOVERY_OFF = 0,
    PID_CRASH_RECOVERY_DISARM,
} pidCrashRecovery_e;

typedef struct pidf_s {
    uint8_t P;
    uint8_t I;
    uint8_t D;
    uint16_t F;
} pidf_t;

typedef enum {
    ANTI_GRAVITY_SMOOTH,
    ANTI_GRAVITY_STEP
} antiGravityMode_e;

typedef enum {
    ITERM_RELAX_OFF,
    ITERM_RELAX_RP,
    ITERM_RELAX_RPY,
    ITERM_RELAX_RP_INC,
    ITERM_RELAX_RPY_INC,
    ITERM_RELAX_COUNT,
} itermRelax_e;

typedef enum {
    ITERM_RELAX_GYRO,
    ITERM_RELAX_SETPOINT,
    ITERM_RELAX_TYPE_COUNT,
} itermRelaxType_e;

typedef enum ffInterpolationType_e {
    FF_INTERPOLATE_OFF,
    FF_INTERPOLATE_ON,
    FF_INTERPOLATE_AVG2,
    FF_INTERPOLATE_AVG3,
    FF_INTERPOLATE_AVG4
} ffInterpolationType_t;

#define MAX_PROFILE_NAME_LENGTH 8u

typedef struct pidProfile_s {
    uint16_t yaw_lowpass_hz;                // Additional yaw filter when yaw axis too noisy
    uint16_t dterm_lowpass_hz;              // Delta Filter in hz
    uint16_t dterm_notch_hz;                // Biquad dterm notch hz
    uint16_t dterm_notch_cutoff;            // Biquad dterm notch low cutoff

    pidf_t  pid[PID_ITEM_COUNT];

    uint8_t dterm_filter_type;              // Filter selection for dterm
    uint8_t itermWindupPointPercent;        // iterm windup threshold, percent motor saturation
    uint16_t pidSumLimit;
    uint16_t pidSumLimitYaw;
    uint8_t pidAtMinThrottle;               // Disable/Enable pids on zero throttle. Normally even without airmode P and D would be active.
    uint8_t levelAngleLimit;                // Max angle in degrees in level mode

    uint8_t angleExpo;                      // How much expo to add to angle mode
    uint8_t horizonTransition;              // horizonTransition
    uint8_t horizonGain;                    // gain for horizon
    uint8_t racemode_tilt_effect;           // inclination factor for Horizon mode
    uint8_t racemode_horizon;               // OFF or ON

    // EmuFlight PID controller parameters
    uint8_t  antiGravityMode;               // type of anti gravity method
    uint16_t itermThrottleThreshold;        // max allowed throttle delta before iterm accelerated in ms
    uint16_t itermAcceleratorGain;          // Iterm Accelerator Gain when itermThrottlethreshold is hit
    uint16_t yawRateAccelLimit;             // yaw accel limiter for deg/sec/ms
    uint16_t rateAccelLimit;                // accel limiter roll/pitch deg/sec/ms
    uint16_t crash_dthreshold;              // dterm crash value
    uint16_t crash_gthreshold;              // gyro crash value
    uint16_t crash_setpoint_threshold;      // setpoint must be below this value to detect crash, so flips and rolls are not interpreted as crashes
    uint8_t crash_recovery;                 // off, disarm, only works in gps mode
    uint8_t feedForwardTransition;          // Feed forward weight transition
    uint16_t itermLimit;
    uint16_t dterm_lowpass2_hz;             // Extra PT1 Filter on D in hz
    uint8_t throttle_boost;                 // how much should throttle be boosted during transient changes 0-100, 100 adds 10x hpf filtered throttle
    uint8_t throttle_boost_cutoff;          // Which cutoff frequency to use for throttle boost. higher cutoffs keep the boost on for shorter. Specified in hz.
    uint8_t iterm_rotation;                 // rotates iterm to translate world errors to local coordinate system
    uint8_t iterm_relax_type;               // Specifies type of relax algorithm
    uint8_t iterm_relax_cutoff;             // This cutoff frequency specifies a low pass filter which predicts average response of the quad to setpoint
    uint8_t iterm_relax;                    // Enable iterm suppression during stick input
    uint8_t dterm_filter2_type;             // Filter selection for 2nd dterm
    uint16_t dyn_lpf_dterm_min_hz;
    uint16_t dyn_lpf_dterm_max_hz;
    uint8_t launchControlMode;              // Whether launch control is limited to pitch only (launch stand or top-mount) or all axes (on battery)
    uint8_t launchControlThrottlePercent;   // Throttle percentage to trigger launch for launch control
    uint8_t launchControlAngleLimit;        // Optional launch control angle limit (requires ACC)
    uint8_t launchControlGain;              // Iterm gain used while launch control is active
    uint8_t launchControlAllowTriggerReset; // Controls trigger behavior and whether the trigger can be reset
    uint8_t thrustLinearization;            // Compensation factor for pid linearization
    uint8_t d_min[XYZ_AXIS_COUNT];          // Minimum D value on each axis
    uint8_t d_min_gain;                     // Gain factor for amount of gyro / setpoint activity required to boost D
    uint8_t d_min_advance;                  // Percentage multiplier for setpoint input to boost algorithm
    uint8_t motor_output_limit;             // Upper limit of the motor output (percent)
    int8_t auto_profile_cell_count;         // Cell count for this profile to be used with if auto PID profile switching is used
    uint8_t ff_boost;                       // amount of high-pass filtered FF to add to FF, 100 means 100% added
    char profileName[MAX_PROFILE_NAME_LENGTH + 1]; // Descriptive name for profile

    uint8_t idle_min_rpm;                   // minimum motor speed enforced by integrating p controller
    uint8_t idle_adjustment_speed;          // how quickly the integrating p controller tries to correct
    uint8_t idle_p;                         // kP
    uint8_t idle_pid_limit;                 // max P
    uint8_t idle_max_increase;              // max integrated correction

    uint8_t ff_interpolate_sp;              // Calculate FF from interpolated setpoint
    uint8_t ff_max_rate_limit;              // Maximum setpoint rate percentage for FF
    uint8_t ff_smooth_factor;               // Amount of smoothing for interpolated FF steps
    uint8_t dyn_lpf_curve_expo;             // set the curve for dynamic dterm lowpass filter
    uint8_t nfe_racemode;                // NFE race mode - when true pitch setpoint calcualtion is gyro based in level mode
    uint8_t vbat_sag_compensation;          // Reduce motor output by this percentage of the maximum compensation amount

    uint16_t dtermDynNotchQ;                // Q value for the dynamic dterm notch
    uint16_t dterm_dyn_notch_min_hz;        // min hz for the dynamic dterm notch
    uint16_t dterm_dyn_notch_max_hz;        // max hz for the dynamic dterm notch
    uint8_t dterm_dyn_notch_location;       // location of the dyn dterm notch

    uint16_t dterm_dynlpf2_fmin;
    uint16_t dterm_dynlpf2_fmax;
    uint16_t dterm_dynlpf2_gain;
    uint16_t dterm_dynlpf2_fc_fc;
    uint16_t dterm_dynlpf2_center_threshold;
    uint16_t dterm_dynlpf2_throttle_threshold;
    uint16_t dterm_dynlpf2_throttle_gain;
    uint8_t  dterm_dynlpf2_enable;
    uint8_t  dterm_dynlpf2_type;
    uint8_t  dterm_dynlpf2_debug;

    uint8_t  dtermMeasurementSlider;

    uint16_t emuBoostPR;
    uint16_t emuBoostY;
    uint8_t emuBoostLimitPR;
    uint8_t emuBoostLimitY;
    uint16_t dtermBoost;
    uint8_t dtermBoostLimit;

    uint8_t i_decay;
    uint8_t i_decay_cutoff;
} pidProfile_t;

PG_DECLARE_ARRAY(pidProfile_t, PID_PROFILE_COUNT, pidProfiles);

typedef struct pidConfig_s {
    uint8_t pid_process_denom;              // Processing denominator for PID controller vs gyro sampling rate
    uint8_t runaway_takeoff_prevention;          // off, on - enables pidsum runaway disarm logic
    uint16_t runaway_takeoff_deactivate_delay;   // delay in ms for "in-flight" conditions before deactivation (successful flight)
    uint8_t runaway_takeoff_deactivate_throttle; // minimum throttle percent required during deactivation phase
} pidConfig_t;

PG_DECLARE(pidConfig_t, pidConfig);

union rollAndPitchTrims_u;
void pidController(const pidProfile_t *pidProfile);

typedef struct pidAxisData_s {
    float P;
    float I;
    float D;
    float F;

    float Sum;
} pidAxisData_t;

typedef union dtermLowpass_u {
    pt1Filter_t pt1Filter;
    biquadFilter_t biquadFilter;
} dtermLowpass_t;

typedef struct pidCoefficient_s {
    float Kp;
    float Ki;
    float Kd;
    float Kf;
} pidCoefficient_t;

typedef struct pidRuntime_s {
    float dT;
    float pidFrequency;
    bool pidStabilisationEnabled;
    float previousPidSetpoint[XYZ_AXIS_COUNT];
    filterApplyFnPtr dtermNotchApplyFn;
    biquadFilter_t dtermNotch[XYZ_AXIS_COUNT];
    filterApplyFnPtr dtermLowpassApplyFn;
    dtermLowpass_t dtermLowpass[XYZ_AXIS_COUNT];
    filterApplyFnPtr dtermLowpass2ApplyFn;
    dtermLowpass_t dtermLowpass2[XYZ_AXIS_COUNT];
    filterApplyFnPtr ptermYawLowpassApplyFn;
    pt1Filter_t ptermYawLowpass;
    filterApplyFnPtr dtermDynNotchApplyFn;
    biquadFilter_t dtermNotchFilterDyn[XYZ_AXIS_COUNT];
    fftAnalyseState_t dtermFFTAnalyseState;
    dynlpf2_t dynLpfDterm[XYZ_AXIS_COUNT];
    bool antiGravityEnabled;
    uint8_t antiGravityMode;
    pt1Filter_t antiGravityThrottleLpf;
    pt1Filter_t antiGravitySmoothLpf;
    float antiGravityOsdCutoff;
    float antiGravityThrottleHpf;
    float antiGravityPBoost;
    float ffBoostFactor;
    float itermAccelerator;
    uint16_t itermAcceleratorGain;
    float feedForwardTransition;
    pidCoefficient_t pidCoefficient[XYZ_AXIS_COUNT];

    float P_angle_low;
    float D_angle_low;
    float P_angle_high;
    float D_angle_high;
    float F_angle;
    float horizonGain;
    float horizonTransition;
    float horizonCutoffDegrees;
    float horizonFactorRatio;
    uint8_t horizonTiltExpertMode;
    float previousAngle[XYZ_AXIS_COUNT];
    float attitudePrevious[XYZ_AXIS_COUNT];

    float maxVelocity[XYZ_AXIS_COUNT];
    float itermWindupPointInv;
    float crashGyroThreshold;
    float crashDtermThreshold;
    float crashSetpointThreshold;
    float itermLimit;
    bool itermRotation;
    bool zeroThrottleItermReset;
    bool nfeRaceMode;
    float dtermMeasurementSlider;
    float dtermMeasurementSliderInverse;

    float emuBoostPR;
    float emuBoostY;
    float emuBoostLimitPR;
    float emuBoostLimitY;
    float dtermBoost;
    float dtermBoostLimit;

#ifdef USE_ITERM_RELAX
    pt1Filter_t windupLpf[XYZ_AXIS_COUNT];
    uint8_t itermRelax;
    uint8_t itermRelaxType;
    uint8_t itermRelaxCutoff;
#endif

#ifdef USE_D_MIN
    biquadFilter_t dMinRange[XYZ_AXIS_COUNT];
    pt1Filter_t dMinLowpass[XYZ_AXIS_COUNT];
    float dMinPercent[XYZ_AXIS_COUNT];
    float dMinGyroGain;
    float dMinSetpointGain;
#endif

#ifdef USE_RC_SMOOTHING_FILTER
    pt1Filter_t setpointDerivativePt1[XYZ_AXIS_COUNT];
    biquadFilter_t setpointDerivativeBiquad[XYZ_AXIS_COUNT];
    bool setpointDerivativeLpfInitialized;
    uint8_t rcSmoothingDebugAxis;
    uint8_t rcSmoothingFilterType;
#endif // USE_RC_SMOOTHING_FILTER

#ifdef USE_DYN_LPF
    uint8_t dynLpfFilter;
    uint16_t dynLpfMin;
    uint16_t dynLpfMax;
    uint8_t dynLpfCurveExpo;
#endif

#ifdef USE_LAUNCH_CONTROL
    uint8_t launchControlMode;
    uint8_t launchControlAngleLimit;
    float launchControlKi;
#endif

#ifdef USE_THRUST_LINEARIZATION
    float thrustLinearization;
    float throttleCompensateAmount;
#endif

#ifdef USE_INTERPOLATED_SP
    ffInterpolationType_t ffFromInterpolatedSetpoint;
    float ffSmoothFactor;
#endif
} pidRuntime_t;

extern pidRuntime_t pidRuntime;

extern const char pidNames[];

extern pidAxisData_t pidData[3];

extern uint32_t targetPidLooptime;

extern float throttleBoost;
extern pt1Filter_t throttleLpf;

void pidResetIterm(void);
void pidStabilisationState(pidStabilisationState_e pidControllerState);
void pidSetItermAccelerator(float newItermAccelerator);
void pidAcroTrainerInit(void);
void pidSetAcroTrainerState(bool newState);
void pidUpdateAntiGravityThrottleFilter(float throttle);
bool pidOsdAntiGravityActive(void);
bool pidOsdAntiGravityMode(void);
void pidSetAntiGravityState(bool newState);
bool pidAntiGravityEnabled(void);
#ifdef USE_THRUST_LINEARIZATION
float pidApplyThrustLinearization(float motorValue);
float pidCompensateThrustLinearization(float throttle);
#endif


#ifdef UNIT_TEST
#include "sensors/acceleration.h"
extern float axisError[XYZ_AXIS_COUNT];
void applyItermRelax(const int axis, const float iterm,
    const float gyroRate, float *itermErrorRate, float *currentPidSetpoint);
void applyAbsoluteControl(const int axis, const float gyroRate, float *currentPidSetpoint, float *itermErrorRate);
void rotateItermAndAxisError();
float pidLevel(int axis, const pidProfile_t *pidProfile,
    const rollAndPitchTrims_t *angleTrim, float currentPidSetpoint);
float calcHorizonLevelStrength(const pidProfile_t *pidProfile);
#endif
void dynLpfDTermUpdate(float throttle);
void pidSetItermReset(bool enabled);
float pidGetPreviousSetpoint(int axis);
float pidGetDT();
float pidGetPidFrequency();
float pidGetFfBoostFactor();
float pidGetFfSmoothFactor();
float dynLpfCutoffFreq(float throttle, uint16_t dynLpfMin, uint16_t dynLpfMax, uint8_t expo);
