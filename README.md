# Hello!! 

This is a repository for HLS related code pertaining to the Spring 2025 c2-the-p2 advanced seminar

## Main Exercise:

First we will want to navigate to an environment in which we can utilize HLS... 

  Now that we have our environment setup with xilinx and Vitis HLS, we can start to compile some code!!
Think of these exercises as more of a sandbox type of thing, where you can mess around with existing code by adjusting directives and structures to change the throughput and timing of the code. 

First clone this repo:
- git clone https://github.com/hshaddix/c2p2_HLS.git

Navigate into HLS_Exercises, here you should see several directories, one with working files, and two others with some files that are necessary to compile an RTL kernel using the HLS code we will be changing and working with. 

  First, open the file designated as 'Improper_Code.cpp.' Take a look at this code, do not focus on the logic itself, but note the data types, the loops, and considerable lack of efficiency pragmas. Compile this using the Vitis GUI, by clicking the 'C Synthesis' button in the bottom left. 
  - Make note of the resource utilization.
  - Navigate to the 'warnings' tab on the bottom. What do you see?
  - Consider how this file may need some brushing up given the concepts you have seen regarding pipelining, parallelization, and throughput.

  Alternatively, we can run this file using the Makefile. 
  To do this, 
  1) move Improper_Code.cpp into the kernel/ directory
  2) change the name to processHits.cpp
  3) go back into src/ and run "make" to compile. 

Next.. 
