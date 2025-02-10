**🛠️ Custom Linux Container**
==============================

**📌 Overview**
---------------

**Custom Linux Container** is a **lightweight** and **minimal** container runtime written in **C**. It isolates processes and filesystems using **Linux namespaces**, `pivot_root()`, and **OverlayFS**, allowing secure execution of commands in a containerized environment.

**✨ Features**
--------------

✅ **Process Isolation** -- Uses `clone()` with new PID and mount namespaces.\
✅ **Filesystem Isolation** -- Implements `pivot_root()` to switch root filesystems.\
✅ **Overlay Filesystem** -- Creates a layered filesystem using OverlayFS.\
✅ **Mount Management** -- Sets up `/proc` and `/dev` inside the container.\
✅ **Lightweight Execution** -- Runs commands inside a minimal containerized environment.

**📂 Project Structure**
------------------------
- ├── 🏗️ Makefile             # Build script
- ├── 📖 README.md            # Project documentation
- ├── 🔄 change_root.c        # Implements pivot_root and mounts necessary filesystems
- ├── 📝 change_root.h        # Header file for change_root function
- ├── 🚀 container.c          # Main container execution logic`

**⚙️ Installation**
-------------------

### **📌 Prerequisites**

🔹 Linux-based OS (Ubuntu, Debian, etc.)\
🔹 `gcc` compiler\
🔹 `make` build tool\
🔹 **Root privileges** (for system calls like `pivot_root()`)

### **🛠️ Building the Project**

To compile the project, run:

`make`

This will generate the **`container`** executable. 🎉

**🚀 Usage**
------------

### **▶️ Running a Container**

To execute a container using an image and a command:

`sudo ./container <container_id> <image_name> <command> [args...]`

#### **💡 Example**

`sudo ./container test_container alpine /bin/sh`

### **🔍 How It Works**

1️⃣ **Creates an Overlay Filesystem**

-   `lowerdir` → `/images/<image_name>`
-   `upperdir` → `/tmp/container/<container_id>/upper`
-   `workdir` → `/tmp/container/<container_id>/work`
-   `merged` → `/tmp/container/<container_id>/merged`

2️⃣ **Changes Root Filesystem**

-   Uses `pivot_root()` to set the new root filesystem.

3️⃣ **Executes the Given Command**

-   Runs `/bin/sh` or any specified command inside the isolated environment.

**🔍 Implementation Details**
-----------------------------

### **📌 change_root.c**

🛑 Unmounts `/proc` before switching root.\
🔄 Uses `pivot_root()` to move the root filesystem.\
📌 Mounts `/dev` and `/proc` inside the new root.\
🛠️ Sets up `PATH` for command execution.

### **📌 container.c**

📂 Initializes an overlay filesystem for the container.\
👶 Uses `clone()` to create a new isolated process.\
🔄 Calls `change_root()` to set the new environment.\
▶️ Runs the provided command using `execvp()`.

**🧹 Cleanup**
--------------

`rm -rf /tmp/container`

**🔮 Future Enhancements**
--------------------------

🚀 Implement networking with `CLONE_NEWNET`\
📊 Add resource limits using `cgroups`\
🔍 Improve logging and debugging tools\
♻️ Support container checkpointing and restoration

**📝 License**
--------------

This project is open-source and provided under the **MIT License**.
