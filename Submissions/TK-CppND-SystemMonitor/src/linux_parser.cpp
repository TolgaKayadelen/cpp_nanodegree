#include <dirent.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <iomanip>
#include <unordered_map>

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
  string memtype, value, line;
  std::unordered_map <string, string> memory;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> memtype >> value;
      memory[memtype] = value;
    }
  }
  filestream.close();
  auto m = memory["MemTotal:"];
  auto n = memory["MemFree:"];
  return (stof(m) - stof(n)) / stof(m);
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  string line, uptime, idle;
  std::ifstream filestream(kProcDirectory + kUpTimePath);
  std::getline(filestream, line);
  std::istringstream linestream(line);
  linestream >> uptime >> idle;
  filestream.close();
  return std::stoi(uptime);
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  long jiffies = 0;
  for (string j : CpuUtilization()) {
    jiffies += atol(j.c_str());
  }
  return jiffies;
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
  return atol(CpuUtilization()[3].c_str());
  }

// Read and return System CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  string jiffy;
  vector<string> values{};
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (filestream >> jiffy) {
      values.push_back(jiffy);
    }
  }
  filestream.close();
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
  filestream.close();
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
  filestream.close();
  return std::atoi(running_processes.c_str());
}

// Cpu utilizatiob for process, based on stackoverflow answer in
// stackoverflow.com/questions/16726779
float LinuxParser::CpuUtilization(int pid) {
  string line, tmp;
  float sys_uptime;
  float utime, stime, cutime, cstime, starttime, hertz, seconds, total_time;
  float proc_cpu;
  vector<string> values{};
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    while (linestream >> tmp) {
      values.push_back(tmp);
    }
  }
  filestream.close();
  // get the required values.
  sys_uptime = UpTime();
  utime = stof(values[13]);
  stime = stof(values[14]);
  cutime = stof(values[15]);
  cstime = stof(values[16]);
  starttime = stof(values[21]);
  hertz = sysconf(_SC_CLK_TCK);

  // Calculate process cpu
  total_time = utime + stime;
  total_time = total_time + cutime + cstime;
  seconds = sys_uptime - (starttime / hertz);
  proc_cpu = (total_time / hertz) / seconds;
  return proc_cpu;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string cmdline;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (filestream.is_open()) {
    std::getline(filestream, cmdline);
  }
  filestream.close();
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
  filestream.close();
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
  filestream.close();
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
  filestream.close();
   return user;
 }

 //  Read and return the uptime of a process
 long int LinuxParser::ProcessUpTime(int pid) {
   string line;
   string value;
   std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
   std::getline(filestream, line);
   std::istringstream linestream(line);
   std::istream_iterator<string> beg(linestream), end;
   vector<string> values(beg, end);
   filestream.close();
   return float(stof(values[13]) / sysconf(_SC_CLK_TCK));
}
