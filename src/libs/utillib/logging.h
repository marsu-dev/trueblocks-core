#pragma once
/*-------------------------------------------------------------------------------------------
 * qblocks - fast, easily-accessible, fully-decentralized data from blockchains
 * copyright (c) 2018, 2019 TrueBlocks, LLC (http://trueblocks.io)
 *
 * This program is free software: you may redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version. This program is
 * distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details. You should have received a copy of the GNU General
 * Public License along with this program. If not, see http://www.gnu.org/licenses/.
 *-------------------------------------------------------------------------------------------*/
#include "basetypes.h"
#include "colors.h"
#include "performance.h"

namespace qblocks {

//----------------------------------------------------------------
typedef enum {
    sev_na = 0,
    sev_info = 1,
    sev_warning,
    sev_error,
    sev_fatal,
    sev_debug0 = 10,
    sev_debug1,
    sev_debug2,
    sev_debug3,
    sev_debug4,
    sev_debug8 = 18,
} severity_t;

//----------------------------------------------------------------
extern uint64_t verbose;
inline bool isLevelOn(severity_t test) {
    if (test < sev_debug0)
        return true;
    return (((severity_t)verbose) >= (test - sev_debug0));
}

//----------------------------------------------------------------
class log_policy_i {
  public:
    char end_line;
    log_policy_i(void) : end_line('\n') {
    }
    virtual void open_ostream(const string_q& name) = 0;
    virtual void close_ostream() = 0;
    virtual void write(const string_q& msg) = 0;
    virtual ~log_policy_i(void) {
    }
};

//----------------------------------------------------------------
class file_log : public log_policy_i {
    unique_ptr<ofstream> os;

  public:
    file_log() : os(new ofstream) {
    }
    void open_ostream(const string_q& name) {
        os->open(name.c_str(), ios_base::binary | ios_base::out);
        if (!os->is_open())
            throw(runtime_error("LOGGER: Unable to open an output stream"));
    }
    void close_ostream() {
        if (os)
            os->close();
    }
    void write(const string_q& msg) {
        if (os) {
            (*os) << msg << end_line;
            os->flush();
        }
    }
    ~file_log() {
        close_ostream();
    }
};

//----------------------------------------------------------------
class err_log : public log_policy_i {
  public:
    err_log() {
    }
    void open_ostream(const string_q& unused) {
    }
    void close_ostream() {
    }
    void write(const string_q& msg) {
        cerr << msg << end_line;
        cerr.flush();
    }
    ~err_log() {
    }
};

//----------------------------------------------------------------
template <typename log_policy>
class logger {
    //----------------------------------------------------------------
    stringstream log_stream;
    log_policy* policy;
    mutex write_mutex;

    //----------------------------------------------------------------
    string_q get_logline_header(void) {
        //           static unsigned log_line_number = 0;

        stringstream header;
        //            header.fill('0');
        //            header.width(7);
        //            header << bBlack << ++log_line_number << " ";
        if (isTestMode()) {
            header << "TIME ~ CLOCK - ";
        } else {
//                header << Now().Format(FMT_EXPORT) << " ~ ";
//                header << TIC() << " ~ ";
#define LOG_TIMING true
#define LOG_THREAD false
            if (LOG_TIMING) {
                static clock_t last_clock = 0;
                header.fill('0');
                header.width(7);
                clock_t now = clock();
                // FIX_THIS_CODE
                // header << now << " (" << padNum7T(uint64_t(now - last_clock)) << ")- ";
                last_clock = now;
            }
            //            if (LOG_THREAD) {
            //                header.fill('0');
            //                header.width(7);
            //                header << this_thread::get_id() << " + ";
            //            }
        }
        header << cOff;
        return header.str();
    }

    //----------------------------------------------------------------
    void print_impl() {
        if (!log_stream.str().empty() && log_stream.str()[log_stream.str().length() - 1] == '\r')
            setEndline('\r');
        policy->write(get_logline_header() + log_stream.str());
        log_stream.str("");
        setEndline('\n');
    }

    //----------------------------------------------------------------
    template <typename First, typename... Rest>
    void print_impl(First parm1, Rest... parm) {
        log_stream << parm1;
        print_impl(parm...);
    }

  public:
    //----------------------------------------------------------------
    void setEndline(char ch) {
        if (policy)
            policy->end_line = ch;
    }

    //----------------------------------------------------------------
    explicit logger(const string_q& name) {
        policy = new log_policy;
        if (!policy) {
            throw std::runtime_error("LOGGER: Unable to create the logger instance");
        }
        if (!name.empty())
            policy->open_ostream(name);
    }

    //----------------------------------------------------------------
    template <severity_t severity, typename... Args>
    void print(Args... args) {
        if (!isLevelOn(severity))
            return;
        write_mutex.lock();
        switch (severity) {
            case sev_debug0:
                log_stream << ": " << cWhite << "0" << cOff << "-";
                break;
            case sev_debug1:
                log_stream << ": " << cWhite << "1" << cOff << "--";
                break;
            case sev_debug2:
                log_stream << ": " << cGreen << "2" << cOff << "---";
                break;
            case sev_debug3:
                log_stream << ": " << cYellow << "3" << cOff << "----";
                break;
            case sev_debug4:
                log_stream << ": " << cRed << "4" << cOff << "-----";
                break;
            case sev_debug8:
                log_stream << ": " << cTeal << "8" << cOff << "------";
                break;
            case sev_info:
                log_stream << bGreen << "<INFO>  " << cOff << ": ";
                break;
            case sev_warning:
                log_stream << bYellow << "<WARNG> " << cOff << ": ";
                break;
            case sev_error:
                log_stream << bRed << "<ERROR> " << cOff << ": ";
                break;
            case sev_fatal:
                log_stream << bTeal << "<FATAL> " << cOff << ": ";
                break;
        }
        print_impl(args...);
        write_mutex.unlock();
    }

    //----------------------------------------------------------------
    virtual ~logger() {
        if (policy) {
            policy->close_ostream();
            // delete policy;
        }
    }
};

extern logger<log_policy_i>* dLogger;
extern logger<log_policy_i>* eLogger;
}  // namespace qblocks

#define LOGGING_LEVEL
#ifdef LOGGING_LEVEL
#define LOG0 qblocks::dLogger->print<sev_debug0>
#define LOG1 qblocks::dLogger->print<sev_debug1>
#define LOG2 qblocks::dLogger->print<sev_debug2>
#define LOG3 qblocks::dLogger->print<sev_debug3>
#define LOG4 qblocks::dLogger->print<sev_debug4>
#define LOG8 qblocks::dLogger->print<sev_debug8>
#define LOG_INFO qblocks::eLogger->print<sev_info>
#define LOG_WARN qblocks::eLogger->print<sev_warning>
#define LOG_ERR qblocks::eLogger->print<sev_error>
#define LOG_FATAL qblocks::eLogger->print<sev_fatal>
#define SEP1(a) LOG1(cWhite + string_q(10, '-') + (a) + string_q(10, '-') + cOff)
#define SEP2(a) LOG2(cGreen + string_q(10, '-') + (a) + string_q(10, '-') + cOff)
#define SEP3(a) LOG3(cYellow + string_q(10, '-') + (a) + string_q(10, '-') + cOff)
#define SEP4(a) LOG4(cRed + string_q(10, '-') + (a) + string_q(10, '-') + cOff)
#define SEP8(a) LOG8(cTeal + string_q(10, '-') + (a) + string_q(10, '-') + cOff)
#define LOG_PROGRESS1(op, progress, goal, post) LOG_INFO((op), " ", (progress), " of ", (goal), (post))
#define LOG_PROGRESS(op, progress, goal) LOG_PROGRESS1((op), (progress), (goal), "\n")
#else
#define LOG0(...)
#define LOG1(...)
#define LOG2(...)
#define LOG3(...)
#define LOG4(...)
#define LOG8(...)
#define LOG_INFO(...)
#define LOG_WARN(...)
#define LOG_ERR(...)
#define LOG_FATAL(...)
#define SEP1(...)
#define SEP2(...)
#define SEP3(...)
#define SEP4(...)
#define SEP5(...)
#define LOG_PROGRESS(...)
#define LOG_PROGRESS1(...)
#endif

// The LOG parts of these routines disappear if turned off, but they still do their work because of the returns
namespace qblocks {
extern string_q _logEnter(const string_q& func);
extern string_q _logExit(const string_q& func);
extern bool silenceEnter;
extern bool silenceExit;
}  // namespace qblocks

#define ENTER(a)                                                                                                       \
    {                                                                                                                  \
        if (!silenceEnter)                                                                                             \
            LOG4(_logEnter(a));                                                                                        \
    }                                                                                                                  \
    string_q l_funcName = (a);

#define EXIT_USAGE(a)                                                                                                  \
    {                                                                                                                  \
        LOG_ERR(_logExit(l_funcName));                                                                                 \
        return usage((a));                                                                                             \
    }

#define EXIT_FAIL(a)                                                                                                   \
    {                                                                                                                  \
        LOG_WARN(_logExit(l_funcName));                                                                                \
        cerr << (a);                                                                                                   \
        return false;                                                                                                  \
    }

#define EXIT_MSG(a, b)                                                                                                 \
    {                                                                                                                  \
        if (!silenceExit)                                                                                              \
            LOG4(_logExit(l_funcName));                                                                                \
        cerr << (a);                                                                                                   \
        return (b);                                                                                                    \
    }

#define EXIT_NOMSG(b)                                                                                                  \
    {                                                                                                                  \
        if (!silenceExit)                                                                                              \
            LOG4(_logExit(l_funcName));                                                                                \
        return (b);                                                                                                    \
    }

#define EXIT_VOID()                                                                                                    \
    {                                                                                                                  \
        if (!silenceExit)                                                                                              \
            LOG4(_logExit(l_funcName));                                                                                \
        return;                                                                                                        \
    }

#define ENTER8(a)                                                                                                      \
    {                                                                                                                  \
        if (!silenceEnter)                                                                                             \
            LOG8(_logEnter(a));                                                                                        \
    }                                                                                                                  \
    string_q l_funcName = (a);

#define EXIT_MSG8(a, b)                                                                                                \
    {                                                                                                                  \
        if (!silenceExit)                                                                                              \
            LOG8(_logExit(l_funcName));                                                                                \
        cerr << a;                                                                                                     \
        return (b);                                                                                                    \
    }

#define EXIT_NOMSG8(b)                                                                                                 \
    {                                                                                                                  \
        if (!silenceExit)                                                                                              \
            LOG8(_logExit(l_funcName));                                                                                \
        return (b);                                                                                                    \
    }
