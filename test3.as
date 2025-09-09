        lw      0       1       five    load reg1 with 5 (symbolic address)
        lw      1       2       start   load reg2 with -1 (numeric address)
start   add     1       2       2       decrement reg1
        nor     0       2       1
        jalr    2       1       
        beq     0       1       neg1    goto end of program when reg1==0
        beq     0       0       stAddr  go back to the beginning of the loop
        beq     1       1       -32768  
        lw      2       1       start
        lw      0       1       done
        noop
done    halt                            end of program
five    .fill   done
neg1    .fill   neg1
stAddr  .fill   start                   will contain the address of start (2)