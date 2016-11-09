// For an explanation of this class, look at its header,
// HTG710FixedDMA.hh, as well as
// https://cdcvs.fnal.gov/redmine/projects/artdaq-demo/wiki/Fragments_and_FragmentGenerators_w_Toy_Fragments_as_Examples

#include "artdaq-demo/Generators/HTG710FixedDMA.hh"

#ifdef CANVAS
#include "canvas/Utilities/Exception.h"
#else
#include "art/Utilities/Exception.h"
#endif

#include "artdaq/Application/GeneratorMacros.hh"
#include "artdaq-core/Utilities/SimpleLookupPolicy.h"

#include "artdaq-core-demo/Overlays/HTG710FixedDMAFragment.hh"
#include "artdaq-core-demo/Overlays/FragmentType.hh"
#include "HTG710FixedDMAHardwareInterface/HTG710FixedDMAHardwareInterface.hh"


#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <iostream>

#include <unistd.h>


demo::HTG710FixedDMA::HTG710FixedDMA(fhicl::ParameterSet const & ps)
  :
  CommandableFragmentGenerator(ps),
  hardware_interface_( new HTG710FixedDMAHardwareInterface(ps) ),
  timestamp_(0),
  timestampScale_(ps.get<int>("timestamp_scale_factor", 1)),
  readout_buffer_(nullptr),
  fragment_type_(static_cast<decltype(fragment_type_)>( artdaq::Fragment::InvalidFragmentType ))
{

  std::cout << "HTG710FixedDMA_generator.cc::constructor. BoardType() is " << hardware_interface_->BoardType() << std::endl;
  hardware_interface_->AllocateReadoutBuffer(&readout_buffer_);   

  metadata_.board_serial_number = hardware_interface_->SerialNumber();
  metadata_.num_adc_bits = hardware_interface_->NumADCBits();

  // 1000 is added in HTG710FixedDMAHardwareInterface::BoardType()
  switch (hardware_interface_->BoardType()) {
  case 1006: // as assigned by m.f.'ing artdaq-core-demo's FragmentType.cc
    fragment_type_ = toFragmentType("HTG710FixedDMA");
    break;
  case 1007:
    fragment_type_ = toFragmentType("TOY");
    break;
  default:
    std::cout << "HTG710FixedDMA_generator.cc::constructor2. BoardType() is " << hardware_interface_->BoardType() << std::endl;
    throw cet::exception("HTG710FixedDMA") << "Unable to determine board type supplied by hardware";
  }
}

demo::HTG710FixedDMA::~HTG710FixedDMA() {
  hardware_interface_->FreeReadoutBuffer(readout_buffer_);
}

bool demo::HTG710FixedDMA::getNext_(artdaq::FragmentPtrs & frags) {

  if (should_stop()) {
    return false;
  }

  // ToyHardwareInterface (an instance to which "hardware_interface_"
  // is a unique_ptr object) is just one example of the sort of
  // interface a hardware library might offer. For example, other
  // interfaces might require you to allocate and free the memory used
  // to store hardware data in your generator using standard C++ tools
  // (rather than via the "AllocateReadoutBuffer" and
  // "FreeReadoutBuffer" functions provided here), or could have a
  // function which directly returns a pointer to the data buffer
  // rather than sticking the data in the location pointed to by your
  // pointer (which is what happens here with readout_buffer_)

  std::size_t bytes_read = 0;
  hardware_interface_->FillBuffer(readout_buffer_ , &bytes_read);


  std::unique_ptr<artdaq::Fragment> fragptr(
   					    artdaq::Fragment::FragmentBytes(bytes_read,  
   									    ev_counter(), fragment_id(),
   									    fragment_type_, 
										metadata_, timestamp_));

  memcpy(fragptr->dataBeginBytes(), readout_buffer_, bytes_read );

  frags.emplace_back( std::move(fragptr ));

  if(metricMan_ != nullptr) {
    metricMan_->sendMetric("Fragments Sent",ev_counter(), "Events", 3);
  }

  ev_counter_inc();
  timestamp_ += timestampScale_;

  return true;
}

void demo::HTG710FixedDMA::start() {
  hardware_interface_->StartDatataking();
}

void demo::HTG710FixedDMA::stop() {
  hardware_interface_->StopDatataking();
}

// The following macro is defined in artdaq's GeneratorMacros.hh header
DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(demo::HTG710FixedDMA) 
