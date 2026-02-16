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
  float peakCh1mA_ = 0.0f;
  float peakCh2mA_ = 0.0f;
  float peakCh3mA_ = 0.0f;
};

#endif
