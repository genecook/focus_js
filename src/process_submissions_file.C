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
#include <set>
#include <exception>


#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/info_parser.hpp"
#include "boost/foreach.hpp"

namespace pt = boost::property_tree;

#include "job_server.h"

namespace TULESOFT {

//!
//! A <i>jobs submission file</i> contains one or more job submissions. Overall options in the file
//! (see example file <i>example_project.info</i>) include project name, output directory, repeat
//! count, and fails threshhold (# of fails to tolerate before aborting all remaining jobs).
  
int job_server::process_submissions_file(std::vector<TULESOFT::job_submission> &submissions,std::string submissions_file) {

    int num_submits = 0;

    // using boost ptree, info parser (simplified json format sort of)...
  
    pt::ptree tree;

    // parse the entire file contents into tree...
    
    pt::read_info(submissions_file, tree);

    // file describes job submissions for a single project...
    
    pt::ptree project = tree.get_child("project");

    // pick off project parameters...
    
    std::string project_name     = project.get<std::string>("name");
    std::string output_dir       = project.get<std::string>("output_directory");
    int         repeat_count     = project.get<int>("run_count", 1);
    int         fails_threshhold = project.get<int>("fails_threshhold", -1);
    
    std::cout << "    project: " << project_name << std::endl;
    std::cout << "    output directory: " << output_dir << std::endl;

    if (repeat_count > 1)
      std::cout << "    run count: " << repeat_count << std::endl;

    if (fails_threshhold != -1)
      std::cout << "    max fails count: " << fails_threshhold << std::endl;
      
    std::cout << "    units:";

    // traverse the tree child nodes looking for units...
    
    for (pt::ptree::iterator child = project.begin(); child != project.end(); child++) {
      if (child->first == "unit") {
	pt::ptree unit = child->second;
	
	std::string unit_name        = unit.get<std::string>("name","?");
	std::string run_script       = unit.get<std::string>("run_script","?");
	std::string files_pattern    = unit.get<std::string>("files","");
	int         unit_run_count   = unit.get<int>("run_count", 1);
	std::string options          = unit.get<std::string>("options","");
	std::string dispensation     = unit.get<std::string>("passing_tests","compress");

	if (unit_name == "?") {
          std::cerr << "\nERROR: Unit-name missing." << std::endl;
	  num_submits = 0;
	  break;
	}
	if (run_script == "?") {
          std::cerr << "\nERROR: For unit '" << unit_name << "', no run-script specified." << std::endl;
	  num_submits = 0;
	  break;
	}
	
        bool do_compress = (dispensation == "compress");
        bool do_remove   = (dispensation == "remove");

	if (do_compress && do_remove) {
          std::cerr << "\nERROR [Unit-name %s]: 'compress' and 'remove' may not both be specified." << std::endl;
	  num_submits = 0;
	  break;
	}
	  
        std::cout << " " << unit_name;

	int run_count = repeat_count * unit_run_count;

	TULESOFT::job_submission next_job(output_dir,project_name,unit_name,run_script,
					  files_pattern,options,run_count,do_compress,
					  do_remove,
					  fails_threshhold // recorded on every submission, but applies
					                   // to entire project
					  );

	submissions.push_back(next_job);
	num_submits += 1;
      }
    }

    std::cout << std::endl;
    
    return num_submits;
}

}

