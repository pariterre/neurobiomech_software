#define STIMWALKER_API_EXPORTS
#include "Utils/Benchmark.h"

#include "Utils/Timer.h"
#include "Utils/String.h"

using namespace STIMWALKER_NAMESPACE::utils;

Benchmark::Benchmark() : m_timers(std::map<String, Timer>()),
                         m_counts(std::map<String, int>())
{
}

void Benchmark::startTimer(
    const String &name,
    bool force)
{
    if (force)
    {
        m_timers[name].start();
    }
    else if (!m_timers[name].isStarted())
    {
        m_timers[name].start();
    }
}

double Benchmark::getLap(
    const String &name)
{
    return m_timers[name].getLap();
}

double Benchmark::stopTimer(
    const String &name)
{
    return m_timers[name].stop();
}

void Benchmark::pauseTimer(
    const String &name)
{
    m_timers[name].pause();
}

void Benchmark::resumeTimer(
    const String &name)
{
    m_timers[name].resume();
}

void Benchmark::addTimer(
    const String &name)
{
    ++(m_counts[name]);
}

int Benchmark::getTimerIdx(
    const String &name)
{
    return m_counts[name];
}

void Benchmark::wasteTime(
    double seconds)
{
    // Wait for seconds ask doing dummy stuff

    std::clock_t start(std::clock());

    while (((static_cast<double>(std::clock() - start)) / CLOCKS_PER_SEC) < seconds)
    {
    }
}
