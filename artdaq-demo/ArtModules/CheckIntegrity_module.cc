////////////////////////////////////////////////////////////////////////
// Class:       CheckIntegrity
// Module Type: analyzer
// File:        CheckIntegrity_module.cc
// Description: Prints out information about each event.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"

#ifdef CANVAS
#include "canvas/Utilities/Exception.h"
#else
#include "art/Utilities/Exception.h"
#endif

#include "artdaq-core-demo/Overlays/ToyFragment.hh"
#include "artdaq-core/Data/Fragments.hh"

#include "messagefacility/MessageLogger/MessageLogger.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>

namespace demo {
  class CheckIntegrity;
}

class demo::CheckIntegrity : public art::EDAnalyzer {
public:
  explicit CheckIntegrity(fhicl::ParameterSet const & pset);
  virtual ~CheckIntegrity() = default;

  virtual void analyze(art::Event const & evt);

private:
  std::string raw_data_label_;
  std::string frag_type_;
};


demo::CheckIntegrity::CheckIntegrity(fhicl::ParameterSet const & pset)
    : EDAnalyzer(pset),
      raw_data_label_(pset.get<std::string>("raw_data_label")),
      frag_type_(pset.get<std::string>("frag_type"))
{
}

void demo::CheckIntegrity::analyze(art::Event const & evt)
{

  art::Handle<artdaq::Fragments> raw;
  evt.getByLabel(raw_data_label_, frag_type_, raw);

  if (raw.isValid()) {

    for (size_t idx = 0; idx < raw->size(); ++idx) {
      const auto& frag((*raw)[idx]);

      ToyFragment bb(frag);

      {
	auto adc_iter = bb.dataBeginADCs();
	ToyFragment::adc_t expected_adc = 1; 

	for ( ; adc_iter != bb.dataEndADCs(); adc_iter++, expected_adc++) {
	  if (*adc_iter != expected_adc) {
	    mf::LogError("CheckIntegrity") << "Error: in run " << evt.run() << ", subrun " << evt.subRun() <<
	      ", event " << evt.event() << ", seqID " << frag.sequenceID() <<
	      ", fragID " << frag.fragmentID() << ": expected an ADC value of " << expected_adc << 
	      ", got " << *adc_iter;
	    return;
	  }
	}

	mf::LogDebug("CheckIntegrity") << "In run " << evt.run() << ", subrun " << evt.subRun() <<
	  ", event " << evt.event() << ", everything is fine";
      }
    }
  }
  else {
    mf::LogError("CheckIntegrity") << "In run " << evt.run() << ", subrun " << evt.subRun() <<
      ", event " << evt.event() << ", raw.isValid() returned false";
  }

}

DEFINE_ART_MODULE(demo::CheckIntegrity)
