#include <AstroStepperPro.h>

// -----------------------
// Pin Configuration
// -----------------------
#define RA_STEP_PIN  2
#define RA_DIR_PIN   3
#define RA_EN_PIN    4

#define DEC_STEP_PIN 5
#define DEC_DIR_PIN  6
#define DEC_EN_PIN   7

// -----------------------
// Create Motors
// -----------------------
AstroStepper stepperRA(
    AstroStepper::DRIVER,
    RA_STEP_PIN,
    RA_DIR_PIN,
    RA_EN_PIN,
    true   // enable active LOW
);

AstroStepper stepperDEC(
    AstroStepper::DRIVER,
    DEC_STEP_PIN,
    DEC_DIR_PIN,
    DEC_EN_PIN,
    true
);

// -----------------------
// Mount Parameters
// -----------------------
// Example:
// 200 steps motor
// 128 microstep
// 40:1 gearbox
// RA worm 144:1
//
// steps per RA revolution:
//
const double STEPS_PER_REV =
    200.0 * 128.0 * 40.0 * 144.0;

// Sidereal rate (1 rev per 86164 sec)
const double SIDEREAL_STEPS_PER_SEC =
    STEPS_PER_REV / 86164.0905;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("AstroStepper Observatory Tracking Demo");

    // -----------------------
    // Configure RA
    // -----------------------
    stepperRA.setMaxSpeed(50000);        // high ceiling
    stepperRA.setAcceleration(20000);    // smooth slew accel

    // Start sidereal tracking
    stepperRA.setSpeed(SIDEREAL_STEPS_PER_SEC);

    // -----------------------
    // Configure DEC
    // -----------------------
    stepperDEC.setMaxSpeed(20000);
    stepperDEC.setAcceleration(10000);
}

void loop()
{
    // -----------------------
    // Tracking
    // -----------------------
    stepperRA.runSpeed();  // DDS tracking

    // -----------------------
    // Example Slew
    // -----------------------
    static bool slewed = false;

    if (!slewed && millis() > 5000)
    {
        Serial.println("Slewing DEC 5000 steps...");
        stepperDEC.move(5000);
        slewed = true;
    }

    stepperDEC.run();  // trapezoidal slew
}
