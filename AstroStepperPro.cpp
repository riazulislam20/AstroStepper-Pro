#include "AstroStepperPro.h"
#include "driver/gpio.h"
#include <math.h>

AstroStepper* AstroStepper::instances[4] = {nullptr};
uint8_t AstroStepper::instanceCount = 0;

hw_timer_t* AstroStepper::_sharedTimer = nullptr;
bool AstroStepper::_timerStarted = false;

// 100kHz base frequency
static constexpr double TIMER_FREQ = 100000.0;

// ----------------------------------------------------
AstroStepper::AstroStepper(uint8_t interface,
                           uint8_t stepPin,
                           uint8_t dirPin,
                           int8_t enablePin,
                           bool enableActiveLow)
{
    _instanceId = instanceCount;
    instances[instanceCount++] = this;

    _stepPin = stepPin;
    _dirPin  = dirPin;
    _enablePin = enablePin;
    _enableActiveLow = enableActiveLow;

    pinMode(_stepPin, OUTPUT);
    pinMode(_dirPin, OUTPUT);

    if (_enablePin >= 0)
    {
        pinMode(_enablePin, OUTPUT);
        disableDriver();
    }

    _currentPos = 0;
    _targetPos  = 0;
    _running    = false;

    _pulseHigh   = false;
    _pulseCounter = 0;

    _phase = 0.0;
    _phaseIncrement = 0.0;
    _errorShaper = 0.0;

    _maxSpeed     = 1000.0f;
    _acceleration = 500.0f;
    _currentSpeed = 0.0f;

    _lastPlannerMicros = micros();
    _lastDebugMillis   = millis();

    if (!_timerStarted)
    {
        _sharedTimer = timerBegin(100000);   // 100kHz
        timerAttachInterrupt(_sharedTimer, &AstroStepper::onTimer);
        timerAlarm(_sharedTimer, 1, true, 0);
        _timerStarted = true;
    }
}

// ----------------------------------------------------
void AstroStepper::enableDriver()
{
    if (_enablePin < 0) return;
    digitalWrite(_enablePin, _enableActiveLow ? LOW : HIGH);
}

void AstroStepper::disableDriver()
{
    if (_enablePin < 0) return;
    digitalWrite(_enablePin, _enableActiveLow ? HIGH : LOW);
}

// ----------------------------------------------------
// ===== SHARED DDS TIMER ISR =====
// ----------------------------------------------------
void IRAM_ATTR AstroStepper::onTimer()
{
    for (uint8_t i = 0; i < instanceCount; i++)
    {
        AstroStepper* s = instances[i];
        if (!s || !s->_running) continue;

        // DDS phase accumulation
        double shapedIncrement = s->_phaseIncrement + s->_errorShaper;
        s->_phase += shapedIncrement;

        if (s->_phase >= 1.0)
        {
            s->_phase -= 1.0;

            // first order noise shaping
            s->_errorShaper = shapedIncrement - 1.0;

            gpio_set_level((gpio_num_t)s->_dirPin, s->_dir);
            gpio_set_level((gpio_num_t)s->_stepPin, 1);
            gpio_set_level((gpio_num_t)s->_stepPin, 0);

            if (s->_dir) s->_currentPos++;
            else         s->_currentPos--;

            s->_pulseCounter++;
        }
        else
        {
            s->_errorShaper = shapedIncrement;
        }
    }
}

// ----------------------------------------------------
// Slew (trapezoidal)
// ----------------------------------------------------
bool AstroStepper::run()
{
    if (!_running) return false;

    uint32_t now = micros();
    float dt = (now - _lastPlannerMicros) / 1000000.0f;
    _lastPlannerMicros = now;

    if (dt > 0.01f) dt = 0.01f;

    long distance = _targetPos - _currentPos;
    if (distance == 0)
    {
        stop();
        return false;
    }

    enableDriver();

    float direction = (distance > 0) ? 1.0f : -1.0f;

    float stoppingDistance =
        (_currentSpeed * _currentSpeed) / (2.0f * _acceleration);

    if (fabs(distance) <= stoppingDistance)
        _currentSpeed -= direction * _acceleration * dt;
    else
        _currentSpeed += direction * _acceleration * dt;

    if (fabs(_currentSpeed) > _maxSpeed)
        _currentSpeed = direction * _maxSpeed;

    _dir = (_currentSpeed > 0);

    _phaseIncrement = fabs(_currentSpeed) / TIMER_FREQ;

    if (millis() - _lastDebugMillis >= 1000)
    {
        Serial.printf("🔢 Pulses/sec (motor %d): %lu\n",
                      _instanceId,
                      _pulseCounter);
        _pulseCounter = 0;
        _lastDebugMillis = millis();
    }

    return true;
}

// ----------------------------------------------------
// Tracking / Jog
// ----------------------------------------------------
bool AstroStepper::runSpeed()
{
    if (!_running) return false;

    enableDriver();

    if (millis() - _lastDebugMillis >= 1000)
    {
        Serial.printf("🔢 Pulses/sec (motor %d): %lu\n",
                      _instanceId,
                      _pulseCounter);
        _pulseCounter = 0;
        _lastDebugMillis = millis();
    }

    return true;
}

// ----------------------------------------------------
void AstroStepper::setSpeed(float speed)
{
    _currentSpeed = constrain(speed, -_maxSpeed, _maxSpeed);

    if (_currentSpeed == 0)
    {
        stop();
        return;
    }

    enableDriver();
    _dir = (_currentSpeed > 0);

    _phaseIncrement = fabs(_currentSpeed) / TIMER_FREQ;
    _running = true;
}

// ----------------------------------------------------
void AstroStepper::moveTo(long absolute)
{
    _targetPos = absolute;
    _currentSpeed = 0;
    _phase = 0;
    _errorShaper = 0;
    _lastPlannerMicros = micros();

    if (_currentPos != _targetPos)
    {
        enableDriver();
        _running = true;
    }
    else
        _running = false;
}

// ----------------------------------------------------
void AstroStepper::move(long relative)
{
    moveTo(_currentPos + relative);
}

void AstroStepper::setMaxSpeed(float speed)
{
    _maxSpeed = fabs(speed);
}

void AstroStepper::setAcceleration(float acceleration)
{
    _acceleration = fabs(acceleration);
}

long AstroStepper::distanceToGo()
{
    return _targetPos - _currentPos;
}

long AstroStepper::currentPosition()
{
    return _currentPos;
}

void AstroStepper::setCurrentPosition(long position)
{
    _currentPos = position;
}

void AstroStepper::stop()
{
    _targetPos = _currentPos;
    _currentSpeed = 0;
    _phaseIncrement = 0;
    _running = false;
    disableDriver();
}

bool AstroStepper::isRunning()
{
    return _running;
}
