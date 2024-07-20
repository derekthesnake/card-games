#include <iomanip>
#include <source_location>
#include <string_view>

// TODO: possibly migrate to making an object file
// TODO: make timestamps print (well-formatted) at >= debug levels

namespace logging {
  
  enum loglevel_t {
    SILENT = 0,
    STANDARD = 1,
    INFO = 5,
    DEBUG = 10,
  };

  static loglevel_t log_level = STANDARD;

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

  std::ostream& timestamp(std::ostream& str) {
    std::time_t now = std::time(nullptr);
    const std::chrono::zoned_time tz{
    const std::chrono::time_point<std::chrono::system_clock> baseline = std::chrono::system_clock::now();
    const auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(baseline).time_since_epoch().count();
    const auto secs = std::chrono::floor<std::chrono::seconds>(baseline).time_since_epoch().count();
    const auto mins = std::chrono::floor<std::chrono::minutes>(baseline).time_since_epoch().count();
    const auto hours = std::chrono::floor<std::chrono::hours>(baseline).time_since_epoch().count();
    const std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(baseline)};
    
    std::tm * time = std::localtime(&now);
    str << "|" << std::put_time(time, "%F %T") << "|" << static_cast<int>(ymd.year()) << "/" << static_cast<unsigned>(ymd.month()) << "/" << static_cast<unsigned>(ymd.day()) << " " << hours % 24 << ":" << mins % 60 << ":" << secs % 60 << "." << std::setw(3) << std::setfill('0') << std::setw(3) << ms % 1000;
    return str;
  }
  
  std::ostream& debug(const std::source_location location = std::source_location::current()) {
    if (log_level >= DEBUG) {
      return std::clog << "DEBUG:"
		       << timestamp << "|"
		       << location.file_name() << ':'
		       << location.line() << ": ";
    }
    return blackhole;
  }
  
  std::ostream& info(const std::source_location location = std::source_location::current()) {
    if (log_level >= INFO) {
      std::clog << "INFO:";
      if (log_level >= DEBUG) {
	std::clog << location.file_name() << ':'
		  << location.line() << ':';
      }
      return std::clog;
    }
    return blackhole;
  }

  std::ostream& message(const std::source_location location = std::source_location::current()) {
    if (log_level >= STANDARD) {
      if (log_level >= DEBUG) {
	std::clog << "MSG:" << location.file_name() << ':'
		  << location.line() << ':';
      }
      return std::cout;
    }
    return blackhole;
  }

  std::ostream& error(const std::source_location location = std::source_location::current()) {
    return std::cerr << "ERROR:"
		     << location.file_name() << ':'
		     << location.line() << " in `"
		     << location.function_name() << "`: ";
  }
}
