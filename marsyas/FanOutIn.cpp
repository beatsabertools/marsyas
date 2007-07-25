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
\class FanOutIn
\ingroup Composites
\brief FanOutIn of MarSystem objects

Combines a parallel set of MarSystem objects to a single MarSystem 
corresponding to executing the System objects in parallel using the
same input, and combining their outputs using a sum or a multiplication
combinator.

Controls:
- \b mrs_string/combinator [rw] : choose a * or + combination operation

*/

#include "FanOutIn.h"

using namespace std;
using namespace Marsyas;

FanOutIn::FanOutIn(string name):MarSystem("FanOutIn", name)
{
	isComposite_ = true;
	wrongOutConfig_ = false;
	addControls();
}

FanOutIn::FanOutIn(const FanOutIn& a):MarSystem(a)
{
	ctrl_combinator_ = getctrl("mrs_string/combinator");
	wrongOutConfig_ = false;
}

FanOutIn::~FanOutIn()
{
	deleteSlices();
}

void 
FanOutIn::deleteSlices()
{
	vector<realvec *>::const_iterator iter;
	for (iter= slices_.begin(); iter != slices_.end(); iter++)
	{
		delete *(iter);
	}
	slices_.clear();
}

MarSystem* 
FanOutIn::clone() const 
{
	return new FanOutIn(*this);
}

void 
FanOutIn::addControls()
{
	addctrl("mrs_bool/probe", false);
	setctrlState("mrs_bool/probe", true);
	addctrl("mrs_natural/disable", -1);
	setctrlState("mrs_natural/disable", true);
	addctrl("mrs_natural/enable", -1);
	setctrlState("mrs_natural/enable", true);

	addctrl("mrs_string/combinator", "+", ctrl_combinator_);
}

void 
FanOutIn::myUpdate(MarControlPtr sender)
{
	if (enabled_.getSize() != marsystemsSize_)
	{
		enabled_.create(marsystemsSize_);
		enabled_.setval(1.0);
	}

	disable_ = getctrl("mrs_natural/disable")->toNatural();
	if (disable_ != -1) 
	{
		enabled_(disable_) = 0.0;
		setctrl("mrs_natural/disable", -1);
	}
	enable_ = getctrl("mrs_natural/enable")->toNatural();
	if (enable_ != -1) 
	{
		enabled_(enable_) = 1.0;
		setctrl("mrs_natural/enable", -1);
	}

	if (marsystemsSize_ != 0)
	{
		wrongOutConfig_ = false;
				
		//propagate in flow controls to first child
		marsystems_[0]->setctrl("mrs_natural/inObservations", inObservations_);
		marsystems_[0]->setctrl("mrs_natural/inSamples", inSamples_);
		marsystems_[0]->setctrl("mrs_real/israte", israte_);
		marsystems_[0]->setctrl("mrs_string/inObsNames", inObsNames_);
		marsystems_[0]->update();

	  // update dataflow component MarSystems in order
		for (mrs_natural i=1; i < marsystemsSize_; i++)
		{
			marsystems_[i]->setctrl("mrs_natural/inSamples", marsystems_[0]->getctrl("mrs_natural/inSamples"));
			marsystems_[i]->setctrl("mrs_natural/inObservations", marsystems_[0]->getctrl("mrs_natural/inObservations"));
			marsystems_[i]->setctrl("mrs_real/israte", marsystems_[0]->getctrl("mrs_real/israte"));
			marsystems_[i]->setctrl("mrs_string/inObsNames", marsystems_[0]->getctrl("mrs_string/inObsNames"));
			marsystems_[i]->update(sender);

			if(marsystems_[i]->getctrl("mrs_natural/onSamples")->to<mrs_natural>() != 
				marsystems_[0]->getctrl("mrs_natural/onSamples")->to<mrs_natural>() ||
				marsystems_[i]->getctrl("mrs_natural/onObservations")->to<mrs_natural>() != 
				marsystems_[0]->getctrl("mrs_natural/onObservations")->to<mrs_natural>())
			{
				//MRSERR("FanInOut::myUpdate - child MarSystem " + marsystems_[i]->getPrefix() + 
				//	" ouput configuration is not the same as the one from the first child MarSystem(" + 
				//	marsystems_[0]->getPrefix() + " ! Outputing zero valued result...");
				wrongOutConfig_ = true;
			}
		}

		// forward flow propagation
		setctrl(ctrl_onSamples_, marsystems_[0]->getctrl("mrs_natural/onSamples")->toNatural());
		setctrl(ctrl_onObservations_, marsystems_[0]->getctrl("mrs_natural/onObservations")->toNatural());
		setctrl(ctrl_osrate_, marsystems_[0]->getctrl("mrs_real/osrate")->toReal());
		ostringstream oss;
		oss << marsystems_[0]->getctrl("mrs_string/onObsNames");
		setctrl(ctrl_onObsNames_, oss.str());

		// update buffers between components 
		if ((mrs_natural)slices_.size() < marsystemsSize_) 
			slices_.resize(marsystemsSize_, NULL);
		for (mrs_natural i=0; i< marsystemsSize_; i++)
		{
			if (slices_[i] != NULL) 
			{
				if ((slices_[i])->getRows() != marsystems_[i]->getctrl("mrs_natural/onObservations")->toNatural()  ||
					(slices_[i])->getCols() != marsystems_[i]->getctrl("mrs_natural/onSamples")->toNatural())
				{
					delete slices_[i];
					slices_[i] = new realvec(marsystems_[i]->getctrl("mrs_natural/onObservations")->toNatural(), 
						marsystems_[i]->getctrl("mrs_natural/onSamples")->toNatural());
				}
			}
			else 
			{
				slices_[i] = new realvec(marsystems_[i]->getctrl("mrs_natural/onObservations")->toNatural(), 
					marsystems_[i]->getctrl("mrs_natural/onSamples")->toNatural());
			}
			(slices_[i])->setval(0.0);
		}
	}
	else //if composite is empty...
		MarSystem::myUpdate(sender);
}

void
FanOutIn::myProcess(realvec& in, realvec& out)
{
	if(marsystemsSize_>0)
	{
		if(ctrl_combinator_->to<mrs_string>() == "+")
			out.setval(0); //identity operator
		if(ctrl_combinator_->to<mrs_string>() == "*")
			out.setval(1); //identity operator
		
		if(wrongOutConfig_)
		{
			//if there is  a child with a non matching output configuration, just output zeros
			MRSERR("FanInOut::myUpdate - at least one child MarSystem ouput \
						 configuration is not the same as the one from the first child \
						 MarSystem! Outputing zero valued result...");
			out.setval(0);
			return;
		}
		
		for (mrs_natural i = 0; i < marsystemsSize_; i++)
		{
			if (enabled_(i))
			{
				marsystems_[i]->process(in, *(slices_[i]));
				if(ctrl_combinator_->to<mrs_string>() == "+")
					out += *(slices_[i]);
				if(ctrl_combinator_->to<mrs_string>() == "*")
					out *= *(slices_[i]);
			}
		}
	}
	else //composite has no children!
	{
		MRSWARN("FanOutIn::process: composite has no children MarSystems - passing input to output without changes.");
		out = in;
	}
}
