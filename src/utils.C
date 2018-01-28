//-----------------------------------------------------------------------
// Copyright  © 2017,2018 Tuleta Software, Inc.
// (Reference ../docs/focus_js_license.html)
//-----------------------------------------------------------------------

#include <string>
#include <algorithm>
#include <ctime>
#include <csignal>
#include <stdexcept>
#include <stdlib.h>
#include <sys/time.h>
#include <linux/limits.h>

#include "utils.h"

// called from constructor. creates output directory, project directory...

namespace TULESOFT {
  
std::string todays_date() {
  // use standard time functions to form date/time string...
  
  std::time_t result = std::time(nullptr);
  std::string start_time = std::ctime(&result);

  // fixup the string to yield something reasonable to use as a directory name...
  
  std::replace(start_time.begin(),start_time.end(),' ','_');
  start_time.erase( start_time.find("\n"), 1);
  
  return start_time;
}

// Use system call to create run directory. Quick-n-dirty insofar as programming
// goes, but not efficient...

void make_run_dir(std::string rdir, std::string rdir_desc) {
    char cmdline[PATH_MAX + 128];
    sprintf(cmdline,"mkdir -p %s",rdir.c_str());
    
    const int dir_err = system(cmdline);

    if (dir_err == -1) {
      char tbuf[PATH_MAX + 128];
      sprintf(tbuf,"Unable to create %s directory: '%s'.", rdir_desc.c_str(), rdir.c_str());
      throw std::runtime_error(tbuf);
    }
  }
  
}

