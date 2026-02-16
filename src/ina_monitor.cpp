#include "ina_monitor.h"

#include <Adafruit_INA3221.h>

#include "config.h"

namespace
{
// Single INA3221 driver instance.
Adafruit_INA3221 g_inaDevice;
} // namespace

void InaMonitor::begin(TwoWire &wire)
{
  // Try sensor at configured I2C address.
  ready_ = g_inaDevice.begin(Config::INA3221_I2C_ADDRESS, &wire);
  if (ready_)
  {
    // Per-channel shunt values are needed for current conversion.
    g_inaDevice.setShuntResistance(0, Config::INA3221_SHUNT_OHMS_CH1);
    g_inaDevice.setShuntResistance(1, Config::INA3221_SHUNT_OHMS_CH2);
    g_inaDevice.setShuntResistance(2, Config::INA3221_SHUNT_OHMS_CH3);
  }
}

void InaMonitor::update(const unsigned long nowMs)
{
  if (!ready_)
  {
    // Keep previous values when sensor is unavailable.
    return;
  }

  if ((nowMs - lastUpdateMs_) < Config::INA3221_REFRESH_MS)
  {
    // Respect configured refresh period to reduce I2C traffic.
    return;
  }

  lastUpdateMs_ = nowMs;

  ch1mA_ = g_inaDevice.getCurrentAmps(0) * 1000.0f * Config::INA3221_CAL_FACTOR_CH1;
  ch2mA_ = g_inaDevice.getCurrentAmps(1) * 1000.0f * Config::INA3221_CAL_FACTOR_CH2;
  ch3mA_ = g_inaDevice.getCurrentAmps(2) * 1000.0f * Config::INA3221_CAL_FACTOR_CH3;

  // Peak-hold logic used by the PEAK LCD screen.
  if (ch1mA_ > peakCh1mA_)
  {
    peakCh1mA_ = ch1mA_;
  }
  if (ch2mA_ > peakCh2mA_)
  {
    peakCh2mA_ = ch2mA_;
  }
  if (ch3mA_ > peakCh3mA_)
  {
    peakCh3mA_ = ch3mA_;
  }
}

bool InaMonitor::ready() const
{
  return ready_;
}

float InaMonitor::ch1mA() const
{
  return ch1mA_;
}

float InaMonitor::ch2mA() const
{
  return ch2mA_;
}

float InaMonitor::ch3mA() const
{
  return ch3mA_;
}

float InaMonitor::peakCh1mA() const
{
  return peakCh1mA_;
}

float InaMonitor::peakCh2mA() const
{
  return peakCh2mA_;
}

float InaMonitor::peakCh3mA() const
{
  return peakCh3mA_;
}
