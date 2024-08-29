# OpenSBI Artifact

This repository automates the build of [OpenSBI](https://github.com/riscv-software-src/opensbi/) for the purpose of generating artifacts that can be use for integration tests in the Miralis project.

## How to release new artifacts

Create a new tag and push it to upstream, e.g.:

```sh
git tag v0.1.0
git push origin v0.1.0
```

## Binaries

### opensbi only:
`opensbi`: simple opensbi with sample firmware.

`opensbi_jump`: simple opensbi firmware that jump at adresse 0x80400000.

### opensbi with linux kernel as a payload:

`opensbi-linux-kernel-exit`: It simply exits after booting.

`opensbi-linux-kernel-shell`: It opens a shell after booting.

`opensbi-linux-kernel-driver`: It inserts an out-of-tree module (`driver.ko`) after booting. For now, it does an ecall to Miralis with the benchmark code to print data and exit.