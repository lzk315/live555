/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**********/
// Copyright (c) 1996-2003 Live Networks, Inc.  All rights reserved.
// A C++ equivalent to the standard C routine "strdup()".
// This generates a char* that can be deleted using "delete[]"
// Implementation

#include "strDup.hh"
#include "string.h"

char* strDup(char const* str) {
  size_t len = strlen(str) + 1;
  char* copy = new char[len];

  if (copy != NULL) {
    memcpy(copy, str, len);
  }
  return copy;
}

char* strDupSize(char const* str) {
  size_t len = strlen(str) + 1;
  char* copy = new char[len];

  return copy;
}
