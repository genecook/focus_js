//-----------------------------------------------------------------------
// Copyright  © 2017,2018 Tuleta Software, Inc.
// (Reference ../docs/focus_js_license.html)
//-----------------------------------------------------------------------

#ifndef __JOBCLASS__
#include <string>

namespace TULESOFT {

//!
//! The <i>job</i> represents a single test to be verified..
//!

class job {
 public:
 job() : do_compress(false), do_remove(false) {};
  ~job() {};
  
 job(std::string _project_dir, std::string _run_dir, std::string _cmdline, int _job_id, bool _do_compress, bool _do_remove)
   : project_dir(_project_dir), run_dir(_run_dir), cmdline(_cmdline), exit_code(-1), job_id(_job_id),
    do_compress(_do_compress), do_remove(_do_remove) {
  };

  //! The project directory name is formed from the project-name and unit-name.
  std::string ProjectDir() { return project_dir; };
  //! For each invocation of <b>focus_js</b>, all individual jobs runs have an assigned and unique <i>rrun-directory</i> name.
  std::string RunDir() { return run_dir; };
  //! The full path to a jobs run directory.
  std::string RunDirPath() { return rundir_path; };
  //! The complete job <i>command line</i> used to execute a job.
  std::string CommandLine() { return cmdline; };
  //! Returns true if a job directory may be compressed upon successful execution.
  bool Compress() { return do_compress; };
  //! Returns true if a job directory may be removed upon successful execution.  
  bool Remove() { return do_remove; };
  //! The <i>Run</i> does just what its name implies. It runs the job. All job output is collected in the job <i>RunDir</i>.
  void Run();
  //! After a job is run, this method may be used to compress (tar/gzip) the run directory.
  int CompressResults();
  //! After a job is run, this method may be used to remove the run directory.
  int RemoveResults();
  //! The ExitCode methods returns the outcome of a jobs execution. A system call is used for execution. The <i>exit code</i>
  //! reflects the system call exit code.
  int ExitCode() { return exit_code; };

 private:
  std::string project_dir;  // rooted path to project output directory
  std::string run_dir;      // name to use for job run directory
  std::string cmdline;      // command line to be executed.
  int         job_id;       // numeric job ID
  bool        do_compress;  // if true, tar/zip run dir, then remove same
  bool        do_remove;    // if true, remove (passing test) run dir
  
  // using input paths, job forms, then creates run-directory. heres the rooted path:
  
  std::string rundir_path;
  
  int exit_code;
};

};

#endif
#define __JOBCLASS__ 1




