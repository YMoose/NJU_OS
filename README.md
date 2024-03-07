# notes
## 实验MISC
### QEMU
`qemu-system-x86_64 -S -s -serial mon:stdio -nographic -drive  format=raw,file=build/hello-x86_64-qemu -vnc :1`
### vscode调试
launch.json配置
```json
{
            "name": "(gdb) qemu",
            "type": "cppdbg",
            "request": "launch",
            "miDebuggerServerAddress": "localhost:1234",
            "program": "<path_to_elf>",
            "args": [],
            "stopAtEntry": true,
            "cwd": "${fileDirname}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
            ]
},
```