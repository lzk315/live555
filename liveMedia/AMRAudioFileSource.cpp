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
// "liveMedia"
// Copyright (c) 1996-2002 Live Networks, Inc.  All rights reserved.
// A source object for AMR audio files (as defined in RFC 3267, section 5)
// Implementation

#if defined(__WIN32__) || defined(_WIN32)
#include <io.h>
#include <fcntl.h>
#endif

#include "AMRAudioFileSource.hh"
#include "GroupsockHelper.hh"

////////// AMRAudioFileSource //////////

AMRAudioFileSource*
AMRAudioFileSource::createNew(UsageEnvironment& env, char const* fileName) {
  FILE* fid = NULL;
  Boolean magicNumberOK = True;
  do {

    // Check for a special case file name: "stdin"
    if (strcmp(fileName, "stdin") == 0) {
      fid = stdin;
#if defined(__WIN32__) || defined(_WIN32)
      _setmode(_fileno(stdin), _O_BINARY); // convert to binary mode
#endif
    } else { 
      fid = fopen(fileName, "rb");
      if (fid == NULL) {
	env.setResultMsg("unable to open file \"",fileName, "\"");
	break;
      }
    }

    // Now, having opened the input file, read the first few bytes, to
    // check the required 'magic number':
    magicNumberOK = False; // until we learn otherwise
    Boolean isWideband = False; // by default
    unsigned numChannels = 1; // by default
    char buf[100];
    // Start with the first 6 bytes (the first 5 of which must be "#!AMR"):
    if (fread(buf, 1, 6, fid) < 6) break;
    if (strncmp(buf, "#!AMR", 5) != 0) break; // bad magic #
    unsigned bytesRead = 6;
    
    // The next bytes must be "\n", "-WB\n", "_MC1.0\n", or "-WB_MC1.0\n"
    if (buf[5] == '-') {
      // The next bytes must be "WB\n" or "WB_MC1.0\n"
      if (fread(&buf[bytesRead], 1, 3, fid) < 3) break;
      if (strncmp(&buf[bytesRead], "WB", 2) != 0) break; // bad magic #
      isWideband = True;
      bytesRead += 3;
    }
    if (buf[bytesRead-1] == '_') {
      // The next bytes must be "MC1.0\n"
      if (fread(&buf[bytesRead], 1, 6, fid) < 6) break;
      if (strncmp(&buf[bytesRead], "MC1.0\n", 6) != 0) break; // bad magic #
      bytesRead += 6;

      // The next 4 bytes contain the number of channels:
      char channelDesc[4];
      if (fread(channelDesc, 1, 4, fid) < 4) break;
      numChannels = channelDesc[3]&0xF;
    } else if (buf[bytesRead-1] != '\n') {
      break; // bad magic #
    }

    // If we get here, the magic number was OK:
    magicNumberOK = True;

#ifdef DEBUG
    fprintf(stderr, "isWideband: %d, numChannels: %d\n",
	    isWideband, numChannels);
#endif
    return new AMRAudioFileSource(env, fid, isWideband, numChannels);
  } while (0);

  // An error occurred:
  if (fid != NULL) fclose(fid);
  if (!magicNumberOK) {
    env.setResultMsg("Bad (or nonexistent) AMR file header");
  }
  return NULL;
}

AMRAudioFileSource
::AMRAudioFileSource(UsageEnvironment& env, FILE* fid,
		     Boolean isWideband, unsigned numChannels)
  : FramedFileSource(env, fid),
    fIsWideband(isWideband), fNumChannels(numChannels) {
}

AMRAudioFileSource::~AMRAudioFileSource() {
  fclose(fFid);
}

#ifdef BSD
static struct timezone Idunno;
#else
static int Idunno;
#endif

// The mapping from the "FT" field to frame size.
// Values of 65535 are invalid.
// Values of 65534 are currently unknown (because I was too cheap to buy
// the AMR spec, which defines these).  Please replace these
// FT_SIZE_UNKNOWN values, if you know what they should be.
#define FT_INVALID 65535
#define FT_SIZE_UNKNOWN 65534
static unsigned short frameSize[16] = {
  12, 13, 15, 17,
  19, 20, 26, 31,
  5/*???*/, FT_INVALID, FT_INVALID, FT_INVALID,
  FT_INVALID, FT_INVALID, FT_INVALID, 0
};
static unsigned short frameSizeWideband[16] = {
  17, 23, FT_SIZE_UNKNOWN, FT_SIZE_UNKNOWN,
  FT_SIZE_UNKNOWN, FT_SIZE_UNKNOWN, FT_SIZE_UNKNOWN, FT_SIZE_UNKNOWN,
  FT_SIZE_UNKNOWN, FT_SIZE_UNKNOWN, FT_INVALID, FT_INVALID,
  FT_INVALID, FT_INVALID, 0, 0
};

Boolean AMRAudioFileSource::isAMRAudioSource() const {
  return True;
}

void AMRAudioFileSource::doGetNextFrame() {
  if (feof(fFid) || ferror(fFid)) {
    handleClosure(this);
    return;
  }

  // Begin by reading the 1-byte frame header (and checking it for validity)
  while (1) {
    if (fread(&fLastFrameHeader, 1, 1, fFid) < 1) {
      handleClosure(this);
      return;
    }
    if ((fLastFrameHeader&0x83) != 0) {
#ifdef DEBUG
      fprintf(stderr, "Invalid frame header 0x%02x (padding bits (0x83) are not zero)\n", fLastFrameHeader);
#endif
    } else {
      unsigned char ft = (fLastFrameHeader&0x78)>>3;
      fFrameSize = fIsWideband ? frameSizeWideband[ft] : frameSize[ft];
      if (fFrameSize == FT_INVALID) {
#ifdef DEBUG
	fprintf(stderr, "Invalid FT field %d (from frame header 0x%02x)\n",
		ft, fLastFrameHeader);
#endif
      } else if (fFrameSize == FT_SIZE_UNKNOWN) {
#ifdef DEBUG
	fprintf(stderr, "Unknown size for FT value %d.  Please fix this in \"liveMedia/AMRAudioFileSource.cpp\"\n", ft);
#endif
      } else {
	// The frame header is OK
#ifdef DEBUG
	fprintf(stderr, "Valid frame header 0x%02x -> ft %d -> frame size %d\n", fLastFrameHeader, ft, fFrameSize);
#endif
	break;
      }
    }
  } 
      
  // Next, read the frame into the buffer provided
  if (fFrameSize > fMaxSize) {
    fFrameSize = fMaxSize;
  }
  fFrameSize = fread(fTo, 1, fFrameSize, fFid);

  // Set the 'presentation time':
  if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0) {
    // This is the first frame, so use the current time:
    gettimeofday(&fPresentationTime, &Idunno);
  } else {
    // Increment by the play time of the previous frame (20 ms)
    unsigned uSeconds	= fPresentationTime.tv_usec + 20000;
    fPresentationTime.tv_sec += uSeconds/1000000;
    fPresentationTime.tv_usec = uSeconds%1000000;
  }

  // Switch to another task, and inform the reader that he has data:
  nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
				(TaskFunc*)FramedSource::afterGetting, this);
 }

float AMRAudioFileSource::getPlayTime(unsigned numFrames) const {
  return (float)(numFrames*0.020); // each frame is 20 ms
}