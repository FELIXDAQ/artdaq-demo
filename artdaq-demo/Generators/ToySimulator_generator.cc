#include "artdaq-demo/Generators/ToySimulator.hh"

#include "art/Utilities/Exception.h"
#include "artdaq/Application/GeneratorMacros.hh"
#include "cetlib/exception.h"
#include "artdaq-demo/Overlays/ToyFragment.hh"
#include "artdaq-demo/Overlays/ToyFragmentWriter.hh"
#include "artdaq-demo/Overlays/FragmentType.hh"
#include "fhiclcpp/ParameterSet.h"
#include "artdaq/Utilities/SimpleLookupPolicy.h"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <iostream>

#include <unistd.h>

namespace {

  size_t typeToADC(demo::FragmentType type)
  {
    switch (type) {
    case demo::FragmentType::TOY1:
      return 12;
      break;
    case demo::FragmentType::TOY2:
      return 14;
      break;
    default:
      throw art::Exception(art::errors::Configuration)
        << "Unknown board type "
        << type
        << " ("
        << demo::fragmentTypeToString(type)
        << ").\n";
    };
  }

}



demo::ToySimulator::ToySimulator(fhicl::ParameterSet const & ps)
  :
  CommandableFragmentGenerator(ps),
  nADCcounts_(ps.get<size_t>("nADCcounts", 600000)),
  fragment_type_(toFragmentType(ps.get<std::string>("fragment_type"))),
  throttle_usecs_(ps.get<size_t>("throttle_usecs", 0)),
  engine_(ps.get<int64_t>("random_seed", 314159)),
  uniform_distn_(new std::uniform_int_distribution<int>(0, pow(2, typeToADC( fragment_type_ ) ) - 1 ))
{

  // Check and make sure that the fragment type will be one of the "toy" types
  
  std::vector<artdaq::Fragment::type_t> const ftypes = 
    {FragmentType::TOY1, FragmentType::TOY2 };

  if (std::find( ftypes.begin(), ftypes.end(), fragment_type_) == ftypes.end() ) {
    throw cet::exception("Error in ToySimulator: unexpected fragment type supplied to constructor");
  }
    
}


bool demo::ToySimulator::getNext_(artdaq::FragmentPtrs & frags) {

  if (should_stop()) {
    return false;
  }

  usleep( throttle_usecs_ );

  // Set fragment's metadata

  ToyFragment::Metadata metadata;
  metadata.board_serial_number = 999;
  metadata.num_adc_bits = typeToADC(fragment_type_);

  // And use it, along with the artdaq::Fragment header information
  // (fragment id, sequence id, and user type) to create a fragment

  // We'll use the static factory function 

  // artdaq::Fragment::FragmentBytes(std::size_t payload_size_in_bytes, sequence_id_t sequence_id,
  //  fragment_id_t fragment_id, type_t type, const T & metadata)

  // which will then return a unique_ptr to an artdaq::Fragment
  // object. The advantage of this approach over using the
  // artdaq::Fragment constructor is that, if we were to want to
  // initialize the artdaq::Fragment with a nonzero-size payload (data
  // after the artdaq::Fragment header and metadata), we could provide
  // the size of the payload in bytes, rather than in units of the
  // artdaq::Fragment's RawDataType (8 bytes, as of 3/26/14). The
  // artdaq::Fragment constructor itself was not altered so as to
  // maintain backward compatibility.

  std::size_t initial_payload_size = 0;

  frags.emplace_back( artdaq::Fragment::FragmentBytes(initial_payload_size,  
						      ev_counter(), fragment_id(),
						      fragment_type_, metadata) );

  // Then any overlay-specific quantities next; will need the
  // ToyFragmentWriter class's setter-functions for this

  ToyFragmentWriter newfrag(*frags.back());

  newfrag.set_hdr_run_number(999);

  newfrag.resize(nADCcounts_);

  // And generate nADCcounts ADC values ranging from 0 to max with an
  // equal probability over the full range (a specific and perhaps
  // not-too-physical example of how one could generate simulated
  // data)

  std::generate_n(newfrag.dataBegin(), nADCcounts_,
  		  [&]() {
  		    return static_cast<ToyFragment::adc_t>
  		      ((*uniform_distn_)( engine_ ));
  		  }
  		  );

  // Check and make sure that no ADC values in this fragment are
  // larger than the max allowed

  newfrag.fastVerify( metadata.num_adc_bits );

  ev_counter_inc();

  return true;
}

// The following macro is defined in artdaq's GeneratorMacros.hh header
DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(demo::ToySimulator) 
