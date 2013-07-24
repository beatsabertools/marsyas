/*
** Copyright (C) 2013 Steven Ness <sness@sness.net>
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
   nextract: new feature extraction
*/

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector> 
#include "Collection.h"
#include "MarSystemManager.h"
#include "CommandLineOptions.h"
#include "common_source.h"

using namespace std;
using namespace Marsyas;

int helpOpt_;
int usageOpt_;
mrs_natural memSize_ = 40;
mrs_natural winSize_ = 512;
mrs_natural hopSize_ = 512;
mrs_natural stereo_ = false;

string outputFilename_ = EMPTYSTRING;
mrs_bool timeOutput_ = false;

mrs_bool mfccFeature_ = false;
mrs_bool centroidFeature_ = false;
mrs_bool rolloffFeature_ = false;
mrs_bool fluxFeature_ = false;
mrs_bool kurtosisFeature_ = false;
mrs_bool skewnessFeature_ = false;
mrs_bool sfmFeature_ = false;
mrs_bool scfFeature_ = false;
mrs_bool chromaFeature_ = false;

mrs_bool rmsFeature_ = false;
mrs_bool yinFeature_ = false;

mrs_natural numMfccs_ = 13;

mrs_natural downsample_ = 1;
string wekaFilename_ = EMPTYSTRING;

mrs_string outputFormat_ = "libsvm";

CommandLineOptions cmdOptions_;

int printUsage(string progName)
{
  MRSDIAG("nextract.cpp - printUsage");
  cerr << "Usage : " << progName << " collection" << endl;
  cerr << endl;
  return 0;
}

int
printHelp(string progName)
{
  MRSDIAG("nextract.cpp - printHelp");
  cerr << "nextract, MARSYAS, Copyright 2013 Steven Ness" << endl;
  cerr << "---------------------------------------------" << endl;
  cerr << "Prints information about the sound files provided as arguments " << endl;
  cerr << endl;
  cerr << "Usage : " << progName << " file1 file2 ... fileN" << endl;
  cerr << endl;
  cerr << "where file1 file2 ... fileN are in a Marsyas supported format." << endl;
  cerr << "Help Options:" << endl;
  cerr << "-u       --usage        : Display short usage info." << endl;
  cerr << "-v       --verbose      : Verbose output." << endl;
  cerr << "-m       --memory       : Memory size." << endl;
  cerr << "-ws      --windowsize   : Analysis window size in samples." << endl;
  cerr << "-hp      --hopsize      : Analysis hop size in samples." << endl;
  cerr << "         --stereo       : Do analysis on stereo (default convert to mono)." << endl;

  cerr << "         --timeOutput   : Output the current time as the label" << endl;
  cerr << "         --mfcc         : Output MFCC features." << endl;
  cerr << "         --ctd          : Output spectral centroid features." << endl;
  cerr << "         --rlf          : Output spectral rolloff features." << endl;
  cerr << "         --flx          : Output spectral flux features." << endl;
  cerr << "         --kurtosis     : Output spectral kurtosis features." << endl;
  cerr << "         --skewness     : Output spectral skewness features." << endl;
  cerr << "         --sfm          : Output spectral flatness measure features." << endl;
  cerr << "         --scf          : Output spectral crest factor features." << endl;
  cerr << "         --chroma       : Output chroma features." << endl;

  cerr << "         --numMfccs     : Number of MFCCoefficients to output." << endl;

  // cerr << "-zcrs    --zeroCrossings                        : Output zero crossings as a feature." << endl;
  // cerr << "-lsp     --lineSpectralPair                     : Output line spectral pair features." << endl;
  // cerr << "-lpcc    --linearPredictionCepstralCoefficients : Output linear prediction cepstral coefficients features." << endl;

  cerr << "-rms     --rms                                  : Output rms as a feature." << endl;
  cerr << "-yin     --yin                                  : Output yin pitch estimate as a feature." << endl;

  cerr << "-ds      --downsample                                  : Downsampling ratio" << endl;

  cerr << "-o       --outputFilename                             : File to save data to." << endl;
  cerr << "-of      --outputFormat                                : Output file format (libsvm, sonicvisualiser)" << endl;
  cerr << "-w       --wekafile                : weka .arff filename " << endl;
  
  return 0;
}


void initOptions()
{
  cmdOptions_.addBoolOption("help", "h", false);
  cmdOptions_.addBoolOption("usage", "u", false);
  cmdOptions_.addBoolOption("verbose", "v", false);
  cmdOptions_.addBoolOption("timeOutput", "", false);

  cmdOptions_.addNaturalOption("memory", "m", 20);
  cmdOptions_.addNaturalOption("windowsize", "ws", 512);
  cmdOptions_.addNaturalOption("hopsize", "hp", 512);

  cmdOptions_.addNaturalOption("stereo", "st", false);

  cmdOptions_.addBoolOption("mfcc","", false);
  cmdOptions_.addBoolOption("ctd","", false);
  cmdOptions_.addBoolOption("rlf","", false);
  cmdOptions_.addBoolOption("flx","", false);
  cmdOptions_.addBoolOption("kurtosis","", false);
  cmdOptions_.addBoolOption("skewness","", false);
  cmdOptions_.addBoolOption("sfm","", false);
  cmdOptions_.addBoolOption("scf","", false);
  cmdOptions_.addBoolOption("chroma", "", false);

  cmdOptions_.addNaturalOption("numMfccs", "", 13);

  // cmdOptions_.addBoolOption("zeroCrossings", "zcrs", false);
  // cmdOptions_.addBoolOption("lineSpectralPair", "lsp", false);
  // cmdOptions_.addBoolOption("linearPredictionCepstralCoefficients", "lpcc", false);
  
  cmdOptions_.addBoolOption("rms", "", false);
  cmdOptions_.addBoolOption("yin", "", false);

  cmdOptions_.addNaturalOption("downsample", "ds", 1);

  cmdOptions_.addStringOption("outputFilename", "o", EMPTYSTRING);
  cmdOptions_.addStringOption("outputFormat", "of", "libsvm");
  cmdOptions_.addStringOption("wekaFilename", "w", EMPTYSTRING);
}

void loadOptions()
{
  helpOpt_ = cmdOptions_.getBoolOption("help");
  usageOpt_ = cmdOptions_.getBoolOption("usage");
  timeOutput_ = cmdOptions_.getBoolOption("timeOutput");
  memSize_ = cmdOptions_.getNaturalOption("memory");
  winSize_ = cmdOptions_.getNaturalOption("windowsize");
  hopSize_ = cmdOptions_.getNaturalOption("hopsize");
  mfccFeature_ = cmdOptions_.getBoolOption("mfcc");
  centroidFeature_ = cmdOptions_.getBoolOption("centroid");
  rolloffFeature_ = cmdOptions_.getBoolOption("rolloff");
  fluxFeature_ = cmdOptions_.getBoolOption("flux");
  kurtosisFeature_ = cmdOptions_.getBoolOption("kurtosis");
  skewnessFeature_ = cmdOptions_.getBoolOption("skewness");
  sfmFeature_ = cmdOptions_.getBoolOption("sfm");
  scfFeature_ = cmdOptions_.getBoolOption("scf");
  chromaFeature_ = cmdOptions_.getBoolOption("chroma");

  numMfccs_ = cmdOptions_.getNaturalOption("numMfccs");

  rmsFeature_ = cmdOptions_.getBoolOption("rms");
  yinFeature_ = cmdOptions_.getBoolOption("yin");

  downsample_ = cmdOptions_.getNaturalOption("downsample");

  outputFilename_ = cmdOptions_.getStringOption("outputFilename");
  outputFormat_ = cmdOptions_.getStringOption("outputFormat");
  wekaFilename_ = cmdOptions_.getStringOption("wekaFilename");

  // // If no features were explicitly set, out a small standard set of features
  // if ((mfccFeature_ == false) && 
  //     (centroidFeature_ == false) &&
  //     (rolloffFeature_ == false) &&
  //     (fluxFeature_ == false) &&
  //     (kurtoisFeature_ == false) &&
  //     (skewnessFeature_ == false) &&
  //     (sfmFeature_ == false) &&
  //     (scfFeature_ == false) &&
  //     (chromaFeature_ == false) &&
  //     (rmsFeature_ == false) &&
  //     (yinFeature_ == false)) {
  //   mfccFeature_ = true;
  // }

}

void extract(string inCollectionName)
{
  MarSystemManager mng;

  MarSystem* net = mng.create("Series", "net");
  net->addMarSystem(mng.create("SoundFileSource", "src"));
  if (stereo_ == false) {
    net->addMarSystem(mng.create("Stereo2Mono","toMono"));
  }
  net->addMarSystem(mng.create("ShiftInput/si"));
  net->addMarSystem(mng.create("Windowing", "ham"));

  MarSystem* mainFanout = mng.create("Fanout", "mainFanout");

  // Time based features
  if (rmsFeature_ || yinFeature_) {
    MarSystem* timeSeries = mng.create("Series", "timeSeries");
    MarSystem* timeFanout = mng.create("Fanout", "timeFanout");
    if (rmsFeature_) {
      timeFanout->addMarSystem(mng.create("Rms", "rms"));
    }
    if (yinFeature_) {
      timeFanout->addMarSystem(mng.create("Yin", "yin"));
    }
    timeSeries->addMarSystem(timeFanout);
    mainFanout->addMarSystem(timeSeries);
  }

  // Spectrum based features
  if (mfccFeature_ || centroidFeature_ || rolloffFeature_ || fluxFeature_ || 
      kurtosisFeature_ || skewnessFeature_ || sfmFeature_ || scfFeature_ || 
      chromaFeature_) {
    MarSystem* spectralSeries = mng.create("Series", "spectralSeries");
    spectralSeries->addMarSystem(mng.create("Spectrum", "spk"));
    spectralSeries->addMarSystem(mng.create("PowerSpectrum", "pspk"));
    
    MarSystem* spectralFanout = mng.create("Fanout", "spectralFanout");
    if (mfccFeature_) {
      spectralFanout->addMarSystem(mng.create("MFCC", "mfcc"));
      spectralFanout->updControl("MFCC/mfcc/mrs_natural/coefficients", numMfccs_);
    }
    
    if (centroidFeature_) {
      spectralFanout->addMarSystem(mng.create("Centroid", "centroid"));
    }
    
    if (rolloffFeature_) {
      spectralFanout->addMarSystem(mng.create("Rolloff", "rolloff"));
    }
    
    if (fluxFeature_) {
      spectralFanout->addMarSystem(mng.create("Flux", "flux"));
    }

    if (kurtosisFeature_) {
      spectralFanout->addMarSystem(mng.create("Kurtosis", "kurtosis"));
    }

    if (skewnessFeature_) {
      spectralFanout->addMarSystem(mng.create("Skewness", "skewness"));
    }

    if (sfmFeature_) {
      spectralFanout->addMarSystem(mng.create("SFM", "sfm"));
    }

    if (scfFeature_) {
      spectralFanout->addMarSystem(mng.create("SCF", "scf"));
    }

    if (chromaFeature_) {
      MarSystem* chromaPrSeries = mng.create("Series", "chromaPrSeries");
      chromaPrSeries->addMarSystem(mng.create("Spectrum2Chroma", "chroma"));
      chromaPrSeries->addMarSystem(mng.create("PeakRatio","pr"));
      spectralFanout->addMarSystem(chromaPrSeries);
    }

    spectralSeries->addMarSystem(spectralFanout);
    mainFanout->addMarSystem(spectralSeries);
  }
  net->addMarSystem(mainFanout);

  if (wekaFilename_ != EMPTYSTRING) {
    net->addMarSystem(mng.create("Annotator", "annotator"));
	net->addMarSystem(mng.create("WekaSink", "wsink"));
  }
  

  if(memSize_ != 0) {
    net->addMarSystem(mng.create("TextureStats", "tStats"));
    net->updControl("TextureStats/tStats/mrs_natural/memSize", memSize_);
  }

  net->linkControl("mrs_real/currentLabel",
                   "SoundFileSource/src/mrs_real/currentLabel");
  net->linkControl("mrs_natural/nLabels",
                   "SoundFileSource/src/mrs_natural/nLabels");
  net->linkControl("mrs_string/labelNames",
                   "SoundFileSource/src/mrs_string/labelNames");
  net->linkControl("mrs_string/currentlyPlaying",
                   "SoundFileSource/src/mrs_string/currentlyPlaying");

  net->linkControl("WekaSink/wsink/mrs_string/currentlyPlaying",
                   "mrs_string/currentlyPlaying");
  net->linkControl("WekaSink/wsink/mrs_string/labelNames",
                   "mrs_string/labelNames");
  net->linkControl("WekaSink/wsink/mrs_natural/nLabels",
                   "mrs_natural/nLabels");
  net->linkControl("Annotator/annotator/mrs_real/label",
                   "mrs_real/currentLabel");

  net->updControl("SoundFileSource/src/mrs_string/filename", inCollectionName);
  net->updControl("mrs_natural/inSamples", hopSize_);
  net->updControl("ShiftInput/si/mrs_natural/winSize", winSize_);
  net->updControl("WekaSink/wsink/mrs_string/filename", wekaFilename_);


  ofstream ofs;
  if (outputFilename_ != EMPTYSTRING) {
    ofs.open(outputFilename_.c_str());
  }

  mrs_realvec data;
  int numTicks = 0;
  float currentTime;
  float sampleRate = net->getctrl("SoundFileSource/src/mrs_real/osrate")->to<mrs_real>();

  mrs_string previouslyPlaying, currentlyPlaying;
  MarControlPtr ctrl_previouslyPlaying = net->getctrl("mrs_string/previouslyPlaying");
  MarControlPtr ctrl_currentlyPlaying = net->getctrl("mrs_string/currentlyPlaying");

  int i = 0;
  while ( net->getctrl("SoundFileSource/src/mrs_bool/hasData")->to<mrs_bool>() ){
    net->tick();
    numTicks++;

    data = net->getctrl("mrs_realvec/processedData")->to<mrs_realvec>();
    if (wekaFilename_ != EMPTYSTRING) {
      net->updControl("WekaSink/wsink/mrs_string/injectComment", "% filename " + currentlyPlaying);
    }

    currentTime = (numTicks * hopSize_) / sampleRate;

    if (outputFilename_ != EMPTYSTRING) {
      if (i % downsample_ == 0) {
        if (timeOutput_) {
          ofs << currentTime << " ";
        } else {
          ofs << net->getctrl("SoundFileSource/src/mrs_real/currentLabel")->to<mrs_real>() << " ";
        }

        for (int i = 0; i < data.getRows(); i++) {

          if (outputFormat_ == "libsvm") {
            ofs << i+1 << ":";
          }
          ofs << data(i, 0) << " ";
        }
        ofs << endl;
      }
    }

    i++;
  }

  if (outputFilename_ != EMPTYSTRING) {
    ofs.close();
  }


}
  

int
main(int argc, const char **argv)
{
  string progName = argv[0];

  if (argc == 1)
    {
      printUsage(progName);
      return 0;
    }

  initOptions();
  cmdOptions_.readOptions(argc, argv);
  loadOptions();

  if (helpOpt_)
    return printHelp(progName);

  if (usageOpt_)
    return printUsage(progName);

  cout << endl;
  cout << "Window Size (in samples): " << winSize_ << endl;
  cout << "Hop Size (in samples): " << hopSize_ << endl;
  cout << "Memory Size (in analysis windows):" << memSize_ << endl;

  vector<string> soundfiles = cmdOptions_.getRemaining();
  vector<string>::iterator sfi;

  for (sfi = soundfiles.begin(); sfi != soundfiles.end(); ++sfi) {
    string sfname = *sfi;
    extract(sfname);
  }

  exit(0);

}
