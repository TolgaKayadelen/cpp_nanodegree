#include "processor.h"
#include "linux_parser.h"

// Return the aggregate CPU utilization
Processor::Processor() {
  this->prev_idle_jiff = 0;
  this->prev_total_jiff = 0;
}

float Processor::Utilization() {
  long idle_jiff = LinuxParser::IdleJiffies();
  long total_jiff = LinuxParser::Jiffies();
  long idle_diff = idle_jiff - this->prev_idle_jiff;
  long total_diff = total_jiff - this->prev_total_jiff;
  float diff_usage = (10*(total_diff - idle_diff)/total_diff)/(float)10;
  this->prev_idle_jiff = idle_jiff;
  this->prev_total_jiff = total_jiff;
  return diff_usage;
}
