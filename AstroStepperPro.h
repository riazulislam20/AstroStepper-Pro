#ifndef ASTRO_STEPPER_PRO_H
#define ASTRO_STEPPER_PRO_H

#include <Arduino.h>

class AstroStepper
{
public:
    enum MotorInterfaceType
    {
        DRIVER = 1
    };

    AstroStepper(uint8_t interface,
                 uint8_t stepPin,
                 uint8_t dirPin,
                 int8_t enablePin = -1,
                 bool enableActiveLow = true);

    void moveTo(long absolute);
    void move(long relative);

    bool run();        // trapezoidal slewing
    bool runSpeed();   // DDS tracking / jog

    void setMaxSpeed(float speed);
    void setAcceleration(float acceleration);
    void setSpeed(float speed);

    long distanceToGo();
    long currentPosition();
    void setCurrentPosition(long position);

    void stop();
    bool isRunning();

private:
    static void IRAM_ATTR onTimer();
    static hw_timer_t* _sharedTimer;
    static bool _timerStarted;

    static AstroStepper* instances[4];
    static uint8_t instanceCount;

    uint8_t _instanceId;

    uint8_t _stepPin;
    uint8_t _dirPin;

    int8_t _enablePin;
    bool _enableActiveLow;

    volatile int64_t _currentPos;
    volatile int64_t _targetPos;

    volatile bool _dir;
    volatile bool _running;

    // ===== DDS CORE =====
    volatile double _phase;           // 0..1
    volatile double _phaseIncrement;  // steps per timer tick
    volatile double _errorShaper;     // sigma-delta noise shaping

    volatile bool _pulseHigh;
    volatile uint32_t _pulseCounter;

    // ===== Slew planner =====
    float _maxSpeed;
    float _acceleration;
    float _currentSpeed;

    uint32_t _lastPlannerMicros;
    uint32_t _lastDebugMillis;

    void enableDriver();
    void disableDriver();
};

#endif
