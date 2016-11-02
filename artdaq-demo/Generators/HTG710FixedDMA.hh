#ifndef artdaq_demo_Generators_HTG710FixedDMA_hh
#define artdaq_demo_Generators_HTG710FixedDMA_hh

// HTG710FixedDMA is a simple type of fragment generator intended to be
// studied by new users of artdaq as an example of how to create such
// a generator in the "best practices" manner. Derived from artdaq's
// CommandableFragmentGenerator class, it can be used in a full DAQ
// simulation, obtaining data from the ToyHardwareInterface class

// HTG710FixedDMA is designed to simulate values coming in from one of
// two types of digitizer boards, one called "TOY1" and the other
// called "TOY2"; the only difference between the two boards is the #
// of bits in the ADC values they send. These values are declared as
// FragmentType enum's in artdaq-demo's
// artdaq-core-demo/Overlays/FragmentType.hh header.

// Some C++ conventions used:

// -Append a "_" to every private member function and variable

#include "fhiclcpp/fwd.h"
#include "artdaq-core/Data/Fragments.hh" 
#include "artdaq/Application/CommandableFragmentGenerator.hh"
#include "artdaq-core-demo/Overlays/HTG710FixedDMAFragment.hh"
#include "artdaq-core-demo/Overlays/FragmentType.hh"
#include "HTG710FixedDMAHardwareInterface/HTG710FixedDMAHardwareInterface.hh"

#include "HTG710FixedDMA.hh"

#include <random>
#include <vector>
#include <atomic>

namespace demo {    

  class HTG710FixedDMA : public artdaq::CommandableFragmentGenerator {
  public:
    explicit HTG710FixedDMA(fhicl::ParameterSet const & ps);
    ~HTG710FixedDMA();

  private:

    // The "getNext_" function is used to implement user-specific
    // functionality; it's a mandatory override of the pure virtual
    // getNext_ function declared in CommandableFragmentGenerator

    bool getNext_(artdaq::FragmentPtrs & output) override;

    // The start, stop and stopNoMutex methods are declared pure
    // virtual in CommandableFragmentGenerator and therefore MUST be
    // overridden; note that stopNoMutex() doesn't do anything here

    void start() override;
    void stop() override;
    void stopNoMutex() override {}

    std::unique_ptr<HTG710FixedDMAHardwareInterface> hardware_interface_;
    artdaq::Fragment::timestamp_t timestamp_;
    int timestampScale_;

    HTG710FixedDMAFragment::Metadata metadata_;

    // buffer_ points to the buffer which the hardware interface will
    // fill. Notice that it's a raw pointer rather than a smart
    // pointer as the API to ToyHardwareInterface was chosen to be a
    // C++03-style API for greater realism

    char* readout_buffer_;

    FragmentType fragment_type_;

  };
}

#endif /* artdaq_demo_Generators_HTG710FixedDMA_hh */
