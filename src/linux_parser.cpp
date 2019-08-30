#include <dirent.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <iomanip>
#include <unordered_map>
#include <assert.h>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string memtype, value;
  std::unordered_map <string, string> memory;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (filestream >> memtype >> value) {
      memory[memtype] = value;
    }
  }
  auto m = memory["MemTotal:"];
  auto n = memory["MemFree:"];
  //float memtotal = stof(m);
  //float memfree = stof(n);
  //return (memtotal - memfree) / memtotal;
  return 0.0;
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  std::ifstream filestream(kProcDirectory + kUpTimePath);
  string line;
  getline(filestream, line);
  std::istringstream buffer(line);
  std::istream_iterator<string> beg(buffer), end;
  vector<string> values(beg, end);
  return std::stoi(values[0]);
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  vector<string> jiff = CpuUtilization();
  long total_jiff = 0;
  for (string j : jiff) {
    total_jiff += atol(j.c_str());
  }
  return total_jiff;
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid[[maybe_unused]]) { return 0; }

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  return Jiffies() - IdleJiffies();
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> jiff = CpuUtilization();
  return atol(jiff[3].c_str());
  }

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  string user, nice, system, idle, iowait, irq, softirq, steal;
  string line;
  vector<string> values{};
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
    }
  values.push_back(user);
  values.push_back(nice);
  values.push_back(system);
  values.push_back(idle);
  values.push_back(iowait);
  values.push_back(irq);
  values.push_back(softirq);
  values.push_back(steal);
  return values;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string name, total_processes, value;
  string line;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> name >> value) {
        if (name == "processes") {
          total_processes = value;
          break;
        } // inner if
      } // inner while
    } // outer while
  } // outer if
  return std::atoi(total_processes.c_str());
 }

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string name, running_processes, value;
  string line;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> name >> value) {
        if (name == "procs_running") {
          running_processes = value;
        } // inner if
      } // inner while
    } // outer while
  } // outer if
  return std::atoi(running_processes.c_str());
}

float LinuxParser::CpuUtilization(int pid) {
  string line;
  string value;
  float result;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  getline(stream, line);
  string str = line;
  std::istringstream buf(str);
  std::istream_iterator<string> beg(buf), end;
  vector<string> values(beg, end);
  // acquiring relevant times for calculation of active occupation of CPU for
  // selected process
  float utime = ProcessUpTime(pid);
  float stime = stof(values[14]);
  float cutime = stof(values[15]);
  float cstime = stof(values[16]);
  float starttime = stof(values[21]);
  float uptime = UpTime();  // sys uptime
  float freq = sysconf(_SC_CLK_TCK);
  float total_time = utime + stime;// + cutime + cstime;
  float seconds = uptime - (starttime / freq);
  result = 100.0 * ((total_time / freq) / seconds);
  return result;
}



// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string cmdline;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (filestream.is_open()) {
    std::getline(filestream, cmdline);
  }
  return cmdline;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string name, value;
  int ram;
  string line;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> name >> value) {
        if (name == "VmSize:") {
          ram = std::stof(value.c_str());
          break;
        }
      }
    }
  }
  return to_string(ram / 1024);
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string name, uid, value;
  string line;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> name >> value) {
        if (name == "Uid") {
          uid = value;
          break;
        }
      }
    }
  }
  return uid;
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string uid = Uid(pid);
  string delimiter = ":x:" + uid;
  string user;
  string line;
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      if (line.find(delimiter) != std::string::npos) {
        user = line.substr(0, line.find(delimiter));
        break;
      }
    }
  }
   return user;
 }

 //  Read and return the uptime of a process
 long int LinuxParser::ProcessUpTime(int pid) {
   string line;
   string value;
   std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
   std::getline(stream, line);
   std::istringstream linestream(line);
   std::istream_iterator<string> beg(linestream), end;
   vector<string> values(beg, end);
   return float(stof(values[13]) / sysconf(_SC_CLK_TCK));
}
