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

#include "CollectionFileSource.h"

using namespace std;
using namespace Marsyas;

CollectionFileSource::CollectionFileSource(string name):AbsSoundFileSource("SoundFileSource", name)
{
	addControls();
}

CollectionFileSource::CollectionFileSource(const CollectionFileSource& a):AbsSoundFileSource(a)
{
	ctrl_currentlyPlaying_ = getctrl("mrs_string/currentlyPlaying");
	ctrl_currentLabel_ = getctrl("mrs_natural/currentLabel");
	ctrl_labelNames_ = getctrl("mrs_string/labelNames");
	ctrl_nLabels_ = getctrl("mrs_natural/nLabels");
}

CollectionFileSource::~CollectionFileSource()
{
	delete isrc_;
	delete downsampler_;
}

MarSystem* 
CollectionFileSource::clone() const
{
	return new CollectionFileSource(*this);
}

void
CollectionFileSource::addControls()
{
	addctrl("mrs_bool/notEmpty", true);  
	notEmpty_ = true;
	addctrl("mrs_natural/pos", (mrs_natural)0);
	setctrlState("mrs_natural/pos", true);

	addctrl("mrs_natural/loopPos", (mrs_natural)0);
	setctrlState("mrs_natural/pos", true);  

	addctrl("mrs_string/filename", "daufile");
	setctrlState("mrs_string/filename", true);
	addctrl("mrs_natural/size", (mrs_natural)0);
	addctrl("mrs_string/filetype", "mf");
	addctrl("mrs_natural/cindex", 0);
	setctrlState("mrs_natural/cindex", true);

	addctrl("mrs_string/allfilenames", "collectionFileSource");
	setctrlState("mrs_string/allfilenames", true);
	addctrl("mrs_natural/numFiles", 0);  

	addctrl("mrs_real/repetitions", 1.0);
	setctrlState("mrs_real/repetitions", true);
	addctrl("mrs_real/duration", -1.0);
	setctrlState("mrs_real/duration", true);

	addctrl("mrs_bool/advance", false);
	setctrlState("mrs_bool/advance", true);

	addctrl("mrs_bool/shuffle", false);
	setctrlState("mrs_bool/shuffle", true);

	addctrl("mrs_natural/cindex", 0);
	setctrlState("mrs_natural/cindex", true);

	addctrl("mrs_string/currentlyPlaying", "daufile", ctrl_currentlyPlaying_);
	addctrl("mrs_natural/currentLabel", 0, ctrl_currentLabel_);
	addctrl("mrs_string/labelNames", ",", ctrl_labelNames_);
	addctrl("mrs_natural/nLabels", 0, ctrl_nLabels_);
	mngCreated_ = false; 
}

void 
CollectionFileSource::getHeader(string filename)
{
	col_.read(filename);
	updctrl("mrs_string/allfilenames", col_.toLongString());
	updctrl("mrs_natural/numFiles", col_.getSize());  

	cindex_ = 0;
	setctrl("mrs_natural/cindex", 0);
	setctrl("mrs_bool/notEmpty", true);
	ctrl_currentlyPlaying_->setValue(col_.entry(0), NOUPDATE);
	if (col_.hasLabels())
	{
		ctrl_currentLabel_->setValue(col_.labelNum(col_.labelEntry(0)), NOUPDATE);
		ctrl_labelNames_->setValue(col_.getLabelNames(), NOUPDATE);
		ctrl_nLabels_->setValue(col_.getNumLabels(), NOUPDATE);
	}
	addctrl("mrs_natural/size", 1); // just so it's not zero 
	setctrl("mrs_natural/pos", 0);
	pos_ = 0;
}

void
CollectionFileSource::myUpdate(MarControlPtr sender)
{

	
	(void) sender;
	inSamples_ = getctrl("mrs_natural/inSamples")->to<mrs_natural>();
	inObservations_ = getctrl("mrs_natural/inObservations")->to<mrs_natural>();

	filename_ = getctrl("mrs_string/filename")->to<mrs_string>();    
	pos_ = getctrl("mrs_natural/pos")->to<mrs_natural>();

	if (mngCreated_ == false) 
	{
		isrc_ = new SoundFileSource("isrc");
		mngCreated_ = true;
		downsampler_ = new DownSampler("downsampler_"); 
	}

	repetitions_ = getctrl("mrs_real/repetitions")->to<mrs_real>();
	duration_ = getctrl("mrs_real/duration")->to<mrs_real>();
	advance_ = getctrl("mrs_bool/advance")->to<mrs_bool>();
	cindex_ = getctrl("mrs_natural/cindex")->to<mrs_natural>();

	if (getctrl("mrs_bool/shuffle")->isTrue())
	{
		col_.shuffle();
		setctrl("mrs_bool/shuffle", false);
	}

	if (cindex_ < col_.size()) 
	{
		isrc_->updctrl("mrs_string/filename", col_.entry(cindex_));
		ctrl_currentlyPlaying_->setValue(col_.entry(cindex_), NOUPDATE);
		ctrl_currentLabel_->setValue(col_.labelNum(col_.labelEntry(cindex_)), NOUPDATE);
		ctrl_labelNames_->setValue(col_.getLabelNames(), NOUPDATE);
		ctrl_nLabels_->setValue(col_.getNumLabels(), NOUPDATE);
	}

	myIsrate_ = isrc_->getctrl("mrs_real/israte")->to<mrs_real>();//[!] why get an INPUT flow control?!
	onObservations_ = isrc_->getctrl("mrs_natural/onObservations")->to<mrs_natural>();

	setctrl("mrs_real/israte", myIsrate_);//[!] why set an INPUT flow control?!?
	setctrl("mrs_real/osrate", myIsrate_);
	setctrl("mrs_natural/onObservations", onObservations_);

	isrc_->updctrl("mrs_natural/inSamples", inSamples_);
	setctrl("mrs_natural/onSamples", inSamples_);
	setctrl("mrs_real/israte", myIsrate_);//[!] why set an INPUT flow control?!?
	setctrl("mrs_real/osrate", myIsrate_);
	setctrl("mrs_natural/onObservations", onObservations_);
	temp_.create(inObservations_, inSamples_);

	isrc_->updctrl("mrs_real/repetitions", repetitions_);
	isrc_->updctrl("mrs_natural/pos", pos_);
	isrc_->updctrl("mrs_real/duration", duration_);
	isrc_->updctrl("mrs_bool/advance", advance_);
	isrc_->updctrl("mrs_natural/cindex", cindex_);

	cindex_ = getctrl("mrs_natural/cindex")->to<mrs_natural>();  


}

void
CollectionFileSource::myProcess(realvec& in, realvec &out)
{
	
	
	if (advance_) 
	{

		
		cindex_ = cindex_ + 1;
		if (cindex_ == col_.size())
			cindex_ = 0;
		setctrl("mrs_natural/cindex", cindex_);

		isrc_->updctrl("mrs_string/filename", col_.entry(cindex_));   
		updctrl("mrs_natural/pos", isrc_->getctrl("mrs_natural/pos"));   
		myIsrate_ = isrc_->getctrl("mrs_real/israte")->to<mrs_real>();
		onObservations_ = isrc_->getctrl("mrs_natural/onObservations")->to<mrs_natural>();

		setctrl("mrs_real/israte", myIsrate_);
		setctrl("mrs_real/osrate", myIsrate_);
		setctrl("mrs_natural/onObservations", onObservations_);
		setctrl("mrs_string/currentlyPlaying", col_.entry(cindex_));
		ctrl_currentlyPlaying_->setValue(col_.entry(cindex_), NOUPDATE);
		
		setctrl("mrs_natural/currentLabel", col_.labelNum(col_.labelEntry(cindex_)));
		ctrl_currentLabel_->setValue(col_.labelNum(col_.labelEntry(cindex_)), NOUPDATE);
		ctrl_labelNames_->setValue(col_.getLabelNames(), NOUPDATE);
		ctrl_nLabels_->setValue(col_.getNumLabels(), NOUPDATE);
		
		update();   

		isrc_->process(in,out);
		setctrl("mrs_natural/pos", isrc_->getctrl("mrs_natural/pos"));
		setctrl("mrs_bool/notEmpty", isrc_->getctrl("mrs_bool/notEmpty"));

		if (cindex_ > col_.size()-2)  
		{
			setctrl("mrs_bool/notEmpty", false);
			notEmpty_ = false;      
			advance_ = false;
			return;
			
		}



		return;
	}
	else
	{
		//finished current file. Advance to next one in collection (if any)
		if (!isrc_->getctrl("mrs_bool/notEmpty")->isTrue())
		{

			
			//check if there a following file ion the collection
			if (cindex_ < col_.size() -1)
			{
				cindex_ = cindex_ + 1;
				setctrl("mrs_natural/cindex", cindex_);
				isrc_->updctrl("mrs_string/filename", col_.entry(cindex_));      
				isrc_->updctrl("mrs_natural/pos", 0);     
				pos_ = 0;
				myIsrate_ = isrc_->getctrl("mrs_real/israte")->to<mrs_real>();
				onObservations_ = isrc_->getctrl("mrs_natural/onObservations")->to<mrs_natural>();

				setctrl("mrs_real/israte", myIsrate_);
				setctrl("mrs_real/osrate", myIsrate_);
				setctrl("mrs_natural/onObservations", onObservations_);
				setctrl("mrs_string/currentlyPlaying", col_.entry(cindex_));
				ctrl_currentlyPlaying_->setValue(col_.entry(cindex_), NOUPDATE);
				setctrl("mrs_natural/currentLabel", col_.labelNum(col_.labelEntry(cindex_)));
				ctrl_currentLabel_->setValue(col_.labelNum(col_.labelEntry(cindex_)), NOUPDATE);
				ctrl_labelNames_->setValue(col_.getLabelNames(), NOUPDATE);
				ctrl_nLabels_->setValue(col_.getNumLabels(), NOUPDATE);
			}
			else //no more files in collection
			{
				setctrl("mrs_bool/notEmpty", false);
				notEmpty_ = false;
			}
		}

		isrc_->process(in,out);
		setctrl("mrs_natural/pos", isrc_->getctrl("mrs_natural/pos"));
		setctrl("mrs_bool/notEmpty", isrc_->getctrl("mrs_bool/notEmpty"));
	} 
}  








