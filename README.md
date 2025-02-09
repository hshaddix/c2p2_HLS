# Hello!! 

This is a repository for HLS related code pertaining to the Spring 2025 c2-the-p2 advanced seminar

## Startup Exercises:

  First, we will want to navigate to an environment in which we can utilize HLS.. 
Run this series of commands (with minor edits!) 
  - ssh -X -Y <YourUsername>@vmlab.niu.edu
  - source /opt/metis/el8/contrib/amdtools/xilinx/Vitis/2023.1/settings64.sh

  Now that we have our environment setup with xilinx and Vitis HLS, we can start to compile some code!!
Think of these exercises as more of a sandbox type of thing, where you can mess around with existing code by adjusting directives and structures to change the throughput and timing of the code. 

First clone this repo:
- git clone https://github.com/hshaddix/c2p2_HLS.git

Navigate into ExerciseFiles, here you should see several directories, one with working files, and two others with some files that are necessary to compile an RTL kernel using the HLS code we will be changing and working with. 

  We will start with some basic functions, including some pragmas as needed to get some sort of intuition with pipelining, throughput, and optimization for HLS.

Open the file 'NoPragmas.cpp' and take a look. Our first job will be to simply open this file in our text editor and make some attempts at including pragmas where needed into existing Cxx code. 

Try your best not to check any of the answers (located in 'WithPragmas.cpp') until you make an attempt at all 10 problems (Note: some will use the same directives). 

As a resource, use the AMD manual (https://docs.amd.com/r/en-US/ug1399-vitis-hls/HLS-Programmers-Guide) along with these past several lectures. Also, don't hesitate to ask questions!

  After making a good attempt at understanding the 10 problems in 'NoPragmas.cpp', we will move on to the actual vitis GUI. 

## C Synthesis and Main Exercises

(Step-by-step of getting GUI setup with HLS)
  
Open the file designated as 'Improper_Code.cpp.' Take a look at this code, do not focus on the logic itself, but note the data types. Compile this using the Vitis GUI, by clicking the 'C Synthesis' button in the bottom left. 
  - Make note of the resource utilization.
  - Navigate to the 'warnings' tab on the bottom. What do you see?
  - Consider how this file may need some brushing up given the concepts you have seen regarding pipelining, parallelization, and throughput.

  Alternatively, we can run this file using the Makefile. 
  To do this, 
  1) move Improper_Code.cpp into the kernel/ directory
  2) change the name to processHits.cpp
  3) go back into src/ and run "make" to compile. 

Next.. 
