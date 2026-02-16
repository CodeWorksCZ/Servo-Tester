#ifndef INA_MONITOR_H
#define INA_MONITOR_H

#include <Arduino.h>
#include <Wire.h>

class InaMonitor
{
public:
  // Initialize INA3221 and channel shunt configuration.
  void begin(TwoWire &wire);
  // Periodic non-blocking refresh from sensor.
  void update(unsigned long nowMs);

  // Runtime status/readouts in milliamps.
  bool ready() const;
  float ch1mA() const;
  float ch2mA() const;
  float ch3mA() const;
  // Bus voltage per channel in volts.
  float busCh1V() const;
  float busCh2V() const;
  float busCh3V() const;
  // Voltage droop (reference minus current bus voltage) in volts.
  float droopCh1V() const;
  float droopCh2V() const;
  float droopCh3V() const;
  // Raw INA3221 mask/enable flags.
  uint16_t flags() const;
  // Channel warning/critical alert states.
  bool warnCh1() const;
  bool warnCh2() const;
  bool warnCh3() const;
  bool critCh1() const;
  bool critCh2() const;
  bool critCh3() const;
  // Peak hold values in milliamps since boot/reset.
  float peakCh1mA() const;
  float peakCh2mA() const;
  float peakCh3mA() const;

private:
  bool ready_ = false;
  unsigned long lastUpdateMs_ = 0;
  float ch1mA_ = 0.0f;
  float ch2mA_ = 0.0f;
  float ch3mA_ = 0.0f;
  float busCh1V_ = 0.0f;
  float busCh2V_ = 0.0f;
  float busCh3V_ = 0.0f;
  float busRefCh1V_ = 0.0f;
  float busRefCh2V_ = 0.0f;
  float busRefCh3V_ = 0.0f;
  uint16_t flags_ = 0;
  bool warnCh1_ = false;
  bool warnCh2_ = false;
  bool warnCh3_ = false;
  bool critCh1_ = false;
  bool critCh2_ = false;
  bool critCh3_ = false;
  float peakCh1mA_ = 0.0f;
  float peakCh2mA_ = 0.0f;
  float peakCh3mA_ = 0.0f;
};

#endif
