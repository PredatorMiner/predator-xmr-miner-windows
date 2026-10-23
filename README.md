# Predator: RandomX Monero (XMR) CPU Miner for Windows

**Tiny (~86 KB), AVX2-optimized RandomX miner for Monero. A fast XMRig alternative for Windows x64.**

![License](https://img.shields.io/badge/license-GPL--3.0-blue)
![Platform](https://img.shields.io/badge/platform-Windows%20x64-0078D6)
![Algorithm](https://img.shields.io/badge/algorithm-RandomX-ff6600?logo=monero&logoColor=white)
![Coin](https://img.shields.io/badge/coin-Monero%20(XMR)-ff6600?logo=monero&logoColor=white)
![ISA](https://img.shields.io/badge/requires-AVX2%20%2B%20AES--NI-orange)
![Size](https://img.shields.io/badge/binary-~86%20KB-brightgreen)
![RAM](https://img.shields.io/badge/RAM-~2.5%20GB%20(Full%20mode)-informational)
![Fee](https://img.shields.io/badge/dev%20fee-1%25-yellow)
![Compiler](https://img.shields.io/badge/built%20with-clang--cl%2020.1.8-262D3A?logo=llvm&logoColor=white)
![Language](https://img.shields.io/badge/language-C%2B%2B%20%2B%20MASM-00599C?logo=cplusplus&logoColor=white)
[![Release](https://img.shields.io/github/v/release/PredatorMiner/predator-xmr-miner-windows?include_prereleases&display_name=tag&label=release)](https://github.com/PredatorMiner/predator-xmr-miner-windows/releases/latest)
![Downloads](https://img.shields.io/github/downloads/PredatorMiner/predator-xmr-miner-windows/total)
![Stars](https://img.shields.io/github/stars/PredatorMiner/predator-xmr-miner-windows?style=flat)

Predator is an open-source RandomX miner written from scratch for one coin and one platform. It mines **1-1.5% faster than XMRig** on the same hardware, thread count and settings, in a single executable with no installer, no config files and no CRT dependency.

## Features

- 1-1.5% higher hashrate than XMRig in our tests
- One ~86 KB executable, four command-line options
- Hand-tuned hot path in C++ and x86-64 assembly (AVX2, AES-NI, BMI2, ADX, FMA)
- Tuned for Intel Core (Alder Lake / Raptor Lake) and AMD Ryzen (Zen 2 / 3 / 4)
- RandomX Full mode only: no code wasted on the slow Light mode
- Minimal console output: connection, shares, hashrate
- Threads clamped to your logical CPU count, process priority from idle to realtime
- Optional WinRing0 support for CPU register (MSR) access
- Not an XMRig fork: no legacy code
- Open source (GPL-3.0), 1% dev fee implemented in plain sight

## Requirements

| | |
|---|---|
| OS | Windows 10 / 11 x64 |
| CPU | AVX2 and AES-NI |
| RAM | ~2.5 GB free |

## Download and run

1. Download the latest archive from [Releases](https://github.com/PredatorMiner/predator-xmr-miner-windows/releases) and unpack it:
   ```
   Predator/
   ├── Predator.exe
   └── StartExample.bat
   ```
2. Edit `StartExample.bat`, set your pool and wallet:
   ```
   @echo off
   cd /d "%~dp0"
   Predator.exe -o pool.example.com:3333 -u <your_xmr_wallet> -p 3
   pause
   ```
3. Run it. You will see the pool connection, then accepted shares and your hashrate. Close the window to stop.

The `cd /d "%~dp0"` line is required: scripts run as Administrator start in `C:\Windows\System32` and would not find `Predator.exe`.

## Command-line options

| Option | Description |
|---|---|
| `-o, --pool <host:port>` | Monero pool address and port |
| `-u, --user <wallet>` | Your XMR wallet address (or `wallet.worker`, if your pool supports it) |
| `-t, --threads <N>` | Mining threads, clamped to `1..logicalCPUs` |
| `-p, --priority <0-5>` | `0` idle, `1` below normal, `2` normal, `3` above normal, `4` high, `5` realtime. Start with `2` or `3`: `5` can make Windows unresponsive |

## WinRing0 (optional)

Predator can use the third-party [WinRing0](https://github.com/xmrig/xmrig/blob/master/bin/WinRing0/WinRing0x64.sys) driver (`WinRing0x64.sys`) for CPU model-specific register (MSR) access. This repository and its releases **do not include** the driver. To use it, open the link above and click **Download raw file**, place `WinRing0x64.sys` next to `Predator.exe` and run Predator as Administrator. Without the driver Predator works as usual.

- Versions of WinRing0 have a known vulnerability (CVE-2020-14979). Microsoft Defender detects the driver as `VulnerableDriver:WinNT/Winring0` and may quarantine it.
- Windows can refuse to load it when Memory Integrity (HVCI) or the vulnerable driver blocklist is enabled.

## Dev fee

1% of mining time goes to the developer's address. It is the only funding of the project, and it is implemented in the open source code: nothing is hidden or obfuscated.

## Why Predator is fast

- **No layers in the hot loop.** Scratchpad fill, VM execution and dataset reads run without virtual calls, exceptions, RTTI or heap allocations.
- **Smarter prefetch.** Write-only scratchpad fill loops use `prefetchw`, so cache lines that will only be written are not read first.
- **Register-level tuning.** `blake2b_compress` was traced down to single registers to remove redundant saves and restores.
- **Fixed C++/ASM layout.** The VM state uses fixed offsets shared by C++ and MASM, audited on every change.
- **Branchless inner loops.** `cmov` and AVX2 blends instead of data-dependent branches.
- **No false sharing.** Shared state and atomics are aligned to 64 bytes.
- **Lean build.** clang-cl 20.1.8, `-O3`, full LTO, no exceptions, RTTI or unwind tables.

Share validity always comes before speed: every change is checked against live pools.

## Limitations

Light mode, CPUs without AVX2 or AES-NI, Linux, macOS, ARM, GPU mining, other coins or algorithms, and NUMA or server tuning are not supported and not planned.

## Build from source

Requirements: Windows 10/11 x64, Visual Studio 2026 with the LLVM / clang-cl toolset (20.1.8).

1. Open `Predator.sln`
2. Select **Release | x64**
3. Build the solution

Linker flags: `/ENTRY:mainCRTStartup /OPT:ICF=10 /OPT:REF`. The build uses `-march=native`, so build on a CPU of the same class as the one that will run it.

## FAQ

**Is Predator faster than XMRig?**
By 1-1.5% in our tests on the same hardware, thread count and settings.

**Does Predator support Light mode or old CPUs?**
No. It needs AVX2 and AES-NI and runs RandomX in Full mode only.

**Antivirus flags Predator. Is it malware?**
Miners are flagged by heuristics no matter who wrote them. Predator is open source: read the code, build it yourself and compare checksums with the release.

**It says `Predator.exe` is not found when run as Administrator.**
Use the `StartExample.bat` from the release, or add `cd /d "%~dp0"` as the second line of your own script.

**The window closes immediately or shows an error.**
Check that your CPU supports AVX2 and AES-NI, that ~2.5 GB of RAM is free, and that the pool address and port are correct.

**Shares are rejected.**
Check the pool address and wallet. If both are correct, open an issue with your CPU model and pool.

## License

GNU General Public License v3.0, see [LICENSE](LICENSE). RandomX is designed by tevador and contributors and released under the BSD 3-Clause license: https://github.com/tevador/RandomX

The "Predator" name and branding are not covered by the license: forks that change or remove the dev fee should use a different name.

Mine only on hardware you own or have permission to use. The software is provided as is, without warranty. Mining keeps the CPU at full load, so make sure cooling and power are adequate.
