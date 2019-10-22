#**************************************************************************
# Makefile to build focus - the focus job-server.
#
#   Copyright  © 2017,2018 Tuleta Software, Inc.
#**************************************************************************

CPP      = g++
BOOSTLIB = /usr/lib/x86_64-linux-gnu/libboost_program_options.a

DOXYGEN  = /usr/bin/doxygen

CFLAGS   = -I./include -std=c++11 -pthread -O3
LDFLAGS  = ${BOOSTLIB}

HFILES   = utils.h job.h job_outcome.h job_submission.h job_server.h
CFILES   = main.C job_server.C job_server_init.C process_submissions_file.C reports.C job.C utils.C

INCLUDES = $(addprefix include/,$(HFILES))
SRCS     = $(addprefix src/,$(CFILES))

OBJECTS  = $(addprefix obj/, $(CFILES:.C=.o))

obj/%.o: src/%.C $(INCLUDES)
	$(CPP) $(CFLAGS) -c -o $@ $<

APP = focus_js


all:
	make clean
	make bin/$(APP) 
	@echo done.


bin/$(APP): $(OBJECTS) 
	$(CPP) $(CFLAGS) -o $@ $+ $(LDFLAGS) 

doc: Doxyfile
	$(DOXYGEN)


# test - Run a self-contained process (my_dummy_script.sh or whatever) ten times, passing cmdline option '-n 1000'
#        to the process. Use the -X option to specify a fails threshhold of one. Use the -K option to specify
#        that the run directory for any job that passes (exits with exit-code of 0) may be removed.

test: bin/$(APP)
	./bin/$(APP) -O foo -P bar -U baz -R ./test_scripts/my_dummy_script.sh -N 10 -L '-n 1000' -X 1 -K

# test2 - Pretty much same as test, but uses (job) submissions file...

test2: bin/$(APP) ./example_project.info
	./bin/$(APP) -S ./example_project.info

# test3 - This time, set shell variable to cause unique job 'log files' to be created for each job run
#         (the dummy script will create the log file in the $MY_PROJECT/logs directory).
#         Now execute focus_js again, this time the # of jobs is implied by the list of files...

test3: bin/$(APP) ./example_project.info
	export MY_PROJECT=`pwd`;./bin/$(APP) -S ./example_project.info
	./bin/$(APP) -O foo -P bar -U baz -R ./test_scripts/my_dummy_script.sh -F './logs/*txt'

clean:
	rm -f obj/*.o bin/$(APP) logs/*



