# Hello!! 

This is a repository for HLS related code pertaining to the Spring 2025 c2-the-p2 advanced seminar

## Startup Exercises:

  First, we will want to navigate to the metis environment in case you have yet to do so!  
  - ssh -X -Y YourUsername@vmlab.niu.edu

Now that we are inside our working environment, we need to access the directories in which we will be viewing working files and adjusting code. 

First clone this repo:
- git clone https://github.com/hshaddix/c2p2_HLS.git

Navigate into ExerciseFiles, here you should see several directories, one with working files, and two others with some files that are necessary to compile an RTL kernel using the HLS code we will be changing and working with. 

We will start with some basic functions, including some pragmas as needed to get some sort of intuition with pipelining, throughput, and optimization for HLS.

Navigate into the working_files directory.
Open the file 'NoPragmas.cpp' and take a look. Our first job will be to simply open this file in our text editor and make some attempts at including pragmas where needed into existing Cxx code. 

Try your best not to check any of the answers (located in 'WithPragmas.cpp') until you make an attempt at all 10 problems (Note: some will use the same directives). 

As a resource, use the AMD manual (https://docs.amd.com/r/en-US/ug1399-vitis-hls/HLS-Programmers-Guide) along with these past several lectures. Also, don't hesitate to ask questions!

  After making a good attempt at understanding the 10 problems in 'NoPragmas.cpp', we will move on to the actual vitis GUI. 

## C Synthesis and Main Exercises

While still in the c2p2_HLS directory, run the command:
  1) source /opt/metis/el8/contrib/amdtools/xilinx-2023.1/Vitis/2023.1/settings64.sh
  2) vitis_hls

With this, we effectively have now initialized xilinx and vitis so we can open the GUI and work on the HLS itself and compiling in C-synthesis. 

NOTE: You may need to download x windows to be able to open the GUI. First try the above commands, and see if a window opens.
      For the case of macOS, downloading the free application 'XQuartz' works to enable this for me. 

Follow these instructions to properly get our HLS code setup in the GUI: 
  1) Click NO on the pesky option to open the new version of the IDE 
  2) Hover over 'File' in the top left and select 'New Project'; If in linux, simply click "New Project"
  3) Name the project (whatever you want!!) ; Make sure the path for the location goes into the c2p2_HLS directory you cloned then press 'Next' 
  4) Put 'processHits' as the name for the top-level function; Press 'Add Files' and navigate to c2p2_HLS/ExerciseFiles/Improper_Code.cpp and click 'Open.'
  5) Press 'Next' on the testbench page.
  6) The only important thing to change on this page is 'Vivado IP Flow Target' into 'Vitis Kernel Flow Target' in the dropdown menu; Press 'Finish.'
  7) If you have a white screen besides the dropdown menu, go to the 'Window' tab on the top left and go to 'Debug.'
  8) Now as our final step to get it set up and actually see things, hover over 'Solution' and go to 'Run C Synthesis -> Active Solution' and press 'ok' when the window shows up.

Now you have used Improper_Code.cpp to generate a utilization report. Click on Improper_Code.cpp and look at it, note the data types, and the amount of loops, iterative portions, etc. Do not overly focus on WHAT the logic is doing, but HOW the logic is doing things. From the 'Synthesis Summary' tab: 
  - Make note of the resource utilization.
  - Navigate to the 'warnings' tab on the bottom. What do you see?
  - Consider how this file may need some brushing up given the concepts you have seen regarding pipelining, parallelization, and throughput.

------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  Alternatively, we can run this file using the Makefile (If we want to look at reports this way, I'll need to look for the report file path, so try to get the GUI working)
  To do this: 
  
  1) move Improper_Code.cpp into the kernel/ directory
  2) change the name to processHits.cpp
  3) go back into src/ and run "make" to compile. 

------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

I want this section of exercise to be treated more of a sandbox, where you can adjust and make additions and play with it, then recompile to try to make things more or less optimal. There are a lot of things you could do, some things to get you started might be: 
  - Adding important PRAGMAS where I have placed // Insert Directive here
  - Fix 'bad' PRAGMAS as indicated in the file 
  - Adjust stream depths
  - Explore fixed point arithmetic (ap_fixed)
  - Change interface PRAGMAS and see what happens

There are more optimizations that can be made, and the file 'Proper_Code.cpp' in the same directory of c2p2_HLS is my attempt at adding as many directives and optimizations as I could (note, it also removes a lot of debug statements in an attempt to make the code the main focus as a way to get some insight after working on 'Improper_Code.cpp'!!). The important thing to note here is that 'Proper_Code.cpp' is NOT a cheat sheet, it is simply a version of the code with some potential optimizations. The purpose of this main exercise is to get you to navigate and get your hands dirty with actively manipulating the file.  
