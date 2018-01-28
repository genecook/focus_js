//-----------------------------------------------------------------------
// Copyright  © 2017,2018 Tuleta Software, Inc.
// (Reference ../docs/focus_js_license.html)
//-----------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sys/time.h>

#include "boost/program_options.hpp"
#include "job_server.h"

//*************************************************************************
// job server main...
//*************************************************************************

// The tool banner including release # has a initial default value, but can be specified via ifdef
// from the top level makefile...

#ifndef BANNER_WITH_RELEASE
#define BANNER_WITH_RELEASE "focus_js - the focus job server, 1.0 - Tuleta Software Co. Copyright 2018. All rights reserved."
#endif

const size_t COMMAND_LINE_ERROR = 1;
const size_t SUCCESS = 0;
const size_t ERROR_UNHANDLED_EXCEPTION = 2;

void display_help() {
  printf("    Command line options:\n");
  printf("        --help (or -h)                         -- Display this help message.\n");
  printf("        --version (or -V)                      -- Print tool version information.\n");

  printf("\n      To specify a single job submission:\n\n");
	 
  printf("        --output_directory (or -O) <directory> -- Main output directory\n");
  printf("        --project (or -P) <project>            -- Project directory (will appear under 'output' directory)\n");
  printf("        --unit (or -U) <category>              -- Sub-project directory (will appear under 'project' directory)\n");
  printf("        --run_script (or -R) <script>          -- Script or program to execute\n");
  printf("        --files (or -F) <files>                -- Input files - optional, defaults to none\n");
  printf("        --options (or -L) <options>            -- Command line options to pass to 'run script' - optional, defaults to none\n");
  printf("        --run_count (or -N <count>             -- Number of runs to make - optional, defaults to 1\n");	 
  printf("        --thread_count (or -T) <count>         -- Number of threads to use - optional, defaults to hardware count\n");
  printf("        --compress_passes (or -Z) <yes|no>     -- If set, tar up 'passing' job directories - optional, default is 'no'\n");
  printf("        --clobber_passes (or -K) <yes|no>      -- If set, remove 'passing' job directories - optional, default is 'no'\n");
  printf("                                                    (Note: compress_passes and clobber_passes are mutually exclusive options)\n");
  printf("        --fails_count (or -X <count>           -- Number of fails that may be tolerated before aborting all remaining runs - optional, no max value.\n");
	 
  printf("\n      To specify job submissions from file:\n\n");
	 
  printf("        --submissions_file (or -S) <file>      -- File containing multiple job submissions - optional.\n");	 
}

int main(int argc, char **argv) {
  std::string version = BANNER_WITH_RELEASE;

  std::cout << "\n" << "\033[0;32m" << version << "\033[0m" << "\n" << std::endl;
  
  std::string output_directory;        // there is one output directory,
  std::string project;                 //   project directory,
  int         thread_count = -1;       //     and optional # of threads to employ
  bool        compress_passes = false; // if true, tar/zip, then remove each run-directory
                                       // (fails are left alone however)
  bool        remove_passes = false;   // or remove passing test dirs altogether
  int         fail_threshhold = -1;    // # of fails to tolerate before aborting
  
  // a 'job submission' specified by:
  std::string unit;                    // project sub-directory 
  std::string run_script;              // verification script or program to use
  std::string files_pattern;           // file(s) - file pattern
  std::string options;                 // command line options for verification script
  int         run_count=1;             // # of times to run

  std::string submissions_file;        // for multiple job submission
  
  try {
    namespace po = boost::program_options;
    po::options_description desc("Options");
    desc.add_options()
      ("help,h","Print help messages")
      ("version,V","Print tool version")
       
      ("output_directory,O",po::value<std::string>(),"Output directory")
      ("project,P",po::value<std::string>(),"Project")
      ("unit,U",po::value<std::string>(),"Unit")
      ("thread_count,T",po::value<int>(),"Number of threads")
      ("compress_passes,Z","Tar up job that exit cleanly, to save space.")
      ("clobber_passes,K","Remove jobs that exit cleanly, to really save space.")
      ("run_script,R",po::value<std::string>(),"Script or program to execute")
      ("files,F",po::value<std::string>(),"Input files")
      ("options,L",po::value<std::string>(),"Script command line options")
      ("run_count,N",po::value<int>(),"Number of runs to make")
      ("fails_count,X",po::value<int>(),"Number of fails to tolerate")

      ("submissions_file,S",po::value<std::string>(),"Job submission file");

       po::variables_map vm;

    try {
      po::store(po::parse_command_line(argc,argv,desc),vm);

      if (vm.count("version")) { return SUCCESS; }

      if (vm.count("help")) {
	display_help();
	return SUCCESS;
      }

      // job-submissions could come from file:

      bool have_job_file = false;
      
      if (vm.count("submissions_file"))  {
        submissions_file = vm["submissions_file"].as<std::string>();
	have_job_file = true;
      } else {
        // if not from file, then single job submission...
	
        if (vm.count("output_directory"))  {
          output_directory = vm["output_directory"].as<std::string>();
        } else {
          fprintf(stderr,"No main output directory specified.\n");
          return(-1);
        }
	
        if (vm.count("project"))  {
          project = vm["project"].as<std::string>();
        } else {
          fprintf(stderr,"No project specified.\n");
          return(-1);
        }
	
        if (vm.count("unit"))  {
          unit = vm["unit"].as<std::string>();
        }  else {
          fprintf(stderr,"No unit specified.\n");
          return(-1);
        }
	
        if (vm.count("thread_count"))  {
          thread_count = vm["thread_count"].as<int>();
	  if (thread_count == 0) {
	    fprintf(stderr,"NOTE: Thread count specified is zero.\n");
	    return(-1);
	  }
        }
	
        if (vm.count("compress_passes"))  {
          compress_passes = true;
        }

        if (vm.count("clobber_passes"))  {
          remove_passes = true;
        }

	if (compress_passes && remove_passes) {
          fprintf(stderr,"Both compress_passes and clobber_passes are set. Enable one or the other.\n");
          return(-1);
	}
	
        if (vm.count("run_script"))  {
          run_script = vm["run_script"].as<std::string>();
        } else {
          fprintf(stderr,"No run-script specified.\n");
          return(-1);
        }
	
        if (vm.count("files"))  {
          files_pattern = vm["files"].as<std::string>();
        }
	
        if (vm.count("options"))  {
          options = vm["options"].as<std::string>();
        }
	
        if (vm.count("run_count"))  {
          run_count = vm["run_count"].as<int>();
        }
	
        if (vm.count("fails_count"))  {
          fail_threshhold = vm["fails_count"].as<int>();
        }
	
      }
      
      po::notify(vm);
    }
    catch(po::error& e) {
      fprintf(stderr,"ERROR: %s\n",e.what());
      return COMMAND_LINE_ERROR;
    }
   }
  
  catch(std::exception& e) {
      fprintf(stderr,"Error(s) occurred when processing command line options\n");
      return ERROR_UNHANDLED_EXCEPTION;
  }


  std::vector<TULESOFT::job_submission> my_submissions;

  int scount = 0;
  
  if (submissions_file.size() > 0) {
     // all job submissions come from file...
    std::cout << "  Job submissions file: " << submissions_file << std::endl;

    scount = TULESOFT::job_server::process_submissions_file(my_submissions,submissions_file);
    
  } else {
     std::cout << "  Output directory: " << output_directory << std::endl;
     std::cout << "  Project: " << project << std::endl;
     std::cout << "  Unit: " << unit << std::endl;
     
     std::cout << "  Script or program to execute: '" << run_script << "'" << std::endl;
     
     if (files_pattern.size() > 0)
       std::cout << "  Input file(s): " << files_pattern << std::endl;
     
     if (options.size() > 0)
       std::cout << "  Run-script cmdline options: '" << options << "'" << std::endl;
     
     std::cout << "  Number of jobs to run: " << run_count << std::endl;
     
     std::cout << "  Compress passing job directories? " << (compress_passes ? "yes" : "no")  << std::endl;
     std::cout << "  Remove passing job directories? " << (remove_passes ? "yes" : "no")  << std::endl;
  
     if (!compress_passes && !remove_passes) {
       std::cout << "\nWARNING: Passing test directories (as per request) will NOT be tar'd up, or removed." << std::endl;
       std::cout << "         The ability to repeatedly produce many tests will significantly impact disk space." << std::endl;
     }

     TULESOFT::job_submission one_job(output_directory,project,unit,run_script,files_pattern,options,run_count,compress_passes,remove_passes,fail_threshhold);

     my_submissions.push_back(one_job);

     scount = 1;
  }
  
  if (thread_count > 0) {
    std::cout << "  Number of threads (number of parallel tasks): " << thread_count << std::endl;
  }

  if (scount >= 1) {
    TULESOFT::job_server my_server(my_submissions,thread_count);
    return my_server.Run();
  } else {
    std::cerr << "No jobs were submitted." << std::endl;
    return -1;
  }
}

