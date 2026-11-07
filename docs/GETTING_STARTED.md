# Getting Started: From Zero to Your Code Running on the Board

This guide takes you from "I've never coded" to "my code is running on the
flight computer and waiting to be merged". You don't need to know anything
before you start. Go step by step and don't skip ahead. Each step checks that
the one before it worked.

**By the end of Sunday's meeting you will be able to:**

1. Download (clone) the project onto your laptop
2. Change some code
3. Compile it (turn it into a program the board can run)
4. Flash it onto the STM32 board and see it run
5. Read messages the board sends back to your laptop
6. Send (push) your change to GitHub so it can be reviewed and merged

> **Do Steps 1–3 at home before Sunday if you can.** The installs are big
> downloads and take a while. If you get stuck, stop and bring your laptop.
> We'll fix it together.

---

## Words you'll hear

| Word | What it means |
|---|---|
| **Repository (repo)** | The project folder, stored on GitHub. Ours is `DSRT_Avionics`. |
| **Clone** | Download a copy of the repo onto your laptop. |
| **Commit** | Save a snapshot of your changes, with a short message saying what you did. |
| **Branch** | Your own copy of the code to work on, so you don't break anyone else's. |
| **Push** | Upload your commits from your laptop to GitHub. |
| **Pull** | Download other people's new commits from GitHub to your laptop. |
| **Pull request (PR)** | Asking the team lead to review your branch and merge it into the main code. |
| **Merge** | Combine your changes into the main code. The team lead does this. |
| **Compile / build** | Turn the C code we write into a file the chip can run (`.elf`). |
| **Flash** | Copy that file onto the STM32 chip. |
| **UART / serial** | A simple cable connection the board uses to send text back to your laptop. |
| **Nucleo** | The STM32 development board we use (NUCLEO-F446RE). |

---

## Step 1: Make a GitHub account and get invited

1. Go to <https://github.com> and click **Sign up**. Pick a username you're
   happy for the team to see.
2. Send your **GitHub username** to the team lead.
3. You'll get an email invitation to the `DSRT_Avionics` repository. Click
   **Accept invitation**. Until you do, you won't be able to upload (push) code.

✅ **Check:** You can open <https://github.com/justjavi25/DSRT_Avionics> and
see the files.

---

## Step 2: Install the tools

You need five things. Install them in this order.

### 2a. Git (keeps track of code changes)

- **Windows:** Download from <https://git-scm.com/download/win> and run the
  installer. Clicking **Next** on every screen is fine.
- **Mac:** Open the **Terminal** app (press `Cmd + Space` and type
  *Terminal*), then run:
  ```
  xcode-select --install
  ```
  Click **Install** on the pop-up. If it says it's already installed, that's
  fine.

### 2b. Visual Studio Code (where you write code)

Download from <https://code.visualstudio.com> and install it.

### 2c. STM32CubeCLT (the compiler and build tools)

This contains the compiler (`arm-none-eabi-gcc`) plus the build tools
`cmake` and `ninja`.

1. Go to <https://www.st.com/en/development-tools/stm32cubeclt.html>.
2. Click **Get Software** and pick your system (Windows or macOS). ST asks you
   to make a free **myST** account before downloading.
3. Run the installer and accept the defaults.

### 2d. STM32CubeProgrammer (puts your program on the board)

1. Go to <https://www.st.com/en/development-tools/stm32cubeprog.html>.
2. Click **Get Software**, pick your system, and install it.
3. **Windows:** If it offers to install the **ST-LINK USB driver**, say yes.

### 2e. VS Code extensions

Open VS Code. Click the **Extensions** icon in the left sidebar (four little
squares), search for each of these, and click **Install**:

| Extension | Made by | What it's for |
|---|---|---|
| **C/C++** | Microsoft | Colours and helps with C code |
| **CMake Tools** | Microsoft | Understands our build setup |
| **Serial Monitor** | Microsoft | Shows the messages the board sends back |

### ✅ Check that everything installed

**Close VS Code completely and open it again**, so it picks up the new tools.
Open a terminal inside VS Code with **Terminal → New Terminal** (top menu) and
run these one at a time:

```
git --version
arm-none-eabi-gcc --version
cmake --version
ninja --version
```

Each one should print a version number. If one says **"not found"** or
**"not recognized"**:

- First, restart your laptop and try again.
- **Mac, `arm-none-eabi-gcc` still not found:** CubeCLT may not have added
  itself to your PATH. Run `ls /opt/ST` and you should see a folder like
  `STM32CubeCLT_1.xx.x`. Ask the team lead to help you add it to your PATH.
  The other option is the manual install the team lead used: Homebrew for
  `cmake` and `ninja`, plus Arm's toolchain download. Make sure you get the
  **`arm-none-eabi`** version. The `aarch64-none-elf` one is the wrong chip
  family and will not work.
- **Still stuck?** Write down the exact error and bring it on Sunday.

---

## Step 3: Download (clone) the project

### First time only: tell Git who you are

In the VS Code terminal, run these two commands with your own name and the
email address you used for GitHub:

```
git config --global user.name "Your Name"
git config --global user.email "you@example.com"
```

### Clone the repo

1. In VS Code press **`Ctrl + Shift + P`** (Windows) or **`Cmd + Shift + P`**
   (Mac). This opens the *Command Palette*.
2. Type **`Git: Clone`** and press Enter.
3. Paste this address and press Enter:
   ```
   https://github.com/justjavi25/DSRT_Avionics.git
   ```
4. Pick a folder to keep it in, for example **Documents**.
5. When VS Code asks **"Would you like to open the cloned repository?"**,
   click **Open**.
6. If VS Code asks you to **sign in to GitHub**, click **Allow** and sign in
   in your browser. This is how VS Code gets permission to upload your work
   later.

If VS Code asks whether you **trust the authors** of the folder, click
**Yes**.

### Already cloned it before?

Get the newest version instead of cloning again. In the VS Code terminal:

```
git checkout main
git pull
```

✅ **Check:** The **Explorer** panel on the left (the top icon) shows folders
called `avionics_interface_v0` and `docs`.

---

## Step 4: Make your own branch

**Never work directly on `main`.** `main` is the code everyone shares. Make
your own branch for each thing you work on.

In the VS Code terminal, run this, using your own name and a few words about
what you're doing:

```
git checkout -b yourname/what-you-are-doing
```

For example: `git checkout -b sam/first-pull-request`

✅ **Check:** The bottom-left corner of VS Code now shows your branch name
instead of `main`.

---

## Step 5: Build (compile) the code

In the VS Code terminal, go into the firmware folder:

```
cd avionics_interface_v0
```

**The first time only**, set up the build:

```
cmake --preset Debug
```

**Every time you change code**, build it:

```
cmake --build --preset Debug
```

✅ **Check:** The last lines look like this:

```
[34/34] Linking C executable avionics_interface_v0.elf
             RAM:        3696 B       128 KB      2.82%
           FLASH:       51316 B       512 KB      9.79%
```

Your program is now in `avionics_interface_v0/build/Debug/avionics_interface_v0.elf`.

**If it fails:** scroll up to the **first** line that says `error:`. It tells
you the file and line number, like `App/Src/avionics.c:42: error: ...`. That's
almost always the line you just changed. A missing `;` is the most common
mistake.

> If the **CMake Tools** extension pops up asking you to pick a *configure
> preset*, choose **Debug**. You can always just use the terminal commands
> above instead.

---

## Step 6: Flash the program onto the board

> ⚠️ **Radio safety:** If the RFM95 radio board is wired up, its **antenna must
> be attached** before you power the board. It transmits as soon as the
> program starts, and transmitting without an antenna can damage it.

1. Plug the Nucleo board into your laptop with the USB cable.
2. Open **STM32CubeProgrammer**.
3. On the right side, make sure the dropdown says **ST-LINK**, then click the
   green **Connect** button. The status should change to *Connected*.
4. Click the **Erasing & programming** icon in the left sidebar (the second
   one down, an arrow pointing into a chip).
5. Next to **File path**, click **Browse** and choose
   `avionics_interface_v0/build/Debug/avionics_interface_v0.elf`.
6. Tick **Verify programming** and **Run after programming**.
7. Click **Start Programming**. Wait for *"File download complete"*.
8. Click **Disconnect** (top right).

**To flash again after changing code:** build again (Step 5), then
**Connect → Start Programming → Disconnect**. CubeProgrammer remembers the
file.

**If Connect fails:**
- Try another USB cable. Some cables only charge and can't send data.
- If it says the ST-LINK firmware is old, click **Firmware upgrade**, run it,
  then unplug and replug the board.

---

## Step 7: See messages from the board (UART)

The board prints messages over the same USB cable you used to flash it.

1. In VS Code, open the bottom panel with **View → Terminal**, then click the
   **SERIAL MONITOR** tab.
2. **Port:** choose the one that mentions **STMicroelectronics** or
   **STLink**. On Windows it's called `COM3`, `COM4` or similar. On a Mac it
   looks like `/dev/tty.usbmodem...`.
3. **Baud rate:** **115200**.
4. Click **Start Monitoring**.
5. Press the **black RESET button** on the Nucleo board.

✅ **Check:** You should see something like this:

```
DSRT avionics booting (takes a few seconds)...

=== DSRT avionics started ===
  BMP388 barometer : OK
  BNO085 IMU       : OK
  RFM95 radio      : OK
  SD card          : not found
  status byte      : 0x07

[    3.004 s]  baro 101325 Pa  alt 0.03 m  23.4 C  |  accel 0.12 -0.05 9.79 m/s2  gyro 0.00 0.01 -0.00 rad/s  |  radio 11 sent
[    4.004 s]  baro 101324 Pa  alt 0.04 m  23.4 C  |  accel 0.11 -0.04 9.80 m/s2  gyro 0.01 0.00 0.00 rad/s  |  radio 15 sent
```

- The first block lists which parts started. **FAILED** means that sensor
  isn't wired right or isn't plugged in. Check its pins against the wiring
  table in the main [README](../README.md).
- After that, one line of readings appears every second. Tilt the board and
  watch the `accel` numbers change.
- **"SD card: not found"** is normal if there's no card in the SD board.

**Nothing showing?** Check that you picked the right port, that the baud rate
is 115200, and press RESET again. **Garbage characters** almost always mean
the baud rate is wrong.

### Printing your own messages

Anywhere in the code you can print to this window with `printf`. Add
`#include <stdio.h>` at the top of the file if it isn't there already:

```c
printf("Hello from my code! x = %d\r\n", x);
```

End each message with `\r\n` so it starts a new line. Don't print in a loop
that runs thousands of times a second: each character takes time to send and
slows the program down.

---

## Step 8: Practice: your first change

Let's change something, build it, flash it, and see it on the board.

1. In the Explorer, open `avionics_interface_v0/App/Src/avionics.c`.
2. Find this line (press `Ctrl + F` / `Cmd + F` and search for `started`):
   ```c
   printf("\r\n=== DSRT avionics started ===\r\n");
   ```
3. Add a line right after it with your name:
   ```c
   printf("Hello from Sam!\r\n");
   ```
4. Save the file (`Ctrl + S` / `Cmd + S`).
5. Build (Step 5), flash (Step 6), and press RESET with the Serial Monitor
   open (Step 7).

✅ **Check:** Your hello message appears under the banner. **You just changed
the flight computer's code!** 🎉

**Now undo that change.** If everyone pushed a hello line in the same spot,
the changes would clash when merging. To undo it, run this in the terminal
(from inside `avionics_interface_v0`):

```
git checkout -- App/Src/avionics.c
```

---

## Step 9: Send your work for review (commit, push, pull request)

For your first pull request, add a small file about yourself:

1. In the Explorer, right-click the `team` folder → **New File** → name it
   `your-github-username.md`, for example `team/sam-codes.md`.
2. Type a few lines in it: your name, and what you'd like to work on (sensors,
   radio, data logging, ...). Save it.

Each person makes their own file, so nobody's changes clash with anybody
else's. When you work on real code tasks later, you follow exactly the same
steps.

### Commit (save a snapshot)

1. Click the **Source Control** icon in the left sidebar (it looks like a
   branch: three dots joined by lines).
2. You'll see your changed files listed. Hover over **Changes** and click the
   **+** to *stage* them, which means choosing what goes in this snapshot.
3. Type a short message in the box at the top saying what you did, for
   example `Add my team file`.
4. Click **Commit**.

### Push (upload to GitHub)

Click **Publish Branch** (or **Sync Changes** if you've pushed this branch
before). If VS Code asks you to sign in to GitHub, do it.

<details>
<summary>Prefer the terminal? Same steps as commands</summary>

```
git add team/your-github-username.md
git commit -m "Add my team file"
git push -u origin yourname/what-you-are-doing
```

`git add .` adds every changed file. Run `git status` first to see what
that includes.
</details>

### Open a pull request

1. Go to <https://github.com/justjavi25/DSRT_Avionics>.
2. A yellow banner says your branch had recent pushes. Click
   **Compare & pull request**.
3. Write:
   - **What you changed**
   - **How you tested it**, for example *"Built and flashed; the serial
     monitor shows all sensors OK"*
4. Click **Create pull request**.

✅ **Check:** Your pull request appears under the repo's **Pull requests**
tab. The team lead will review it and either merge it or leave comments.

**If the team lead asks for changes:** make them on the **same branch**,
then commit and push again. The pull request updates by itself. You don't
need to open a new one.

---

## Starting your next task

Every time you start something new:

```
git checkout main
git pull
git checkout -b yourname/next-thing
```

This gets everyone's latest merged code and gives you a fresh branch.

## Team rules

- 🚫 **Never push to `main`.** Always use a branch and a pull request.
- 🌱 **One branch per task.** Keep each pull request small and about one thing.
- 🔨 **Build and test before you push.** Say how you tested in the pull request.
- ⬇️ **`git pull` on `main` before starting new work.**
- 📁 The `build/` folder is never uploaded. Git ignores it automatically.

---

## Troubleshooting

| Problem | Fix |
|---|---|
| `git` / `cmake` / `arm-none-eabi-gcc`: *"not recognized"* or *"command not found"* | Restart VS Code (or your laptop) after installing. Then see the checks in Step 2. |
| `cmake --preset Debug` says it can't find `arm-none-eabi-gcc` | The compiler isn't on your PATH (Step 2c). After fixing it, delete the `build` folder and run the command again. |
| Build fails with `error:` | Go to the file and line it names. Look for a missing `;`, `)` or `}`. |
| CubeProgrammer: *"No ST-LINK detected"* | Try another USB cable. Check the two jumpers labelled CN2 are on the board. Windows: install the ST-LINK driver. |
| Serial Monitor shows nothing | Pick the STMicroelectronics/STLink port, set 115200 baud, press RESET. |
| Serial Monitor shows garbage | The baud rate isn't 115200. |
| A sensor says **FAILED** | Check its wires against the README wiring table, then press RESET. |
| Push says *"permission denied"* or *"403"* | Accept the GitHub invitation (Step 1) and sign in to GitHub in VS Code. |
| Push says *"rejected"* / *"fetch first"* | Someone else pushed to your branch. Run `git pull`, then push again. |
| Accidentally committed on `main` | Don't push. Ask the team lead. It's easy to move your commit to a branch. |

---

## Sunday checklist

- [ ] GitHub account made and repo invitation accepted
- [ ] Git, VS Code, STM32CubeCLT and STM32CubeProgrammer installed
- [ ] VS Code extensions installed (C/C++, CMake Tools, Serial Monitor)
- [ ] All four version checks in Step 2 print a version
- [ ] Repo cloned and opened in VS Code
- [ ] Code builds (`cmake --build --preset Debug`)
- [ ] Program flashed with STM32CubeProgrammer
- [ ] Startup messages visible in the Serial Monitor
- [ ] Practice change made, seen on the board, then undone
- [ ] Team file committed, pushed, and a pull request opened
