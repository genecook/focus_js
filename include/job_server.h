//-----------------------------------------------------------------------
// Copyright  © 2017,2018 Tuleta Software, Inc.
// (Reference ../docs/focus_js_license.html)
//-----------------------------------------------------------------------

#ifndef __JOB_SERVER__

#include <string>

#include "utils.h"
#include "job.h"
#include "job_outcome.h"
#include "job_submission.h"

namespace TULESOFT {

//!
//! <i>Servers</i> run jobs. There is one server per thread. When a <i>server</i>
//! runs, its sole purpose is to get (and remove) the next job from the job queue
//! and execute same. The server exits (returns from the <i>run</i> method when
//! the job queue is empty.
//!

class server {
  public:
    server(int _id, bool _verbose = false) : id(_id), verbose(_verbose) {};
    ~server() {};

    //! The server <i>run</i> method executes until there are no more jobs to run.
    void run();
    //! This method causes a single job to be executed, records the results,
    //! and optionally causes the job directory to be compressed or removed.
    void service_request(job &the_request);
    
  private:
    int id;
    bool verbose;
};

//!  
//! The <i>job-server</i> class handles all aspects of submitting/running jobs, and managing servers..
//!
 
class job_server {
  public:
 job_server() : thread_count(-1), fails_threshhold(-1) {};
  ~job_server() {};

  //! Start up a <i>job server</i> using a set of job submissions, and (optionally) a thread count. 
  job_server(std::vector<job_submission> &submissions,int _thread_count) : thread_count(_thread_count) {
    init(submissions);
  };

  //! Job submission parameters may be specified via the command line, or from a file. A job-submissions file may
  //! contain one or more job-submissions. This method is used to parse a <i>job submissions file</i> into a list
  //! (vector) of job submissions.
  static int process_submissions_file(std::vector<TULESOFT::job_submission> &my_submissions,std::string submissions_file);

  //! After creating an instance of the server, all one need do is call Run.
  int Run(); 
  //! Used to cause all servers running to shut down. This could occur due to the user typing <i>ctrl-C</i>, or
  //! (unbelievable as it might be) due to some internal error detected.
  static void shut_down_handler(int s);

 private:
  //! The <i>init</i> is called from the constructor.
  void init(std::vector<job_submission> &submissions);
  //! Evaluating a <i>job submission</i> could result in one or more individual verification jobs being run. This
  //! method is used to evaluate and as a result expand a <i>job submission</i> into a set of jobs to be run.
  void expand_submission(job_submission &submissions);
  //! This method will evaluate a file-pattern and produce a set of file-paths.
  bool expand_filelist(std::vector<std::string> &files_list,std::string pattern);
  //! This method is used to reset (unfortunately) some global variables necessary for multi-thread coordination.
  void reset_globals();
  //! This method is used to evaluate all job submissions.
  void process_job_submissions();
  //! 
  void service_job_requests();
  void run_all_jobs(int thread_count_to_use);

  void generate_reports(std::vector<job_outcome> &results);

  void queue_up_request(job my_request);

  int QueuedCount();

  static void start_new_server(); //<---each task starts via this method. must be static.

  std::string project_dir_path; // expanded project-dir path here:

  std::string pass_fail_report;

  int thread_count;

  int fails_threshhold;
  
  // used to create a succinct and unique ID for each task:
  
  static int queue_count;
};

};

#endif
#define __JOB_SERVER__ 1

