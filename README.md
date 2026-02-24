<div align="center">

# GHOST-TALK
### **Secure. Ephemeral. Encrypted.**
*A Billionaire-Tier LAN Messaging Terminal for the Modern Age.*

[![C++](https://img.shields.io/badge/Language-C++17-00599C?style=for-the-badge&logo=c%2B%2B)](https://en.cppreference.com/w/cpp/17)
[![Raylib](https://img.shields.io/badge/GUI-Raylib_5.0-white?style=for-the-badge)](https://www.raylib.com/)
[![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Windows_%7C_macOS-lightgrey?style=for-the-badge)](https://github.com/CodeHashiraX/Ghost-Talk)

</div>

---

## Mission Brief

**Ghost-Talk** is not just a chat app; it is a **cyber-security statement**. 

Built from the ground up in C++ without relying on heavy frameworks, it offers a raw, high-performance communication channel for local networks. It bypasses the internet entirely, ensuring your data never touches a cloud server. 

With its custom **"Neural Float"** physics engine and **Matrix-style decryption visuals**, Ghost-Talk turns simple text messaging into a cinematic experience.

---

## Core Architecture: The Doubly Linked List

Unlike standard apps that use heavy `std::vector` containers, Ghost-Talk manages memory manually using a **Custom Doubly Linked List**. This demonstrates low-level memory mastery and O(1) efficiency.

### Why Linked List?

- **Dynamic Growth:** No need to reallocate/copy memory arrays when chat history grows.
- **Instant Deletion:** Removing a "Self-Destruct" node from the middle of the chain is **O(1)** (constant time), unlike O(N) for vectors.

### Implemented Operations

1. **Insertion (`AddLog`)**: New messages are appended to the `tail` pointer instantly.
2. **Traversal (`Forward/Backward`)**: The GUI traverses from `head` to `tail` to render messages. The new **"Infinite Scroll"** feature uses these pointers to offset the view.
3. **Surgical Deletion (`UpdateNodes`)**: The "Self-Destruct" logic identifies a specific node by its address, unlinks its `prev` and `next` pointers, and frees the memory block immediately without shifting the rest of the list.

---

## Key Capabilities

### The Ghost Protocol (Network Layer)

- **Zero-Trace Architecture:** Messages exist only in RAM. Closing the app triggers an immediate memory wipe.
- **XOR Encryption:** All packets are scrambled before leaving your machine, rendering them unreadable to packet sniffers.
- **LAN Auto-Discovery:** Instantly detects and displays your Local IP address.

### Physics-Enabled UI

- **Neural Float:** Messages don't sit still. They bob gently in a zero-gravity simulation (`sin(time)`), connected by dynamic neural links.
- **Projectile Data Flow:** Visualize network traffic as glowing "Data Orbs" fly physically from sender to receiver.
- **Matrix Decrypt:** Incoming messages appear as raw binary garbage (`#@!%`) before mathematically resolving into readable text.

### Self-Destruct Mode (Red Alert)

- **Ephemeral Nodes:** Activate the **[!]** toggle to send "Burn After Reading" messages.
- **Visual Warning:** Red-pulsing bubbles alert the recipient of the message's volatile nature.

---

## Installation: Things to Consider

### 1. The Audio Asset (Crucial)

- The application requires a sound file to run the audio engine.
- **Ensure:** You have a file named `ting.wav` inside an `assets` folder next to your executable (`Ghost-Talk/assets/ting.wav`).
- *Without this file, the app will run, but you will miss the auditory feedback.*

### 2. Network Firewalls

- Since this is a LAN application, Windows Firewall or macOS Security may ask for permission on the first launch.
- **Action:** Click **"Allow Access"** (Private Networks) to let the sockets communicate.
- **WiFi:** Both devices **MUST** be on the same WiFi network (e.g., Phone Hotspot or College WiFi).

---

## Compilation Guide

### macOS (M-Series / Intel)

*Prerequisites: Install Raylib via Homebrew (`brew install raylib`).*


### Windows (Visual Studio)

- **Setup:** Create an Empty C++ Project.
- **Dependencies:** Add raylib to your Include/Library directories.
- **Linker:** Add `raylib.lib` and `ws2_32.lib` (Winsock) to Input → Additional Dependencies.
- **Build:** Run in Release (x64) mode.

---

## Operational Manual

### Phase 1: Initialization

- **Identity:** Enter your Alias (e.g., Master AK).
- **Mode Selection:**
  - **HOST:** Creates a server on Port 9999.
  - **JOIN:** Connects to a Host IP.
  - **SANDBOX:** Offline testing mode to verify visuals.

### Phase 2: Secure Communication

- **Type & Send:** Messages flow instantly with projectile visuals.
- **Scroll:** Use Mouse Wheel or Up/Down keys to traverse history via the Linked List.
- **Self-Destruct:** Toggle the **[!]** button (Top Right). Messages sent now will vanish in 10s.

---

## Credits

- **Architect:** [Master AK](https://github.com/CodeHashiraX)
- **Engine:** Raylib 5.0
- **Concept:** DLL-Based Secure Messaging

---

<div align="center">

*"Privacy is not an option. It is the default."*

</div>
