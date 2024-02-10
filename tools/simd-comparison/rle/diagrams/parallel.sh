#!/bin/bash

# Define the script or command to run
SCRIPT="./membw"
ARGUMENTS="536870912 64 20"

# Get the number of CPU cores
NUM_SOCKETS=4

# Use GNU parallel to run multiple instances of the script, one per core
parallel -j $NUM_SOCKETS "$SCRIPT $ARGUMENTS" ::: $(seq 1 $NUM_SOCKETS)