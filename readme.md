# Task 1
Implemented the Client1 using the select function in C. It allows connecting to multiple sockets simultaneously 
and easily without having to worry about threads. The output data is being parsed and collected to the outputs array, 
which is printed to STDOUT every 100ms. Running the client1 produces similar looking data is shown in the instructions, 
so based on that I consider the solution as (reasonably) correct and robust. Since I'm not an expert in making SW for mission 
critical systems such as satellites, I do acknowledge the possibility of potential security/reliability problems in the code. 

To understand the data the server is outputting, I captured a small (40s) snippet of it and plotted it 
over time with a simple Python script, see the picture below. 

![task1](https://github.com/migi-t/fsw-homework/assets/77881230/d7446d4e-1051-418b-a62d-5780c108edae)

Each output value has a clear pattern (even though the graph has some issues drawing lines). Given the context, the data is related to a satellite flight systems.  Output1 is oscillating between the amplitudes of 5 and -5, using one floating point precision. 
Output2 is also oscillating between 5 and -5, but compared to output1 it has integer precision, and outputs with half the frequency. 
It can also be seen that the output2 preceeds output1 timewise by a little bit. 
Output3 had values of either 5 or 0, and it does not seem to correlate with the data from output1 or output2. 
I dont know what properties of a satellite system these values are depicting, but I suspect that with a little reading about the subject such things would become evident to me. 

# Task 2
