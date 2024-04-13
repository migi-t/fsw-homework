# Task 1
Implemented the Client1 using the select function in C. It allows connecting to multiple sockets simultaneously 
and easily without having to worry about threads. The output data is being parsed and collected to the outputs array, 
which is printed to STDOUT every 100ms. Running the client1 produces similar looking data is shown in the instructions, 
so based on that I consider the solution as (reasonably) correct and robust. Since I'm not an expert in making SW for mission 
critical systems such as satellites, I do acknowledge the possibility of potential security/reliability problems in the code. 

To understand the data the server is outputting, I captured a small (40s) snippet of it and plotted it 
over time with a simple Python script (code in utils/data_proc.py), see the graph below. 

![task1](https://github.com/migi-t/fsw-homework/assets/77881230/d7446d4e-1051-418b-a62d-5780c108edae)

Each output value has a clear pattern (even though the graph has some issues drawing lines). Given the context, the data is related to a satellite flight systems.  Output1 is oscillating between the amplitudes of 5 and -5, using one floating point precision. 
Output2 is also oscillating between 5 and -5, but compared to output1 it has integer precision, and outputs with half the frequency. 
Output3 had values of either 5 or 0, and it does not seem to correlate with the data from output1 or output2. 
I dont know what properties of a satellite system these values are depicting, but I suspect that with a little reading about the subject such things would become evident to me. 

# Task 2
Built the Client2 on top of the Client1. Added the UDP socket connection, changed from 100ms loops to 20ms, added logic about the output values from the output3, figured out the Control Message structure and implemented the Control Message sending. Figuring out the accepted values for the control message fields Property and Value took a long time, I ended up making a simple C program to go over the possible values for the Property field, while using invalid value in the Value field (code in utils/cntrl_msg_discovery.c). This was possible since the server prints error messages to STDOUT upon invalid control messages. Idk if this was a planned part of the task or if I misunderstood something. But anyways, when running client2 it is evident that the control messages are working, as the server prints OK messages upon changes. Also the prints show the altering amplitudes of output1 and timestamps are valid. I again used the python script to plot a snippet of the client2 output, see the graph below.  

![task2](https://github.com/migi-t/fsw-homework/assets/77881230/c8b4717b-141b-4313-b34c-5d81fd0a0684)

Based on it the changing amplitudes and frequencies of output1 (blue line) are clear. 

# General
I dont really anything special to say about the design of my code, I tried to keep it simple and relatively clean. Overall a very nice exercise!
