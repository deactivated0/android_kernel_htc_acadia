# Acadia Kernel for HTC 10 evo / HTC Bolt (MSM8994)

[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](COPYING)
[![Android Version](https://img.shields.io/badge/Android-14%20(UpsideDownCake)-green.svg)](https://source.android.com)
[![Kernel Version](https://img.shields.io/badge/Kernel-3.10.84-orange.svg)](https://kernel.org)

## 1. Project Overview
This repository contains the production-grade Linux kernel source code for the **Acadia Android 14** operating system running on the **HTC 10 evo / HTC Bolt**.

The HTC 10 evo originally shipped with Android 7.0 (Nougat) powered by Linux kernel 3.10.84. This kernel has been heavily modernized, backported, and optimized to run Android 14 (AOSP / LineageOS 21) smoothly on 3 GB of RAM with strict SELinux compliance, modern storage subsystems, and reliable thermal and memory constraints.

---

## 2. Development History & Reverse Engineering Journey
> **Author's Statement**:
> "I asked HTC for the kernel source code and technical documentation, but they never answered.
>
> The entire kernel bringup had to be completely reverse engineered from scratch through exhaustive trial-and-error using AI assistance. I worked through every component, failure, and edge case until it was fully bugless.
>
> The absolute hardest part was getting the display to show anything at all. In the beginning, the screen only displayed glitched, distorted colors—flashing bright pink and bright green. After multiple rounds of debugging and trial-and-error adjustments, the AI managed to get a working picture. Immediately after, it showed the boot animation, which then bootlooped continuously.
>
> Through further reverse engineering of Android 14 binder interfaces, Bionic syscalls (`sys_renameat2`, `memfd_create`), netlink protocols, and cgroup freezer hooks, the bootloops were eliminated, transforming a dormant device into a daily-driver Android 14 system."

---

## 3. Supported Devices & Hardware
| Property | Value |
| :--- | :--- |
| **Marketing Name** | HTC 10 evo / HTC Bolt |
| **Codenames** | `htc_acauhl`, `aca`, `htc_aca`, `acauhl`, `htc_acaul` |
| **Platform** | Qualcomm Snapdragon 810 (MSM8994 v2.1) |
| **CPU Architecture** | ARMv8-A (64-bit octa-core: 4x Cortex-A57 + 4x Cortex-A53) |
| **GPU** | Adreno 430 |
| **RAM** | 3 GB LPDDR4 |
| **Storage** | 32 GB eMMC 5.1 |
| **Display** | 5.5" Super LCD3, 1440 × 2560 (QHD) |
| **Rear Camera** | 16 MP with Optical Image Stabilization (OIS), f/2.0 |
| **Front Camera** | 8 MP (OmniVision OV8858) |
| **Fingerprint** | Fingerprint Cards FPC1020 (TEE integration) |
| **USB Interface** | Type-C 2.0 with FUSB302 USB Power Delivery / OTG controller |

---

## 4. Key Features & Android 14 Backports

### Android 14 Platform Modernization
- **Syscall Backports**:
  - `sys_renameat2`: Backported for Android 14 Bionic filesystem operations and modern atomic directory swapping.
  - `memfd_create`: UAPI header and syscall support for secure anonymous memory sharing.
- **Binder IPC Modernization**:
  - Full 64-bit binder interface fixes and transaction leak prevention under Android 14 framework load.
- **Process Management & Freezer**:
  - Modern cgroup v1/v2 freezer hooks supporting Android 14 app lifecycle and cached app freezing.
- **Networking & Netlink**:
  - Backported `IFA_FLAGS` netlink attributes required by modern NetworkStack and Tethering mainline modules.
  - Legacy BPF compatibility shim support.

### Memory Optimization Suite (Reclaiming ~830 MB RAM)
- **`dm-bufio` Hash Table Shrink**:
  - Reduced `DM_BUFIO_HASH_BITS` from 20 to 11 in `drivers/md/dm-bufio.c`.
  - Reclaimed **163.9 MB** of kernel heap memory previously exhausted by ~20 APEX loop mount hash tables.
- **DVB Hardware Demuxer Removal**:
  - Stripped unused digital broadcast demuxers (`CONFIG_DVB_CORE`, `CONFIG_DVB_MPQ`) in defconfig, freeing **38.1 MB** of contiguous vmalloc space.
- **ION High-Order Page Pool Capping**:
  - Clamped high-order page pool allocations (order 9, 8, 4, 0 capped at 2, 2, 16, 128 elements) in `drivers/staging/android/ion/ion_page_pool.c`.
  - Prevents ION memory bloat after burst allocations, dropping ION idle usage to **7.6 MB** instead of 741 MB.
- **ZRAM & Swap Optimization**:
  - Configured 2.0 GB LZ4 ZRAM swap with optimized vm pressure and swappiness.

### Peripheral & Driver Fixes
- **USB Type-C & USB OTG**:
  - Patched `drivers/usb/fusb302/` and `drivers/usb/dwc3/` to ensure reliable OTG VBUS delivery and USB device enumeration.
- **Touchscreen Controllers**:
  - Verified dual-driver support for Synaptics DSX v2.6 and Himax 852xES capacitive panels.
- **Camera & Video**:
  - Memory-safe buffer management in MSM ISP AXI and Venus HFI firmware loading.
- **Display Pipeline**:
  - Configured for MDP5 hardware overlay composition with triple buffering to eliminate vertical and horizontal tearing.

---

## 5. Prerequisites & Toolchain
To build the kernel from source, the following host packages are required:
```bash
sudo pacman -S aarch64-linux-gnu-gcc make bc python bison flex libelf openssl
# Or on Ubuntu / Debian:
# sudo apt-get install gcc-aarch64-linux-gnu make bc python3 bison flex libelf-dev libssl-dev
```

> [!NOTE]
> ACA kernel compilation has been validated against GCC 16.1.0 and 16.2.0 cross-compilers (`aarch64-linux-gnu-gcc`).

---

## 6. Build Instructions

### Standalone Compilation
1. Export architecture and toolchain variables:
```bash
export ARCH=arm64
export SUBARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-
```

2. Load the Acadia defconfig:
```bash
make lineageos_aca_defconfig
```

3. Build the kernel Image and append Device Tree:
```bash
make -j$(nproc) Image.gz-dtb
```

### Building within the Acadia Tree
When using the complete Acadia Lineage 21 source tree:
```bash
cd /mnt/Games2/acadia-lineage21
source build/envsetup.sh
lunch acadia_aca-ap2a-userdebug
device/htc/aca/tools/build_acadia.sh bootimage
```
This script compiles the kernel, bundles the Android 14 ramdisk, appends verified DTBs, generates `boot.img`, and runs automated partition and init sanity checks.

---

## 7. Known Issues & Workarounds
- **Legacy 3.10 eBPF Restrictions**: Android 14 native eBPF requires Linux 4.9+ features. Acadia uses the custom legacy BPF compatibility layer (`ro.acadia.legacy_bpf=1`) for network accounting.

---

## 8. Licensing & Mandatory Attribution Requirements
This project is distributed under the terms of the **GNU General Public License Version 2 (GPLv2)** as published by the Free Software Foundation. See [COPYING](COPYING) for the full license text.

### Mandatory Attribution Notice:
Any developer, organization, or project that uses, forks, ports, or cherry-picks this kernel source code or its patches **MUST explicitly credit and mention**:
- **Author**: **deactivated0**
- **Project**: **Acadia Project (Android 14 for HTC 10 evo / HTC Bolt)**
- **Repository**: [https://github.com/deactivated0/android_kernel_htc_acadia](https://github.com/deactivated0/android_kernel_htc_acadia)

Attribution must be clearly maintained in your:
1. Public ROM release threads (e.g. XDA Developers, Telegram, websites).
2. Git commit history (preserving author metadata or `Co-authored-by: deactivated0 <deactivated0@users.noreply.github.com>`).
3. Release documentation and credits.
