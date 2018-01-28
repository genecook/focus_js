//-----------------------------------------------------------------------
// Copyright  © 2017,2018 Tuleta Software, Inc.
// (Reference ../docs/focus_js_license.html)
//-----------------------------------------------------------------------

#ifndef __JOB_SUBMISSION__

#include <string>

namespace TULESOFT {

//!
//! A <i>job-submission</i> results in one or more actual job-runs. A single <i>job-submission</i>
//! can result in many individual jobs being run. 
//!

class job_submission {
 public:
 job_submission() : run_count(1), compress_passes(false), remove_passes(false) {};
  ~job_submission() {};

  job_submission(std::string _output_directory, std::string _project, std::string _unit, 
                 std::string _run_script, std::string _files_pattern, std::string _options,
                 int _run_count, bool _compress_passes, bool _remove_passes, int _fail_threshhold)
    : output_directory(_output_directory), project(_project), unit(_unit), run_script(_run_script),
    files_pattern(_files_pattern), options(_options), run_count(_run_count),
    compress_passes(_compress_passes), remove_passes(_remove_passes), fail_threshhold(_fail_threshhold) {

    if (compress_passes && remove_passes)
      throw std::logic_error("job_submission: compress_passes and remove_passes cannot both be set.");
  };

  //! The <i>main</i> output directory name or path. All project run directories are created as sub-directories of
  //! the <i>output directory</i>..
  std::string OutputDirectory() { return output_directory; };
  //! The <i>project name</i>. Also, a sub-directory of the <i>output directory</i>.
  std::string Project() { return project; };
  //! All jobs are grouped together in <i>unit</i> sub-directories underneath the <i>project directory</i>.
  std::string Unit() { return unit; };
  //! The <i>run script</i> represents a test tool executable or shell script, and presumably some verification process.
  std::string RunScript() { return run_script; };
  //! The (optional) <i>file pattern</i> is evaluated when each <i>job submission</i> is processed, to yield a list
  //! of files. 
  std::string FilesPattern() { return files_pattern; };
  //! The <i>options</i> string records any command line options to be passed to the <i>run script</i>, when each individual
  //! job is run.
  std::string Options() { return options; };
  //! A <i>run count</i> is associated with a job-submission. Each individual job resulting from evaluating a <i>job submission</i>
  //! will be run multiple times, according to the <i>run count</i>.
  int RunCount() { return run_count; };
  //! <i>Compress</i> returns true if a (passing test) job directory may be compressed (tar'd/gzip'ed) after execution.
  bool Compress() { return compress_passes; };
  //! <i>Remove</i> returns true if a (passing test) job directory may be removed after execution. 
  bool Remove() { return remove_passes; };
  //! A <i>fails threshhold</i> is associated with a job-submission. If at any point the fails threshhold (total # of fails)
  //! is met or exceeded, all remaining jobs (jobs not yet picked up by some server) are discarded and execution halts.
  //! NOTE: In current implementation there is only a single project wide fail threshhold.
  int FailThreshhold() { return fail_threshhold; };

 private:
  std::string output_directory;        // output directory
  std::string project;                 // project directory
  std::string unit;                    // project sub-directory 
  std::string run_script;              // verification script or program to use
  std::string files_pattern;           // file(s) - file pattern
  std::string options;                 // command line options for verification script
  int         run_count;               // # of times to run
  bool        compress_passes;         // tar/zip then remove passing rub-directories
  bool        remove_passes;           // just remove passing rub-directories
  int         fail_threshhold;         // fails threshhold - remaining jobs aborted when threshhold reached for a unit
};
 
};

#endif
#define __JOB_SUBMISSION__ 1

