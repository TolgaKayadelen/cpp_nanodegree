#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"
#include "processor.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) : pid_(pid ) {
  this->user_ = LinuxParser::User(pid);
  this->cmd_ = LinuxParser::Command(pid);
  this->ram_ = LinuxParser::Ram(pid);
  this->uptime_ = LinuxParser::ProcessUpTime(pid);
  this->cpu_ = LinuxParser::CpuUtilization(pid);
}
// accessor that returns this process's ID
int Process::Pid() {
  return pid_; }

// accessor returns the process's CPU utilization
float Process::CpuUtilization() const {
  return cpu_ ;}

// accessor returns the command that generated this process
string Process::Command() {
  return cmd_; }

// accessor returns the  process's memory utilization
string Process::Ram() {
  return ram_; }

// accessor returns user (name) that generated this process
string Process::User() {
  return user_; }

// accessor returns the age of this process (in seconds)
long int Process::UpTime() {
  return uptime_;
}

// we overload the greater than operator so that processes with higher cpu are
// on top.
bool Process::operator>(const Process& that) const {
  return this->CpuUtilization() > that.CpuUtilization();
}
