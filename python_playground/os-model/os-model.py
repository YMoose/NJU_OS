#!/usr/bin/env python3

import sys
import random
from pathlib import Path

class OperatingSystem():
    """A minimal executable operating system model."""

    SYSCALLS = ['choose', 'write', 'spawn', 'sched']

    class Thread:
        """A "freezed" thread state."""

        def __init__(self, func, *args):
            self._func = func(*args)
            self.retval = None

        def step(self):
            """Proceed with the thread until its next trap."""
            syscall, args, *_ = self._func.send(self.retval)
            self.retval = None
            return syscall, args

    def __init__(self, src):
        variables = {}
        exec(src, variables)
        self._main = variables['main']

    def run(self):
        threads = [OperatingSystem.Thread(self._main)]
        while threads:  # Any thread lives
            try:
                next_op = (t := threads[0]).step()
                if next_op[0] == 'choose': # case 'choose', xs:  # Return a random choice
                    t.retval = random.choice(next_op[1])
                    continue
                if next_op[0] == 'write':  # Write to debug console
                    print(next_op[1], end='')
                    continue
                if next_op[0] == 'spawn':  # Spawn a new thread
                    fn, args = next_op[1]
                    threads += [OperatingSystem.Thread(fn, *args)]
                    continue
                if next_op[0] =='sched':  # Non-deterministic schedule
                    random.shuffle(threads)
                    continue
            except StopIteration:  # A thread terminates
                threads.remove(t)
                random.shuffle(threads)  # sys_sched()

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(f'Usage: {sys.argv[0]} file')
        exit(1)

    src = Path(sys.argv[1]).read_text()
    for syscall in OperatingSystem.SYSCALLS:
        src = src.replace(f'sys_{syscall}',        # sys_write(...)
                          f'yield "{syscall}", ')  #  -> yield 'write', (...)

    OperatingSystem(src).run()