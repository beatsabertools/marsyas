/*
** Copyright (C) 1998-2006 George Tzanetakis <gtzan@cs.uvic.ca>
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
	\class PeFeatSelect
	\ingroup MarSystem
	\brief This MarSystem takes a realvec with peak information (using peakView)
	and allows to select which peak features should be sent to the output. 
	Used for similarity computations on peaks.  

	Controls:
	- \b mrs_natural/selectedFeatures [rw] : bit field used to select peaks features to output
	- \b mrs_natural/totalNumPeaks [w] : this control sets the total num of peaks at the input (should normally be linked with PeConvert similar control)
	- \b mrs_natural/frameMaxNumPeaks [w] : this control sets the maximum num of peaks per frame at the input (should normally be linked with PeConvert similar control)
*/

#include "PeFeatSelect.h"
#include "peakView.h"

using namespace std;
using namespace Marsyas;

PeFeatSelect::PeFeatSelect(string name):MarSystem("PeFeatSelect", name)
{
	addControls();
}

PeFeatSelect::PeFeatSelect(const PeFeatSelect& a) : MarSystem(a)
{
	ctrl_selectedFeatures_ = getctrl("mrs_natural/selectedFeatures");
	ctrl_totalNumPeaks_ = getctrl("mrs_natural/totalNumPeaks");
	ctrl_frameMaxNumPeaks_ = getctrl("mrs_natural/frameMaxNumPeaks");
}

PeFeatSelect::~PeFeatSelect()
{
}

MarSystem* 
PeFeatSelect::clone() const 
{
	return new PeFeatSelect(*this);
}

void 
PeFeatSelect::addControls()
{
	addctrl("mrs_natural/selectedFeatures", 0, ctrl_selectedFeatures_);
	ctrl_selectedFeatures_->setState(true);
	
	addctrl("mrs_natural/totalNumPeaks", 0, ctrl_totalNumPeaks_);
	ctrl_totalNumPeaks_->setState(true);
	
	addctrl("mrs_natural/frameMaxNumPeaks", 0, ctrl_frameMaxNumPeaks_);
	ctrl_frameMaxNumPeaks_->setState(true);

	selectedFeatures_ = 0;
	frameMaxNumPeaks_ = 0;

	numFeats_ = 0;
}

void
PeFeatSelect::myUpdate(MarControlPtr sender)
{
	if(ctrl_selectedFeatures_->to<mrs_natural>() != selectedFeatures_ ||
		 ctrl_frameMaxNumPeaks_->to<mrs_natural>() != frameMaxNumPeaks_)
	{
		selectedFeatures_ = ctrl_selectedFeatures_->to<mrs_natural>();
		frameMaxNumPeaks_ = ctrl_frameMaxNumPeaks_->to<mrs_natural>();
		
		//determine the number of selected features to output, per peak
		numFeats_ = 0;
		ostringstream oss;
		if(selectedFeatures_ & PeFeatSelect::pkFrequency)
		{
			numFeats_++;
			oss << "pkFrequency,";
		}
		if(selectedFeatures_ & PeFeatSelect::pkAmplitude)
		{
			numFeats_++;
			oss << "pkAmplitude,";
		}
		if(selectedFeatures_ & PeFeatSelect::pkFrame)
		{
			numFeats_++;
			oss << "pkFrame,";
		}
		//-----------------------------------------------------
		if(selectedFeatures_ & (PeFeatSelect::pkSetFrequencies |
			PeFeatSelect::pkSetAmplitudes | 
			PeFeatSelect::pkSetFrames))
		{
			numFeats_++;
			oss << "frameNumPeaks,";
		}
		if(selectedFeatures_ & PeFeatSelect::pkSetFrequencies)
		{
			for(mrs_natural i = 0; i < frameMaxNumPeaks_; ++i)
				oss << "pk_"<< i << "_Frequency,";
			numFeats_ += frameMaxNumPeaks_;
		}
		if(selectedFeatures_ & PeFeatSelect::pkSetAmplitudes)
		{
			for(mrs_natural i = 0; i < frameMaxNumPeaks_; ++i)
				oss << "pk_"<< i << "_Amplitude,";
			numFeats_ += frameMaxNumPeaks_;
		}
		if(selectedFeatures_ & PeFeatSelect::pkSetFrames)
		{
			for(mrs_natural i = 0; i < frameMaxNumPeaks_; ++i)
				oss << "pk_"<< i << "_Frame,";
			numFeats_ += frameMaxNumPeaks_;
		}
		if(numFeats_ == 0)
			oss << ",";

		ctrl_onObsNames_->setValue(oss.str(), NOUPDATE);
	}

	ctrl_onSamples_->setValue(ctrl_totalNumPeaks_->to<mrs_natural>(), NOUPDATE);
	ctrl_onObservations_->setValue(numFeats_, NOUPDATE);
	ctrl_osrate_->setValue(ctrl_israte_, NOUPDATE);
}

void 
PeFeatSelect::myProcess(realvec& in, realvec& out)
{
	peakView inPeakView(in);

	//get the total number of peaks in input texture window,
	//and reconfigure output flow controls (i.e. call update())
	//ctrl_frameMaxNumPeaks_->setValue(inPeakView.getFrameMaxNumPeaks(), NOUPDATE);
	//ctrl_totalNumPeaks_->setValue(inPeakView.getTotalNumPeaks()); //update is called here!

	//if there is at least one peak at the input, and at least one selected feature
	//fill the output realvec (otherwise there is no need to do anything else)
	if(ctrl_totalNumPeaks_->to<mrs_natural>() > 0 && ctrl_selectedFeatures_->to<mrs_natural>() != 0)
	{
		//////////////////////////////////////////////////////////////////////////
		//fill the output realvec with the selected feature vector for each peak
		//////////////////////////////////////////////////////////////////////////
		mrs_natural peak_index = 0;
		for(mrs_natural f=0; f < inPeakView.getNumFrames(); ++f)//frame loop
		{
			mrs_natural frameNumPeaks = inPeakView.getFrameNumPeaks(f);
			for(mrs_natural p=0; p<frameNumPeaks; ++p)//iterate over peaks in this frame (if any)
			{	
				mrs_natural feat_index = 0;

				if(selectedFeatures_ & PeFeatSelect::pkFrequency)
				{
					out(feat_index, peak_index) = inPeakView(p, peakView::pkFrequency, f);
					if(selectedFeatures_ & PeFeatSelect::barkPkFreq)
					{
						out(feat_index, peak_index) = hertz2bark(out(feat_index, peak_index));
					}
					feat_index++;
				}
				if(selectedFeatures_ & PeFeatSelect::pkAmplitude)
				{
					out(feat_index, peak_index) = inPeakView(p, peakView::pkAmplitude, f);
					if(selectedFeatures_ & PeFeatSelect::dBPkAmp)
					{
						out(feat_index, peak_index) = amplitude2dB(out(feat_index, peak_index));
					}
					feat_index++;
				}
				if(selectedFeatures_ & PeFeatSelect::pkFrame)
				{
					out(feat_index, peak_index) = inPeakView(p, peakView::pkFrame, f);
					feat_index++;
				}
				//-----------------------------------------------------------------------------
				if(selectedFeatures_ & (PeFeatSelect::pkSetFrequencies |
					PeFeatSelect::pkSetAmplitudes | 
					PeFeatSelect::pkSetFrames))
				{
					out(feat_index, peak_index) = frameNumPeaks;
					feat_index++;
				}
				if(selectedFeatures_ & PeFeatSelect::pkSetFrequencies)
				{
					//for each peak, just get all the feats of the peaks in the same frame
					for(mrs_natural i=0; i < frameNumPeaks; ++i)
					{
						out(feat_index, peak_index) = inPeakView(i, peakView::pkFrequency, f);
						feat_index++;
					}
				}
				if(selectedFeatures_ & PeFeatSelect::pkSetAmplitudes)
				{
					//for each peak, just get all the feats of the peaks in the same frame
					for(mrs_natural i=0; i < frameNumPeaks; ++i)
					{
						out(feat_index, peak_index) = inPeakView(i, peakView::pkAmplitude, f);
						feat_index++;
					}
				}
				if(selectedFeatures_ & PeFeatSelect::pkSetFrames)
				{
					//for each peak, just get all the feats of the peaks in the same frame
					for(mrs_natural i=0; i < frameNumPeaks; ++i)
					{
						out(feat_index, peak_index) = inPeakView(i, peakView::pkFrame, f);
						feat_index++;
					}
				}
				//move on to next peak
				peak_index++;
			}
		}
	}
}








