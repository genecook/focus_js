//-----------------------------------------------------------------------
// Copyright  © 2017,2018 Tuleta Software, Inc.
// (Reference ../docs/focus_js_license.html)
//-----------------------------------------------------------------------

#include <thread>
#include <mutex>
#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <sstream>
#include <fstream>
#include <csignal>

#include <unistd.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <math.h>

#include "job_server.h"

namespace TULESOFT {

//*********************************************************************************
// There is a single jobs request queue. All servers process requests from this
// queue. When the queue becomes empty all servers shut down.
//*********************************************************************************

// request queue, mutex are global. sad!

std::mutex server_mutex;          // the set of global variables below (once server threads
                                  // are running) should only be accessed by using the mutex

std::queue<job> requests;         // a single queue of job requests, accessed and drained
                                  // by whatever job-servers are running.

int done_count;                   //
int pass_count;                   // <---updated by each run as it completes
int fail_count;                   //

int max_fails;                    // servers shutdown if max fails count exceeded
  
bool shutdown_now;                // need big hammer to stop threads if user hits ctrl-C

bool system_fail;                 // if some thread 'system call' fails, this flag is set
                                  //   and we all fall down...

std::vector<job_outcome> results; // for usability we do need some 'at a glance' way  of knowing
                                  //  which tests failed.

//*********************************************************************************
// server class - each thread 'runs' a server. Servers run 'til the job queue is
//                empty...
//*********************************************************************************

// run - retreive/run next job from the queue. quit when queue empty...

void server::run() {
  bool more_jobs = !shutdown_now; // catch request to shutdown early if possible

   while(more_jobs) {
     job next_job_request;
     {
      std::lock_guard<std::mutex> guard(server_mutex);
      if (shutdown_now) {
	// shut down now irregardless of whats left to do...
	more_jobs = false;
      } else if ( (max_fails >= 0) && (fail_count > max_fails) ) {
	// maxed out on fails...
	more_jobs = false;	
      }	else if (requests.size() > 0) {
	// retreive and remove next job request...
	next_job_request = requests.front();
	requests.pop();
      } else {
	// job queue has been drained. we're done...
	more_jobs = false;
      }
     }
       
     if (more_jobs) {
       service_request(next_job_request);
     }
   }
}

// queue up a single job to run...

void job_server::queue_up_request(job my_request) {
    std::lock_guard<std::mutex> guard(server_mutex);
    requests.push(my_request);
}

int job_server::QueuedCount() {
    return requests.size();
}
  
// service_request - run one job. update pass/fail/done counts...

void server::service_request(job &the_request) {
  the_request.Run();

  bool test_passed = (the_request.ExitCode() == 0);
  
  {
   std::lock_guard<std::mutex> guard(server_mutex);
   done_count++;

   if (test_passed) {
     pass_count++;
   } else
     fail_count++;

   results.push_back(job_outcome(the_request.ExitCode(),the_request.RunDir(),the_request.RunDirPath(),
				 the_request.Compress(),the_request.Remove()));
  }

  if (test_passed) {
    if (the_request.Compress()) {
      if (the_request.CompressResults()) {
        std::lock_guard<std::mutex> guard(server_mutex);
        system_fail = true;
        shutdown_now = true;
      }
    } else if (the_request.Remove()) {
      if (the_request.RemoveResults()) {
        std::lock_guard<std::mutex> guard(server_mutex);
        system_fail = true;
        shutdown_now = true;
      }
    }
  }
}

// (re)set global variables...
  
void job_server::reset_globals() {
    done_count = 0;       //
    pass_count = 0;       // -- not using mutex here since threads have not yet started
    fail_count = 0;       //
    max_fails = -1;       //
    
    shutdown_now = false; // will set if user types ctrl-C

    system_fail = false;  // task could set this on system-related issue with job(s)
}
  
//*********************************************************************************
// job_server 'wrapper' class...
//*********************************************************************************

int job_server::Run() {
  int rcode = 0;
  
   if (QueuedCount() > 0) {
     service_job_requests();   
     generate_reports(results);
   } else {
     std::cout << "No jobs are in queue. Nothing to do..." << std::endl;
     rcode = -1;
   }

   return rcode;
}

// start_new_server - is a static class method, run by each thread at thread startup

int job_server::queue_count = 0; //<--- must also be static, since used in start_new_server

void job_server::start_new_server() {
  std::thread::id my_id = std::this_thread::get_id();

  int queue_id = 0;

  bool verbose = false;

  if (verbose) {
    std::lock_guard<std::mutex> guard(server_mutex);
    std::cout << "Activating new job server..." << std::endl;
  }

  {
    std::lock_guard<std::mutex> guard(server_mutex);
    queue_id = ++queue_count;
  }

  server my_server(queue_id, verbose);

  my_server.run();
}


// on interrupt, set 'shutdown' flag to allow servers to shutdown gracefully...

void job_server::shut_down_handler(int s) {
  {
    std::lock_guard<std::mutex> guard(server_mutex);
    shutdown_now = true;
    std::cout << "Shutting down servers..." << std::endl;
    fflush(stdout);
  }
}

// all jobs requests have been created and queued up. its time to start up job servers
// (one per thread) to run the jobs...

void job_server::service_job_requests() {
  unsigned long num_hardware_threads = std::thread::hardware_concurrency();

  std::cout << "\n  # of 'hardware' threads: " << num_hardware_threads;

  int thread_count_to_use = (thread_count > 0) ? thread_count : num_hardware_threads;

  if (thread_count_to_use != num_hardware_threads)
    std::cout << ",  # of threads requested to be used: " << thread_count_to_use;

  std::cout << "\n" << std::endl;
  
  int run_count = QueuedCount();
  
  // start the clock, run all jobs, stop the clock...

  std::cout << "  Running..." << std::endl;
  
  struct timeval t1,t2;
  gettimeofday(&t1,NULL);

  run_all_jobs(thread_count_to_use);

  gettimeofday(&t2,NULL);

  std::cout << "\n  All done.\n" << std::endl;


  // calculate/print some nice runtime stats...
  
  double elapsed_time = ((t2.tv_sec - t1.tv_sec) * 1000.0)        // pick up milliseconds part
                          + ((t2.tv_usec - t1.tv_usec) / 1000.0);   //  then microseconds...

  double time_per_job = round(elapsed_time / run_count); // assuming all jobs are of same duration

  elapsed_time = elapsed_time / 100.0; // milliseconds was okay for measuring simulation time, but a bit
                                       // course for a job server. lets show seconds instead
  
  printf("  Elapsed time: %.2f seconds (approx. %-.0f milliseconds per job)\n\n",elapsed_time,time_per_job);
  

  // print pass/fail summary, possible system-fail message...
  
  if (system_fail) {
    // actually we don't know why the system error, but it was triggered when one or more
    // job directories were being compressed...
    std::cout << "\nERROR: System fail detected. Probable cause: disk space?\n" << std::endl;
  }
  
  std::cout << "  # passes: " << pass_count  << std::endl;
  std::cout << "  # fails:  " << fail_count  << std::endl;


  // if system errors or user aborted, there could be pending jobs...
  
  if (requests.size() > 0)
    std::cout << "  # requests pended:  " << requests.size() << std::endl;    
}

  
// spawn threads, wait 'til all threads end (gasp)...
  
void job_server::run_all_jobs(int thread_count_to_use) {

  max_fails = fails_threshhold; // set max fails count before threads start

  int run_count = QueuedCount(); // get the count before threads start processing
  
  std::vector<std::thread> threads;

  for (int i = 0; i < thread_count_to_use; i++) {
     threads.push_back(std::thread(start_new_server));
  }

  int num_done = 0;
  int fail_count_so_far = 0;
  int sleep_count = 1;

  if (run_count >= 10000)
    sleep_count = 5;

  // BEWARE: NO TIMEOUT MECHANISM IN PLACE!!!

  // Install ctrl-C handler on 'main' thread, after all server threads are started, to
  // keep SIGINT signal mask from propagating to each thread (if we set the signal
  // mask before the threads were created, then ALL threads would potentially receive
  // the signal AND its indeterminant which thread would get the interrupt. Thus all
  // threads would need to have a signal handler)...

  std::signal(SIGINT, shut_down_handler);

  for (int i = 0; num_done < run_count && !shutdown_now; i++) {
     {    
      std::lock_guard<std::mutex> guard(server_mutex);
      num_done = done_count;
      fail_count_so_far = fail_count;
     }

     int percent_done = (num_done * 100) / run_count;
     
     printf("\t%3d %% (%d out of %d) complete.\r", percent_done,num_done,run_count);
     fflush(stdout);

     if ( (fails_threshhold >= 0) && (fail_count_so_far > fails_threshhold) ) {
       fprintf(stderr,"NOTE: # of fails (%d) exceeds threshhold of %d. Aborting remaining jobs...\n",
	       fail_count_so_far,fails_threshhold);
       shut_down_handler(-1);
     }
     
     sleep(sleep_count);
  }
  
  for (auto& thread : threads) {   
     thread.join();
  }
}

}
