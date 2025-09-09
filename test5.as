start   beq     0       1       2       goto end of program when reg1==0
        beq     0       0       start   go back to the beginning of the loop
        beq     1       1       -32768  
        lw      2       1       start
        lw      0       1       -4
