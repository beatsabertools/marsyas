/*
** Copyright (C) 1998-2007 George Tzanetakis <gtzan@cs.uvic.ca>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/


/** 
    \class StereoSpectrumFeatures
    \brief StereoSpectrumFeatures capture panning information 

    After computing the Stereo Spectrum we can summarize 
it by extracting features using the StereoSpectrumFeatures. 
*/

#ifndef MARSYAS_STEREOSPECTRUMFEATURES_H
#define MARSYAS_STEREOSPECTRUMFEATURES_H

#include "MarSystem.h"	

namespace Marsyas
{

class StereoSpectrumFeatures: public MarSystem
{
private: 
  void myUpdate(MarControlPtr sender);
  
  mrs_real m0_;
  mrs_real m1_;
  
public:
  StereoSpectrumFeatures(std::string name);
  
  ~StereoSpectrumFeatures();
  MarSystem* clone() const;  
  
  void myProcess(realvec& in, realvec& out);
};

}//namespace Marsyas

#endif

	

	

	
