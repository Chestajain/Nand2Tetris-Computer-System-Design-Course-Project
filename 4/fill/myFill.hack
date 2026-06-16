// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/4/Fill.asm

// Runs an infinite loop that listens to the keyboard input. 
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel. When no key is pressed, 
// the screen should be cleared.
(RESTART)

    @SCREEN  
    D=A 

    @0
    M=D

(KBDCHCK)

    @KBD 
    D=M 

    @BLACKEN 
    D;JGT
    @WHITEN
    D;JEQ 

    @KBDCHCK
    0;JMP 

(BLACKEN)

    @3855
    D=A
    @1
    M=D

    @CHANGE
    0;JMP 

(WHITEN)
    @1
    M=0

    @CHANGE
    0;JMP

(CHANGE)
    @1
    D=M 
    @0
    A=M
    M=D
    @0
    D=M+1

    @KBD
    D=A-D
    
    @0
    M=M+1
    A=M
    
    @CHANGE 
    D;JGT
    @RESTART
    0;JMP
