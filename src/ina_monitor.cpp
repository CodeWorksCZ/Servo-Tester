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
    // Balance responsiveness and noise for servo current monitoring.
    g_inaDevice.setAveragingMode(INA3221_AVG_16_SAMPLES);
    g_inaDevice.setBusVoltageConvTime(INA3221_CONVTIME_1MS);
    g_inaDevice.setShuntVoltageConvTime(INA3221_CONVTIME_1MS);
    // Configure hardware warning/critical thresholds.
    g_inaDevice.setWarningAlertThreshold(0, Config::INA3221_WARN_MA_CH1 / 1000.0f);
    g_inaDevice.setWarningAlertThreshold(1, Config::INA3221_WARN_MA_CH2 / 1000.0f);
    g_inaDevice.setWarningAlertThreshold(2, Config::INA3221_WARN_MA_CH3 / 1000.0f);
    g_inaDevice.setCriticalAlertThreshold(0, Config::INA3221_CRIT_MA_CH1 / 1000.0f);
    g_inaDevice.setCriticalAlertThreshold(1, Config::INA3221_CRIT_MA_CH2 / 1000.0f);
    g_inaDevice.setCriticalAlertThreshold(2, Config::INA3221_CRIT_MA_CH3 / 1000.0f);
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
  busCh1V_ = g_inaDevice.getBusVoltage(0);
  busCh2V_ = g_inaDevice.getBusVoltage(1);
  busCh3V_ = g_inaDevice.getBusVoltage(2);

  // Track best (highest) observed bus voltage as a simple no-load reference.
  if (busCh1V_ > busRefCh1V_)
  {
    busRefCh1V_ = busCh1V_;
  }
  if (busCh2V_ > busRefCh2V_)
  {
    busRefCh2V_ = busCh2V_;
  }
  if (busCh3V_ > busRefCh3V_)
  {
    busRefCh3V_ = busCh3V_;
  }

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

  flags_ = g_inaDevice.getFlags();
  const bool ch1Valid = busCh1V_ >= Config::INA3221_ALERT_MIN_BUS_V;
  const bool ch2Valid = busCh2V_ >= Config::INA3221_ALERT_MIN_BUS_V;
  const bool ch3Valid = busCh3V_ >= Config::INA3221_ALERT_MIN_BUS_V;

  // Suppress false alerts on unpowered/floating channels.
  warnCh1_ = ch1Valid && ((flags_ & INA3221_WARN_CH1) != 0);
  warnCh2_ = ch2Valid && ((flags_ & INA3221_WARN_CH2) != 0);
  warnCh3_ = ch3Valid && ((flags_ & INA3221_WARN_CH3) != 0);
  critCh1_ = ch1Valid && ((flags_ & INA3221_CRITICAL_CH1) != 0);
  critCh2_ = ch2Valid && ((flags_ & INA3221_CRITICAL_CH2) != 0);
  critCh3_ = ch3Valid && ((flags_ & INA3221_CRITICAL_CH3) != 0);
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

float InaMonitor::busCh1V() const
{
  return busCh1V_;
}

float InaMonitor::busCh2V() const
{
  return busCh2V_;
}

float InaMonitor::busCh3V() const
{
  return busCh3V_;
}

float InaMonitor::droopCh1V() const
{
  const float droop = busRefCh1V_ - busCh1V_;
  return (droop > 0.0f) ? droop : 0.0f;
}

float InaMonitor::droopCh2V() const
{
  const float droop = busRefCh2V_ - busCh2V_;
  return (droop > 0.0f) ? droop : 0.0f;
}

float InaMonitor::droopCh3V() const
{
  const float droop = busRefCh3V_ - busCh3V_;
  return (droop > 0.0f) ? droop : 0.0f;
}

uint16_t InaMonitor::flags() const
{
  return flags_;
}

bool InaMonitor::warnCh1() const
{
  return warnCh1_;
}

bool InaMonitor::warnCh2() const
{
  return warnCh2_;
}

bool InaMonitor::warnCh3() const
{
  return warnCh3_;
}

bool InaMonitor::critCh1() const
{
  return critCh1_;
}

bool InaMonitor::critCh2() const
{
  return critCh2_;
}

bool InaMonitor::critCh3() const
{
  return critCh3_;
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
