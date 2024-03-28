#include <source_location>
#include <string_view>

// TODO: possibly migrate to making an object file

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

  std::ostream& debug(const std::source_location location = std::source_location::current()) {
    if (log_level >= DEBUG) {
      return std::clog << "DEBUG:"
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
