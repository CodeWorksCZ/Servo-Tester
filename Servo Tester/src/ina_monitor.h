#ifndef INA_MONITOR_H
#define INA_MONITOR_H

#include <Arduino.h>
#include <Wire.h>

class InaMonitor
{
public:
  void begin(TwoWire &wire);
  void update(unsigned long nowMs);

  bool ready() const;
  float ch1mA() const;
  float ch2mA() const;
  float ch3mA() const;
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
