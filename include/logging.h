#ifndef LOGGING_H
#define LOGGING_H

#include <cstdlib>
#include <fstream>
#include <iostream>

class Logging {
  public:
    const char* filename = "arq.log";
    std::ofstream filestream;
    bool isOpen = false;
  
  void logline(std::string message) {
    filestream << message << std::endl;
  }

  static Logging& getInstance() {
    static Logging instance;
    return instance;
  }

  Logging() {
    filestream.open(filename);

    if (!filestream) {
      std::cout << "Failed to open filestream for Logging!" << std::endl;
    }

    isOpen = filestream.is_open();
  }

  ~Logging() {
    filestream.flush();
    filestream.close();
  }
};

#endif
