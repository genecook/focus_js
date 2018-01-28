//-----------------------------------------------------------------------
// Copyright  © 2017,2018 Tuleta Software, Inc.
// (Reference ../docs/focus_js_license.html)
//-----------------------------------------------------------------------

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include <string.h>
#include <linux/limits.h>

#include "job_server.h"

namespace TULESOFT {

// all done? cool, generate summary report. In the current scheme, each thread will
// produce a summary, for each job run (we'll see how much overhead this represents)...

void job_server::generate_reports(std::vector<job_outcome> &results) {
    std::cout << "\nRuns directory: '" << project_dir_path << "'" << std::endl;

    // write csv spreadsheet. each entry: html-link-to-run-dir,status...

    pass_fail_report = project_dir_path + "/pass_fail_summary.csv";

    std::ofstream outfile;
    outfile.open(pass_fail_report);

    std::string path_title = "Path to run directory (or tar file)";
    
    outfile << "Job" << "," << path_title << "," << "Status" << std::endl;
    
    for (std::vector<job_outcome>::iterator i = results.begin(); i != results.end(); i++) {
       std::string dirname = (*i).RunDir();
       std::string rundir = (*i).RunDirPath();
       bool job_passed = (*i).ExitCode() == 0;
       std::string status = job_passed  ? "PASS" : "FAIL";

       char rundir_fullpath[PATH_MAX];
       char *spath = realpath(rundir.c_str(),rundir_fullpath);

       if (job_passed) {
	 if ((*i).Remove()) {
	   // passing test dir was deleted; no point in making 'link' to same...
	   //strcat(rundir_fullpath," (deleted)");
	   continue;
         } else if ((*i).Compress())
	   strcat(rundir_fullpath,".tar.gz");
       }
       
       char html_soft_link[PATH_MAX + 128];
       sprintf(html_soft_link,"file://%s",rundir_fullpath);

       outfile << dirname << "," << html_soft_link << "," << status << std::endl;
    }
    
    outfile.close();

    std::cout << "\n  Pass-fail summary: " << pass_fail_report << std::endl;
}

}
