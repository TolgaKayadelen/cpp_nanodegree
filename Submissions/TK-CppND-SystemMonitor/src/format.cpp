#include <string>

#include "format.h"

using std::string;
using std::to_string;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long times) {
  int minutes = times / 60;
  int seconds = times % 60;
  int hours = minutes / 60;
  minutes = minutes % 60;
  return to_string(hours) + ":" + to_string(minutes) + ":" + to_string(seconds);
}
