#!/bin/sh -f

#----------------------------------------------------------------------------
# Random test generation.
#
# Generate N-instr random instr test, output ELF, simulate using the ELF.
#
# NOTE: Using a64sim to both generate the random test, and to simulate the
#       test afterwards. For demo purposes. We don't expect the test to fail.
#-----------------------------------------------------------------------------


# focus_js sets unique shell variable for each running job...

echo JOB_ID IS $JOB_ID


# will ASSUME a64sim is also installed, and in standard install directory...

export INSTALL_DIR=/opt/tuleta_software/a64sim
export A64SIM=$INSTALL_DIR/bin/a64sim


# use sample 'boot loader' code...

export BOOT_CODE=$INSTALL_DIR/test/boot2el0.obj


# set default instruction count...

if [ -z "$INSTR_COUNT" ]
then
export INSTR_COUNT=1000
fi


# Generate new random test, output ELF only, disassembly to stdout...

export ELF_OUT=gend_test.elf

$A64SIM -L $BOOT_CODE -F -n $INSTR_COUNT -D -S $ELF_OUT $* 1>gen.log 2>&1

# Bail if test fails to generate...

if [ $? -ne 0 ]
then
    echo "FAIL: generation."
    exit 1
fi


# Now simulate the test using the ELF file...

# Specify instruction count as safeguard, in case simulator goes off in the weeds.

$A64SIM -L $ELF_OUT -n $INSTR_COUNT -D $* 1>sim.log 2>&1

# Final exit code based on outcome of simulation.

if [ $? -eq 0 ]
then
    echo "PASS: simulation."
    exit 0
else
    echo "FAIL: simulation."
    exit 1
fi








