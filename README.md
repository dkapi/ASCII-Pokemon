TO RUN : run make cmd without parameters then run binary

-known bugs
flying south is really questionable and i dont know why, because flying 
north is fine. 

the logic is there for removing gates on edges, in case you tried to check
south gate first, please fly to f 400 400, and as you can see no border.

going step by step I.E. n,s,e,w all work perfectly fine

-funny bug, sometimes when flying the grid will print twice, dosent break
 anything but i think the error is in the pointers.

-also logic is there for not moving further if you are already on a border
 grid, but im getting a weird seg fault after that logic, so while that part
 of the assignment is implemented in lines 58 of worldgen.c and it works. 
 until later code is reached...
