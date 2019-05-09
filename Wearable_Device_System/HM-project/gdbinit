#set architecture i386:x86-64:intel #descomentar em caso de gdb normal
target remote :3333
mon reset halt
flushregs
#mon program_esp32 "/home/pi/eclipse-workspace/HM/build/adc.bin" 0x10000 verify
thb app_main
c
