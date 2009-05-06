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
// Copyright (c) 1996-2004 Live Networks, Inc.  All rights reserved.
// A server demultiplexer for a MPEG 1 or 2 Program Stream
// C++ header

#ifndef _MPEG_1OR2_FILE_SERVER_DEMUX_HH
#define _MPEG_1OR2_FILE_SERVER_DEMUX_HH

#ifndef _SERVER_MEDIA_SESSION_HH
#include "ServerMediaSession.hh"
#endif

class MPEG1or2FileServerDemux: public Medium {
public:
  static MPEG1or2FileServerDemux*
  createNew(UsageEnvironment& env, char const* fileName);

  ServerMediaSubsession* newAudioServerMediaSubsession();
  ServerMediaSubsession* newVideoServerMediaSubsession();

private:
  MPEG1or2FileServerDemux(UsageEnvironment& env, char const* fileName);
      // called only by createNew();
  virtual ~MPEG1or2FileServerDemux();
};

#endif