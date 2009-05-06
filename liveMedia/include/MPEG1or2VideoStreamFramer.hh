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
// A filter that breaks up an MPEG 1 or 2 video elementary stream into
//   frames for: Video_Sequence_Header, GOP_Header, Picture_Header
// C++ header

#ifndef _MPEG_1OR2_VIDEO_STREAM_FRAMER_HH
#define _MPEG_1OR2_VIDEO_STREAM_FRAMER_HH

#ifndef _MPEG_VIDEO_STREAM_FRAMER_HH
#include "MPEGVideoStreamFramer.hh"
#endif

class MPEG1or2VideoStreamFramer: public MPEGVideoStreamFramer {
public:
  static MPEG1or2VideoStreamFramer*
      createNew(UsageEnvironment& env, FramedSource* inputSource,
		Boolean iFramesOnly = False,
		double vshPeriod = 5.0
		/* how often (in seconds) to inject a Video_Sequence_Header,
		   if one doesn't already appear in the stream */);

private:
  MPEG1or2VideoStreamFramer(UsageEnvironment& env,
			    FramedSource* inputSource,
			    Boolean iFramesOnly, double vshPeriod);
      // called only by createNew()
  virtual ~MPEG1or2VideoStreamFramer();

  static void continueReadProcessing(void* clientData,
				     unsigned char* ptr, unsigned size);
  void continueReadProcessing();

private:
  // redefined virtual functions:
  virtual Boolean isMPEG1or2VideoStreamFramer() const;
  virtual void doGetNextFrame();
  virtual float getPlayTime(unsigned numFrames) const; 

private:
  unsigned fPictureCount; // hack used to implement doGetNextFrame()
  double fFrameRate;
  struct timeval fPresentationTimeBase;
  TimeCode fCurGOPTimeCode, fPrevGOPTimeCode;
  unsigned fPicturesAdjustment;
  double fPictureTimeBase;
  unsigned fTcSecsBase;
  Boolean fHaveSeenFirstTimeCode;

  void computeTimestamp(unsigned numAdditionalPictures); // sets fTimestamp
  void setTimeCodeBaseParams();
  double getCurrentTimestamp() const;

private: // parsing state
  class MPEG1or2VideoStreamParser* fParser;
  friend class MPEG1or2VideoStreamParser; // hack
};

#endif
