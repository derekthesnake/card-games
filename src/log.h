#include <chrono>
#include <iomanip>
#include <semaphore>
#include <source_location>
#include <string_view>

// TODO: possibly migrate to making an object file
// TODO: make timestamps print (well-formatted) at >= debug levels

/*
  Short note: ALWAYS pass logging::endl to the logging stream when done.
  Otherwise the application will deadlock next time you log something.
 */

namespace logging {
  
  enum loglevel_t {
    SILENT = 0,
    STANDARD = 1,
    INFO = 5,
    DEBUG = 10,
  };

  std::binary_semaphore log_mutex{1};

  loglevel_t log_level = STANDARD;

  // see https://codereview.stackexchange.com/a/260936
  struct BlackholeBuf final : std::streambuf {
    char_type target[64];
    std::streamsize xsputn(const char_type*, std::streamsize n) { return n; }
    int_type overflow(int_type c = traits_type::eof()) {
      return c;
      //        setp(target, std::end(target));
      //        return 0;
    }
  };

  static BlackholeBuf _blackhole_buf;
  static std::ostream blackhole = std::ostream(&_blackhole_buf);

  std::ostream& endl(std::ostream& str) {
    str << std::endl;
    log_mutex.release();
    return str;
  }
  
  std::ostream& timestamp(std::ostream& str) {
    std::time_t now = std::time(nullptr);
    const std::chrono::time_point<std::chrono::system_clock> baseline = std::chrono::system_clock::now();
    const auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(baseline).time_since_epoch().count();
    const auto secs = std::chrono::floor<std::chrono::seconds>(baseline).time_since_epoch().count();
    const auto mins = std::chrono::floor<std::chrono::minutes>(baseline).time_since_epoch().count();
    const auto hours = std::chrono::floor<std::chrono::hours>(baseline).time_since_epoch().count();
    const std::chrono::time_point now2{std::chrono::system_clock::now()};
    const std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(now2)};
    
    str << "|" << static_cast<int>(ymd.year()) << "-" << static_cast<unsigned>(ymd.month()) << "-" << static_cast<unsigned>(ymd.day()) << " " << std::setw(2) << std::setfill('0') << hours % 24 << ":" << std::setw(2) << std::setfill('0') << mins % 60 << ":" << secs % 60 << "." << std::setfill('0') << std::setw(3) << ms % 1000;
    return str;
  }
  
  std::ostream& debug(const std::source_location location = std::source_location::current()) {
    if (log_level >= DEBUG) {
      log_mutex.acquire();
      return std::clog << "DEBUG:"
		       << timestamp << "|"
		       << location.file_name() << ':'
		       << location.line() << ": ";
      // lock is not released until logging::endl is called
    }
    return blackhole;
  }
  
  std::ostream& info(const std::source_location location = std::source_location::current()) {
    if (log_level >= INFO) {
      log_mutex.acquire();
      std::clog << "INFO:";
      if (log_level >= DEBUG) {
	std::clog << timestamp << "|"
		  << location.file_name() << ':'
		  << location.line() << ':';
      }
      return std::clog;
      // lock is not released until logging::endl is called
    }
    return blackhole;
  }

  std::ostream& message(const std::source_location location = std::source_location::current()) {
    if (log_level >= STANDARD) {
      log_mutex.acquire();

      if (log_level >= DEBUG) {
	std::clog << "MSG:"
		  << timestamp << "|"
		  << location.file_name() << ':'
		  << location.line() << ':';
      }
      return std::cout;
      // lock is not released until logging::endl is called
    }
    return blackhole;
  }

  std::ostream& error(const std::source_location location = std::source_location::current()) {
    log_mutex.acquire();
    return std::cerr << "ERROR:"
		     << timestamp << "|"
		     << location.file_name() << ':'
		     << location.line() << " in `"
		     << location.function_name() << "`: ";
    // lock is not released until logging::endl is called
  }
}
