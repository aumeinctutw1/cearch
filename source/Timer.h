#ifndef _H_TIMER
#define _H_TIMER

#include <boost/asio.hpp>

#include "Index.h"

class Timer {
   public:
    Timer(int tick, boost::asio::io_context &io_context, Index &idx)
        : m_interval(std::chrono::seconds(tick)),
          m_timer(io_context, m_interval),
          m_idx(idx) {
        start_timer();
    };

   private:
    std::chrono::seconds m_interval;
    boost::asio::steady_timer m_timer;
    Index &m_idx;

    void start_timer() {
        m_timer.expires_from_now(m_interval);
        m_timer.async_wait([this](const boost::system::error_code &error) {
            this->async_handler(error);
        });
    }

    void async_handler(const boost::system::error_code &error) {
        if (!error) {
            std::cout << "Timer expired!" << std::endl;
            m_idx.run_reindexing();
            /* reschedule the timer */
            start_timer();
        } else {
            std::cerr << "Error occured on timer: " << error.message() << std::endl;
        }
    }
};

#endif