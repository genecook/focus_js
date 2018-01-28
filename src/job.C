//-----------------------------------------------------------------------
// Copyright  © 2017,2018 Tuleta Software, Inc.
// (Reference ../docs/focus_js_license.html)
//-----------------------------------------------------------------------

#include <string>
#include <stdlib.h>
#include <linux/limits.h>

#include "utils.h"
#include "job.h"


namespace TULESOFT {
  
//! 'jobs' run - create what should be a unique directory. Use system call to
//!              execute the verification cmdline, diverting stdout/stderr to files.
//!              The exit code from the system call (what we assume is the outcome
//!              from the verification, is recorded.

void job::Run() {
  char my_rundir[PATH_MAX];
  sprintf(my_rundir,"%s/%s",project_dir.c_str(),run_dir.c_str());

  rundir_path = my_rundir;
  
  TULESOFT::make_run_dir(rundir_path,"run directory");

  char tbuf[rundir_path.size() + cmdline.size() + 128];
  sprintf(tbuf,"export JOB_ID=%d;cd %s;%s 1>runlog.stdout 2>runlog.stderr",job_id,rundir_path.c_str(),cmdline.c_str());
  
  exit_code = system(tbuf);
}

//! user has option of tar'ing dir for a job with 0 exit-code, or removing altogether...

int job::RemoveResults() {
  // remove run directory, ouch...

  char tbuf[project_dir.size() + run_dir.size() * 2 + 128];
  sprintf(tbuf,"cd %s;rm -rf %s 1>/dev/null 2>/dev/null",project_dir.c_str(),run_dir.c_str());
  
  return system(tbuf);
}

//! tar/gzip, then remove run directory...
  
int job::CompressResults() {
  char tbuf[project_dir.size() + run_dir.size() * 2 + 128];
  sprintf(tbuf,"cd %s;tar czf %s.tar.gz %s 1>/dev/null 2>/dev/null",
	  project_dir.c_str(),run_dir.c_str(),run_dir.c_str());

  int rcode = system(tbuf);

  // remove run directory now that its succcessfully been tar'd up..

  if (!rcode) rcode = RemoveResults();
  
  return rcode;
}

}
