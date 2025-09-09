        lw      0       1       2       load reg1 with 5 (symbolic address)
        lw      1       2       3       load reg2 with -1 (numeric address)
        add     1       2       1       decrement reg1
        nor     0       2       1
        jalr    2       1       
        beq     0       1       2       goto end of program when reg1==0
        beq     1       1       -32768  
        lw      2       1       0
        lw      0       1       -4
        noop
        halt                            end of program
        .fill   5
        .fill   -1