////////////////////////////////////////////////////////////////////////
// Class:       ToyDump
// Module Type: analyzer
// File:        ToyDump_module.cc
// Description: Prints out information about each event.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"

#include "art/Utilities/Exception.h"
#include "artdaq-core-demo/Overlays/ToyFragment.hh"
#include "artdaq-core/Data/Fragments.hh"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>

namespace demo {
  class ToyDump;
}

class demo::ToyDump : public art::EDAnalyzer {
public:
  explicit ToyDump(fhicl::ParameterSet const & pset);
  virtual ~ToyDump();

  virtual void analyze(art::Event const & evt);

private:
  std::string raw_data_label_;
  std::string frag_type_;
  uint32_t num_adcs_to_show_;
  bool dump_to_file_;
  bool dump_to_screen_;
  uint32_t columns_to_display_on_screen_;
};


demo::ToyDump::ToyDump(fhicl::ParameterSet const & pset)
    : EDAnalyzer(pset),
      raw_data_label_(pset.get<std::string>("raw_data_label")),
      frag_type_(pset.get<std::string>("frag_type")),
      num_adcs_to_show_(pset.get<uint32_t>("num_adcs_to_show", 0)),
      dump_to_file_(pset.get<bool>("dump_to_file", true)),
      dump_to_screen_(pset.get<bool>("dump_to_screen", false)),
      columns_to_display_on_screen_(pset.get<uint32_t>("columns_to_display_on_screen", 10))
{
}

demo::ToyDump::~ToyDump()
{
}

void demo::ToyDump::analyze(art::Event const & evt)
{
  art::EventNumber_t eventNumber = evt.event();

  // ***********************
  // *** Toy Fragments ***
  // ***********************

  // look for raw Toy data

  art::Handle<artdaq::Fragments> raw;
  evt.getByLabel(raw_data_label_, frag_type_, raw);

  if (raw.isValid()) {
    std::cout << "######################################################################" << std::endl;
    std::cout << std::endl;
    std::cout << "Run " << evt.run() << ", subrun " << evt.subRun()
              << ", event " << eventNumber << " has " << raw->size()
              << " fragment(s) of type " << frag_type_ << std::endl;

    for (size_t idx = 0; idx < raw->size(); ++idx) {
      const auto& frag((*raw)[idx]);

      ToyFragment bb(frag);

      std::cout << std::endl;
      std::cout << "Toy fragment " << frag.fragmentID() << " has total ADC counts = " 
		<< bb.total_adc_values() << std::endl;
      //std::cout << std::endl;

      if (frag.hasMetadata()) {
      std::cout << std::endl;
	std::cout << "Fragment metadata: " << std::endl;
        ToyFragment::Metadata const* md =
          frag.metadata<ToyFragment::Metadata>();
        std::cout << std::showbase << "Board serial number = "
                  << ((int)md->board_serial_number) << ", sample bits = "
                  << ((int)md->num_adc_bits)
		  << " -> max ADC value = " 
		  << bb.adc_range( (int)md->num_adc_bits )
                  << std::endl;
	//std::cout << std::endl;
      }

      if (num_adcs_to_show_ == 0) {
        num_adcs_to_show_ = bb.total_adc_values();
      }

      if (num_adcs_to_show_ > 0) {

	if (num_adcs_to_show_ > bb.total_adc_values() ) {
	  throw cet::exception("num_adcs_to_show is larger than total number of adcs in fragment");
	} else {
	  std::cout << std::endl;
	  std::cout << "First " << num_adcs_to_show_ 
		    << " ADC values in the fragment: " 
		    << std::endl;
	}

        if (dump_to_file_) {
          std::ofstream output ("out.bin", std::ios::out | std::ios::app | std::ios::binary );
          for (uint32_t i_adc = 0; i_adc < num_adcs_to_show_; ++i_adc) {
            output.write((char*)(bb.dataBegin() + i_adc),sizeof(ToyFragment::adc_t));
          }
          output.close();
        }

        if (dump_to_screen_) {
          std::cout << std::right;
          int rows = 1 + (int) ((num_adcs_to_show_ - 1) / columns_to_display_on_screen_);
          uint32_t adc_counter = 0;
          for (int idx = 0; idx < rows; ++idx) {
            std::cout << std::setw(4) << std::setfill('.');
            std::cout << (idx * columns_to_display_on_screen_) << ": ";
            for (uint32_t jdx = 0; jdx < columns_to_display_on_screen_; ++jdx) {
              if (adc_counter >= num_adcs_to_show_) {break;}
              std::cout << std::setw(6) << std::setfill(' ');
              std::cout << (bb.dataBegin()[adc_counter]);
              ++adc_counter;
            }
            std::cout << std::endl;
          }
        }

	std::cout << std::endl;
      }
    }
  }
  else {
    std::cout << "Run " << evt.run() << ", subrun " << evt.subRun()
              << ", event " << eventNumber << " has zero"
              << " Toy fragments." << std::endl;
  }
  std::cout << std::endl;

}

DEFINE_ART_MODULE(demo::ToyDump)
