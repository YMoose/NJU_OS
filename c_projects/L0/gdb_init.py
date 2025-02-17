"""
# Kill process (QEMU) on gdb exits
define hook-quit
  kill
end

define hook-stop
    printf "Program Counter:\n"
    x/i $rip + ($cs * 16)
    printf "---- Memory around 0x7c00 ----\n"
    x/16b 0x7c00
end

# Connect to remote
target remote localhost:1234
file a.out
watch *0x7c00
break *0x7c00
layout src
continue
"""
import gdb
from elftools.elf.elffile import ELFFile

binary_path='/home/yzh/NJU_os/c_projects/L0/build/hello-x86_64-qemu.elf'

# Kill process (QEMU) on gdb exits
def quit_handler(event):
    print("Killing process (QEMU)")
    gdb.execute("kill")
    
gdb.events.exited.connect(quit_handler)

# Define hook-stop
def stop_handler(event):
    print("Program Counter:")
    pc = int(gdb.parse_and_eval("$rip")) + (int(gdb.parse_and_eval("$cs")) * 16)
    gdb.execute("x/i " + str(pc))
    print("---- Memory around 0x7c00 ----")
    gdb.execute("x/16b 0x7c00")

gdb.events.stop.connect(stop_handler)

# Connect to remote
gdb.execute("target remote localhost:1234")
gdb.execute("watch *0x7c00")
gdb.execute("break *0x7c00")

# add symbol
with open(binary_path, 'rb') as file:
    elf_file = ELFFile(file)
    symbol_offset = 0
    # get offset of symbol from readelf 
    for section in elf_file.iter_sections():
        if section.name == '.text':
            symbol_offset = section['sh_addr']
            break
    gdb.execute("add-symbol-file "+binary_path+" "+str(symbol_offset))
    gdb.execute("break main")
    gdb.execute("break _start")
    
