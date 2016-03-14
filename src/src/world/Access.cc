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
#include "world/Access.h"
#include "kernel/Multiboot.h"
#include <iostream>
#include <fstream>

#include <cstring>

map<string,RamFile> kernelFS;
map<string, RamFile2> kernelFS2;

char Access::memoryArray[];
int Access::veryLastCounter;

ssize_t FileAccess::pread(void *buf, size_t nbyte, off_t o) {
  if (o + nbyte > rf.size) nbyte = rf.size - o;
  memcpy( buf, (bufptr_t)(rf.vma + o), nbyte );
  return nbyte;
}

ssize_t FileAccess::read(void *buf, size_t nbyte) {
  olock.acquire();
  ssize_t len = pread(buf, nbyte, offset);
  if (len >= 0) offset += len;
  olock.release();
  return len;
}

off_t FileAccess::lseek(off_t o, int whence) {
  off_t new_o;
  switch (whence) {
    case SEEK_SET: new_o = o; break;
    case SEEK_CUR: new_o = offset + o; break;
    case SEEK_END: new_o = rf.size + o; break;
    default: return -EINVAL;
  }
  if (new_o < 0) return -EINVAL;
  offset = new_o;
  return offset;
}

ssize_t FileAccess2::read(char *buf, size_t nbyte) {
 int i;
 char d;
 int counter = 0;
 for (i=0; i<nbyte; i++){
   d = Access::memoryArray[FileAccess2::offset]; // Set d to the char in the index of FileAccess2's offset
   *(buf + counter) = d; // Put d into the buffer + counter's memory address pointer
   FileAccess2::offset++; // Increment the offset after each byte read
   counter++; // Likewise with the counter
 }
  if (d == '\0'){ // If the end of file is read, we terminate (return length of 0)
    nbyte = 0;
  }
  return nbyte; // Otherwise we return the length
}

ssize_t FileAccess2::write(char a, size_t nbyte) {
  int startR = rf.start;
  void* pttr;
  char* charPttr;
  pttr = charPttr;
  if (startR != Access::veryLastCounter) { // Case 1: We haven't deleted the original file
    for (int i=0; i < rf.size; i++){ // For every block in the file we set it to null
      Access::memoryArray[startR + i] = '\0';
      pttr = &Access::memoryArray[startR + i];
      memcpy((bufptr_t)(rf.vma + offset + i), pttr, 1); // Likewise with the virtual memory address
    }
    startR = Access::veryLastCounter;
    Access::memoryArray[startR] = a; // Write the char into the next available space (aka +1 of the very last block in the array)
    Access::veryLastCounter++; // Increment veryLastCounter
    rf.start = Access::veryLastCounter; // Set the starting location of the ramfile to be the last counter in the array
  }

  else { // Case 2: We have already deleted the original file
    Access::memoryArray[Access::veryLastCounter] = a; // We simply write the character into the veryLastCounter index
    Access::veryLastCounter++; // And then increment veryLastCounter
  }
  return nbyte; // Return the bytes written
}
