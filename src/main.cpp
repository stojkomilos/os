#include "../lib/console.h"

#include "../h/alloc.h"
#include "../h/my_console.h"

extern "C" void trapRoutine();
int gTimer = 0;
void userMain();

void main()
{
    // should be before changing sstatus 0x2 bit
    __asm__ volatile ("csrw stvec, %[vector]" : : [vector] "r" (&trapRoutine));

    // set MODE to 0
    __asm__ volatile ("csrc stvec, 0x1");
    __asm__ volatile ("csrc stvec, 0x2");

    __asm__ volatile ("csrs sstatus, 0x02"); // sets the "sie" bit to 1 (s interupt enable)


    __asm__ volatile ("li a0, 1");
    __asm__ volatile ("li a1, 17");
    __asm__ volatile ("ecall");

//    userMain();
}

void debugInterrupt(int, int);

void cTrapRoutine()
{
    uint64 scause;
    __asm__ volatile ("csrr %[name], scause" : [name] "=r"(scause));

    int isExternal = (scause & (0x1UL << 63)) != 0;
    int cause = scause & (~(1UL << 63));

    debugInterrupt(isExternal, cause);

    if(isExternal == 1)
    {
        __asm__ volatile ("csrc sip, 0xA"); // clears the 9th bit in register sip. (the bit signifies external interupt)

        if (cause == 1) {
            if (gTimer % 10 == 0) {
                putString("Time: ");
                putInt(gTimer / 10);
                putString("s");
                putNewline();
            }
            gTimer++;
        }
    }
    else
    {
        __asm__ volatile ("csrc sip, 0x2"); // clears the 1st bit in register sip. (the bit signifies internal
        if(cause == 8 || cause == 9)
        {
            //dispatchSystemCall();
            uint64 code, parameter1;
            __asm__ volatile ("mv %[name], a0" : [name] "=r"(code));
            __asm__ volatile ("mv %[name], a1" : [name] "=r"(parameter1));

//            __putc('a');
            gTimer+= 99;

            putString("system call. Code=");
            putU64(code);
            putString( " parameter1=");
            putU64(parameter1);
            putNewline();

            assert(code != 0);

            if(code == 1)
            {
//                MemAlloc::get()->allocMem(parameter1);
            }
            else if (code == 2)
            {
//                MemAlloc::get()->freeMem(nullptr);
                assert(false);
            }
            else
            {
                assert(false); // unknown code
            }

        }

        __asm__ volatile ("csrc sip, 0x2");

        uint64 a;
        __asm__ volatile ("csrr %[name], sip" : [name] "=r"(a));
        putString("EVO:");
        putU64(a);
    }

    console_handler();
}

void debugInterrupt(int isExternal, int cause)
{
//    assert(false);
    if (isExternal) {
        if (cause == 1)
        {
//            putString("probably timer");
        } else if (cause == 9)
        {
//            putString("external hardware interrupt");
        }
    }
    else
    {
        if(cause == 2)
        {
            putString("illegal instruction");
        }
        else if(cause == 5)
        {
            putString("no permission for read to adress");
        }
        else if(cause == 7)
        {
            putString("no permission for write to adress");
        }
        else if(cause == 8)
        {
            putString("ecall from user mode");
        }
        else if(cause == 9)
        {
            putString("ecall from system mode wtf?");
//            assert(false);
        }
    }
    putNewline();
}
