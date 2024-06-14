#define STIMWALKER_API_EXPORTS
#include "Utils/Timer.h"

using namespace STIMWALKER_NAMESPACE::utils;

Timer::Timer(bool startNow) : m_isStarted(false),
                              m_isPaused(false),
                              m_start(),
                              m_pauseTime(),
                              m_totalPauseTime(0.0)
{
    if (startNow)
    {
        start();
    }
}

void Timer::start()
{
    m_start = std::clock();
    m_totalPauseTime = 0;
    m_isPaused = false;
    m_isStarted = true;
} // Start a timer

bool Timer::isStarted()
{
    return m_isStarted;
}

void Timer::pause()
{
    if (!m_isPaused)
    {
        m_isPaused = true;
        m_pauseTime = std::clock();
    }
}

void Timer::resume()
{
    if (!m_isStarted)
    {
        start();
    }

    else if (m_isPaused)
    {
        addPauseTime();
        m_isPaused = false;
    }
}

double Timer::getLap()
{
    addPauseTime();

    if (m_isStarted)
    {
        return getTime(m_start) - m_totalPauseTime;
    }
    else
    {
        return 0;
    }
}

double Timer::stop()
{
    if (m_isStarted)
    {
        m_isStarted = false;
        return getLap();
    }
    else
    {
        return 0;
    }
}

void Timer::addPauseTime()
{
    if (m_isPaused)
    {
        m_totalPauseTime += getTime(m_pauseTime);
        m_pauseTime = std::clock();
    }
}

double Timer::getTime(const std::clock_t &timer)
{
    return static_cast<double>(std::clock() - timer) / CLOCKS_PER_SEC;
}
