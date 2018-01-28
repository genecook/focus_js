//-----------------------------------------------------------------------
// Copyright  © 2017,2018 Tuleta Software, Inc.
// (Reference ../docs/focus_js_license.html)
//-----------------------------------------------------------------------

#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <sstream>
#include <fstream>

#include <linux/limits.h>
#include <glob.h>

#include "job_server.h"

namespace TULESOFT {

// init - called from job_server class constructor...

bool job_server::expand_filelist(std::vector<std::string> &files_list,std::string pattern) {
  glob_t glob_result;
  
  glob(pattern.c_str(),GLOB_TILDE,NULL,&glob_result);
  
  for (unsigned int i = 0; i < glob_result.gl_pathc; i++) {
    char file_fullpath[PATH_MAX];
    char *spath = realpath(glob_result.gl_pathv[i],file_fullpath);
    files_list.push_back(std::string(file_fullpath));
  }
  
  globfree(&glob_result);
  
  return files_list.size() > 0;
}
  
// expand one job-submission...

void job_server::expand_submission(job_submission &submission) {
    // create project directory. suffix project-name/unit-name with date...

    std::string this_date = TULESOFT::todays_date();

    char pdir[PATH_MAX];

    sprintf(pdir,"%s/%s",submission.OutputDirectory().c_str(),submission.Project().c_str());

    project_dir_path = pdir;

    sprintf(pdir,"%s/%s/%s",project_dir_path.c_str(),submission.Unit().c_str(),this_date.c_str());

    std::string unit_dir_path = pdir;
    
    TULESOFT::make_run_dir(unit_dir_path,"'next' project");

    // convert run_script (possibly relative) path to full path...

    char run_script_fullpath[PATH_MAX];
    char *spath = realpath(submission.RunScript().c_str(),run_script_fullpath);

    std::string run_script_path;
    
    if (spath != NULL)
      run_script_path = run_script_fullpath;
    else {
      char tbuf[PATH_MAX + 128];
      sprintf(tbuf,"Unable to resolve path to run-script: '%s'.",submission.RunScript().c_str());
      throw std::runtime_error(tbuf);
    }

    // expand out the files list (if any)...
    
    std::vector<std::string> files_list;
      
    if (submission.FilesPattern().size() > 0) {
      if (expand_filelist(files_list,submission.FilesPattern())) {
	std::cout << "files_pattern '" << submission.FilesPattern() << "' expanded to " << files_list.size()
		  << " entries." << std::endl;
      }
    }

    // having the files-list have at least one entry makes the logic below easier...
    
    if (files_list.size() == 0) {
      files_list.push_back("");
    }

    if (files_list.size() > 0)
      std::cout << "    # of files: " << files_list.size() << std::endl;
    
    int expanded_count = 0;
    
    for (int i = 0; i < submission.RunCount(); i++) {
       for (std::vector<std::string>::iterator j = files_list.begin(); j != files_list.end(); j++) {
 	  std::string next_file = (*j);
	  char jobname[128];
          sprintf(jobname,"%05d",expanded_count++);
          std::string job_dir = jobname; 

	  char cmdline[run_script_path.size() + submission.Options().size() + next_file.size() + 128];

	  sprintf(cmdline,"%s %s %s",run_script_path.c_str(),submission.Options().c_str(),next_file.c_str());

	  //std::cout << "\ncmdline: '" << cmdline << "'\n" << std::endl;
	  
          queue_up_request(job(unit_dir_path,job_dir,cmdline,i,submission.Compress(),submission.Remove()));
       }
    }

    std::cout << "    # of queued jobs: " << expanded_count << std::endl;

    // paradoxically, smallest fails threshhold will be used as the project-wide threshhold.
    // In current implementation there is only one job queue and only one project wide
    // fails count...
    
    if (submission.FailThreshhold() < fails_threshhold) 
      fails_threshhold = submission.FailThreshhold();
}
  
void job_server::init(std::vector<job_submission> &submissions) {
  std::cout << "\n# of job submissions: " << submissions.size() << "\n" << std::endl;
  
  for (std::vector<job_submission>::iterator i = submissions.begin(); i != submissions.end(); i++) {
    std::cout << "  Next submission:" << std::endl;
     expand_submission((*i));
  }

  std::cout << "\nTotal # of queued jobs: " << QueuedCount() << std::endl;

  reset_globals();
}

}
