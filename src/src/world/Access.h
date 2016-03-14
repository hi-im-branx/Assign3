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
#ifndef _Access_h_
#define _Access_h_

#include "runtime/SynchronizedArray.h"
#include "kernel/Output.h"
#include "devices/Keyboard.h"

#include <map>
#include <string>
#include <cerrno>
#include <unistd.h> // SEEK_SET, SEEK_CUR, SEEK_END

class Access : public SynchronizedElement {
public:
  virtual ~Access() {}
  virtual ssize_t pread(void *buf, size_t nbyte, off_t o) { return -EBADF; }
  virtual ssize_t pwrite(const void *buf, size_t nbyte, off_t o) { return -EBADF; }
  virtual ssize_t read(void *buf, size_t nbyte) { return -EBADF; }
  virtual ssize_t write(const void *buf, size_t nbyte) { return -EBADF; }
  virtual off_t lseek(off_t o, int whence) { return -EBADF; }
  static char memoryArray[100000]; // The array used to mimic memory for our filesystem
  static int veryLastCounter; // An index pointer to the very last block in the array in use
};

struct RamFile {
  vaddr vma;
  paddr pma;
  size_t size;
  RamFile(vaddr v, paddr p, size_t s) : vma(v), pma(p), size(s) {}
};

struct RamFile2 { // For assign 3
  vaddr vma;
  paddr pma;
  mutable int start; // The starting index of the file; needs to be changeable due to write() shifting positions
  int size; // The size of the file
  RamFile2(vaddr v, paddr p, int t, int s) : vma(v), pma(p), start(t), size(s) {}
};

extern map<string,RamFile> kernelFS;

extern map<string, RamFile2> kernelFS2; // For assign 3

class FileAccess : public Access {
  SpinLock olock;
  off_t offset;
  const RamFile &rf;
public:
  FileAccess(const RamFile& rf) : offset(0), rf(rf) {}
  virtual ssize_t pread(void *buf, size_t nbyte, off_t o);
  virtual ssize_t read(void *buf, size_t nbyte);
  virtual off_t lseek(off_t o, int whence);
};

class FileAccess2 : public Access {
  SpinLock olock;
  off_t offset;
  const RamFile2 &rf;
public:
  FileAccess2(const RamFile2& rf) : offset(rf.start), rf(rf) {} // The offset is initially set to the starting point of the file in the array
  virtual ssize_t read(char *buf, size_t nbyte);
  virtual ssize_t write(char a, size_t nbyte);

};

class KernelOutput;
class OutputAccess : public Access {
  KernelOutput& ko;
public:
  OutputAccess(KernelOutput& ko) : ko(ko) {}
  virtual ssize_t write(const void *buf, size_t nbyte) {
    return ko.write(buf, nbyte);
  }
};

extern Keyboard keyboard;
class InputAccess : public Access {
public:
  InputAccess() {}
  virtual ssize_t read(void *buf, size_t nbyte) {
    if (nbyte == 0) return 0;
    Keyboard::KeyCode k = keyboard.read();
    char* s = (char*)buf;
    for (size_t r = 0; r < nbyte; r += 1) {
      s[r] = k;
      if (!keyboard.tryRead(k)) return r+1;
    }
    return nbyte;
  }
};

#endif /* _Access_h_ */
