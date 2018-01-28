//-----------------------------------------------------------------------
// Copyright  © 2017,2018 Tuleta Software, Inc.
// (Reference ../docs/focus_js_license.html)
//-----------------------------------------------------------------------

#ifndef __JOB_OUTCOME__

#include <string>

//!
//! After a job runs, a <i>job outcome</i> class instance is used to record a brief summary..
//! All <i>job outcomes</i> are recorded. After all jobs have been run, a brief pass/fail summary
//! report will be created, using the vector of <i>job outcomes</i>.
//!

class job_outcome {
 public:
  job_outcome() : do_compress(false), do_remove(false) {};
  ~job_outcome() {};

  //! Job servers use this constructor after a job ends, to record exit-code, run-dir path, dispensation.. 
  job_outcome(int _exit_code, std::string _run_dir, std::string _run_dir_path, bool _do_compress, bool _do_remove)
   : exit_code(_exit_code), run_dir(_run_dir), run_dir_path(_run_dir_path),
    do_compress(_do_compress), do_remove(_do_remove) {};

  //! The job was run via <i>system</i> call. This method returns the system call exit code.
  int ExitCode() { return exit_code; };

  //! The full path of the job run directory.
  std::string RunDirPath() { return run_dir_path; };
  
  //! The name of the job run directory.
  std::string RunDir() { return run_dir; };

  //! Returns true if the run directory is to be compressed on good exit code.
  bool Compress() { return do_compress; };

  //! Returns true if the run directory is to be removed on good exit code.
  bool Remove() { return do_remove; }; 
  
 private:
  int exit_code;
  std::string run_dir;
  std::string run_dir_path;
  bool do_compress;
  bool do_remove;
};

#endif
#define __JOB_OUTCOME__ 1


