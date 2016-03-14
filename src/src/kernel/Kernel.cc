/******************************************************************************
    Copyright � 2012-2015 Martin Karsten

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/
#include "runtime/Thread.h"
#include "kernel/AddressSpace.h"
#include "kernel/Clock.h"
#include "kernel/Output.h"
#include "world/Access.h"
#include "machine/Machine.h"
#include "devices/Keyboard.h"
#include "main/UserMain.h"
#include <array>

AddressSpace kernelSpace(true); // AddressSpace.h
volatile mword Clock::tick;     // Clock.h

extern Keyboard keyboard;

#if TESTING_KEYCODE_LOOP
static void keybLoop() {
  for (;;) {
    Keyboard::KeyCode c = keyboard.read();
    StdErr.print(' ', FmtHex(c));
  }
}
#endif

void kosMain() {
  KOUT::outl("Welcome to KOS!", kendl);
  auto iter = kernelFS.find("motb");
  if (iter == kernelFS.end()) {
    KOUT::outl("motb information not found");
  } else {
    FileAccess f(iter->second);
    for (;;) {
      char c;
      if (f.read(&c, 1) == 0) break;
      KOUT::out1(c);
    }
    KOUT::outl();

 }

  auto iter2 = kernelFS2.find("filesystem_test"); // Testing our filesystems read()
  if (iter2 == kernelFS2.end()) {
    KOUT::outl("Couldnt find the file!");
  }  else {
    FileAccess2 f(iter2->second);
    for (;;) {
      char c; // Read will put the character from our filesystem into &c
     if (f.read(&c, 1) == 0) break; // If read returns 0 then we break out of the for loop
     KOUT::out1(c); // Otherwise we print the character and loop
    }
  }

    auto iter5 = kernelFS2.find("filesystem_test"); // Testing our filesystems write()
    if (iter5 == kernelFS2.end()) {
      KOUT::outl("Couldnt find the file!");
    }  else {
    char charString[] = " My filesystem works !"; // The test string to write to our filesystem
    ssize_t useless;
    FileAccess2 f(iter5->second);
    for (int i=0;i<sizeof(charString);i++) { // Loop for the length of the input string
      char c = charString[i];
      useless = f.write(c, 1); // Write the character to our filesystem; returns a uselesss value
    }
  }

    auto iter4 = kernelFS2.find("filesystem_test"); // Reading the file to make sure write() worked properly
    if (iter4 == kernelFS2.end()) {
      KOUT::outl("Couldnt find the file!");
    }  else {
      FileAccess2 f(iter4->second);
      for (;;) {
        char c;
       if (f.read(&c, 1) == 0) break;
       KOUT::out1(c);
      }
}


#if TESTING_TIMER_TEST
  StdErr.print(" timer test, 3 secs...");
  for (int i = 0; i < 3; i++) {
    Timeout::sleep(Clock::now() + 1000);
    StdErr.print(' ', i+1);
  }
  StdErr.print(" done.", kendl);
#endif
#if TESTING_KEYCODE_LOOP
  Thread* t = Thread::create()->setPriority(topPriority);
  Machine::setAffinity(*t, 0);
  t->start((ptr_t)keybLoop);
#endif
  Thread::create()->start((ptr_t)UserMain);
#if TESTING_PING_LOOP
  for (;;) {
    Timeout::sleep(Clock::now() + 1000);
    KOUT::outl("...ping...");
  }
#endif
}

extern "C" void kmain(mword magic, mword addr, mword idx)         __section(".boot.text");
extern "C" void kmain(mword magic, mword addr, mword idx) {
  if (magic == 0 && addr == 0xE85250D6) {
    // low-level machine-dependent initialization on AP
    Machine::initAP(idx);
  } else {
    // low-level machine-dependent initialization on BSP -> starts kosMain
    Machine::initBSP(magic, addr, idx);
  }
}
