#pragma once

/*  Ayane
 *
 *  Created by Philip Deljanov on 11-10-07.
 *  Copyright 2011 Philip Deljanov. All rights reserved.
 *
 */

/** @file channels.h
  * \brief Contains enumeration of audio channels.
**/

#include <stdint.h>

namespace Ayane
{

  typedef uint64_t Channels;

  /** Enumeration of channels.
    * 
    * This channels contained in this enumeration are the standard PCM channels (7.1)
    * ranging upto the Ultra-High Definition channels (22.2). Totally overkill, but 
    * hopefully future proof.
    *
  **/ 
  enum Channel
  {
    FrontLeft		= 1<<1,  // FL
    FrontRight		= 1<<2,  // FR
    FrontCenter		= 1<<3,  // FC
    LowFrequencyOne	= 1<<4,  // LFE1
    BackLeft		= 1<<5,  // BL
    BackRight		= 1<<6,  // BR
    FrontLeftCenter	= 1<<7,  // FLc
    FrontRightCenter	= 1<<8,  // FRc
    BackCenter		= 1<<9,  // BC
    LowFrequencyTwo	= 1<<10, // LFE2
    SideLeft		= 1<<11, // SiL
    SideRight		= 1<<12, // SiR
    TopFrontLeft	= 1<<13, // TpFL
    TopFrontRight	= 1<<14, // TpFR
    TopFrontCenter	= 1<<15, // TpFC
    TopCenter		= 1<<16, // TpC
    TopBackLeft		= 1<<17, // TpBL
    TopBackRight	= 1<<18, // TpBR
    TopSideLeft		= 1<<19, // TpSiL
    TopSideRight	= 1<<20, // TpSiR
    TopBackCenter	= 1<<21, // TpBc
    BottomFrontCenter	= 1<<22, // BtFC
    BottomFrontLeft	= 1<<23, // BtFL
    BottomFrontRight	= 1<<24  // BtFR
  };
  
  
  /** Enumeration of channel layouts. Most commonly used (and some rarely used: 10.2/22.2) layouts are here
  **/
  enum ChannelLayout
  {
    LayoutMono         = FrontCenter,
    LayoutStereo       = FrontLeft          | FrontRight,
    Layout2Point1      = LayoutStereo       | LowFrequencyOne,
    Layout3Point0      = LayoutStereo       | FrontCenter,
    Layout3Point0Back  = LayoutStereo       | BackCenter,
    Layout4Point0      = Layout3Point0      | BackCenter,
    LayoutQuad         = LayoutStereo       | BackLeft        | BackRight,
    LayoutQuadSide     = LayoutStereo       | SideLeft        | SideRight,
    Layout3Point1      = Layout3Point0      | LowFrequencyOne,
    Layout5Point0      = Layout3Point0      | BackLeft        | BackRight,
    Layout5Point0Side  = Layout3Point0      | SideLeft        | SideRight,
    Layout4Point1      = Layout4Point0      | LowFrequencyOne,
    Layout5Point1      = Layout5Point0      | LowFrequencyOne,
    Layout5Point1Side  = Layout3Point0      | SideLeft        | SideRight        | LowFrequencyOne,
    Layout6Point0      = Layout5Point0      | BackCenter,
    Layout6Point0Front = LayoutQuadSide     | FrontLeftCenter | FrontRightCenter,
    LayoutHexagonal    = Layout5Point0      | BackCenter,
    Layout6Point1      = Layout5Point1Side  | BackCenter,
    Layout6Point1Back  = Layout5Point1      | BackCenter,
    Layout6Point1Front = Layout6Point0Front | LowFrequencyOne,
    Layout7Point0      = Layout5Point0Side  | BackLeft        | BackRight,
    Layout7Point0Front = Layout5Point0Side  | FrontLeftCenter | FrontRightCenter,
    Layout7Point1      = Layout5Point0Side  | BackLeft        | BackRight,
    Layout7Point1Wide  = Layout5Point0Side  | FrontLeftCenter | FrontRightCenter,
    LayoutOctagonal    = Layout5Point0Side  | BackLeft        | BackCenter       | BackRight
  };
  
}
