# Easy QEMU VM Creator (eqemu)

A simple, terminal-based utility for creating and editing QEMU/KVM virtual machines using a friendly ncurses interface.

## Features

*   **Interactive TUI:** Built with ncurses for a straightforward, terminal-based experience.
*   **Create & Edit VMs:** Supports both the creation of new virtual machines and the editing of existing ones.
*   **Detailed Configuration:**
    *   Set basic parameters like VM name, vCPU count, and memory.
    *   Configure storage path and size.
    *   Choose between `q35` and `i440fx` chipsets.
    *   Select `UEFI` or `BIOS` firmware.
*   **Raw XML Editing:** For advanced configurations, you can press 'e' in the form to open the generated libvirt XML in your preferred text editor before defining the VM.
*   **Smart Editor Fallback:** Finds a text editor by searching for `$EDITOR`, then `$VISUAL`, and then common editors like `vi`, `nano`, and `vim`.
*   **KVM Fallback:** Automatically detects if KVM is available and accessible. If not, it falls back to the slower `qemu` software emulation and warns the user.

## Requirements

### Build-time Dependencies
*   A C99 compliant compiler (e.g., `gcc`, `clang`)
*   The `make` utility
*   `ncurses` development library (e.g., `libncurses-dev` on Debian/Ubuntu, `ncurses-devel` on Fedora/CentOS)
*   `libxml2` development library (e.g., `libxml2-dev` on Debian/Ubuntu, `libxml2-devel` on Fedora/CentOS)

### Runtime Dependencies
*   `qemu`
*   `libvirt` (for managing VMs via `virsh`)
*   A terminal emulator that supports ncurses.
*   A text editor for the raw XML editing feature.

## Building

To compile the program, simply run `make`:

```sh
make
```

## Running

Execute the compiled binary to start the application:

```sh
./eqemu
```
