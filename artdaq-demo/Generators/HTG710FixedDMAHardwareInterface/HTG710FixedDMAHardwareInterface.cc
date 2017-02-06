#include "artdaq-demo/Generators/HTG710FixedDMAHardwareInterface/HTG710FixedDMAHardwareInterface.hh"
#include "artdaq-core-demo/Overlays/HTG710FixedDMAFragment.hh"
#include "artdaq-core-demo/Overlays/FragmentType.hh"

#include "fhiclcpp/ParameterSet.h"
#include "cetlib/exception.h"

#include <random>
#include <unistd.h>
#include <iostream>

//#include <sstream>
//#include <iterator>

// JCF, Mar-17-2016

// HTG710FixedDMAHardwareInterface is meant to mimic a vendor-provided hardware
// API, usable within the the ToySimulator fragment generator. For
// purposes of realism, it's a C++03-style API, as opposed to, say, one
// based in C++11 capable of taking advantage of smart pointers, etc.

HTG710FixedDMAHardwareInterface::HTG710FixedDMAHardwareInterface(fhicl::ParameterSet const & ps) :
  taking_data_(false),
  nADCcounts_(ps.get<size_t>("nADCcounts", 40)), 
  maxADCcounts_(ps.get<size_t>("maxADCcounts", 50000000)),
  change_after_N_seconds_(ps.get<size_t>("change_after_N_seconds", 
					 std::numeric_limits<size_t>::max())),
  nADCcounts_after_N_seconds_(ps.get<int>("nADCcounts_after_N_seconds",
					     nADCcounts_)),
  fragment_type_(demo::toFragmentType(ps.get<std::string>("fragment_type"))), 
  maxADCvalue_(pow(2, NumADCBits() ) - 1), // MUST be after "fragment_type"
  throttle_usecs_(ps.get<size_t>("throttle_usecs", 100000)),
  distribution_type_(static_cast<DistributionType>(ps.get<int>("distribution_type"))),
  engine_(ps.get<int64_t>("random_seed", 314159)),
  uniform_distn_(new std::uniform_int_distribution<data_t>(0, maxADCvalue_)),
  gaussian_distn_(new std::normal_distribution<double>( 0.5*maxADCvalue_, 0.1*maxADCvalue_)),
  start_time_(std::numeric_limits<decltype(std::chrono::high_resolution_clock::now())>::max())
{


    std::cout << "HTG710FixedDMAHardwareInterface constructor.EC fragment_type_ is " <<  fragment_type_ << std::endl;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"

  // JCF, Aug-14-2016 

  // The logic of checking that nADCcounts_after_N_seconds_ >= 0,
  // below, is because signed vs. unsigned comparison won't do what
  // you want it to do if nADCcounts_after_N_seconds_ is negative

  if (nADCcounts_ > maxADCcounts_ ||
      (nADCcounts_after_N_seconds_ >= 0 && nADCcounts_after_N_seconds_ > maxADCcounts_)) {
    throw cet::exception("HardwareInterface") << "Either (or both) of \"nADCcounts\" and \"nADCcounts_after_N_seconds\"" <<
      " is larger than the \"maxADCcounts\" setting (currently at " << maxADCcounts_ << ")";
  }

  if (nADCcounts_after_N_seconds_ != nADCcounts_ && 
      change_after_N_seconds_ == std::numeric_limits<size_t>::max()) {
    throw cet::exception("HardwareInterface") << "If \"nADCcounts_after_N_seconds\""
					      << " is set, then \"change_after_N_seconds\" should be set as well";

#pragma GCC diagnostic pop
      
  }



}

// JCF, Mar-18-2017

// "StartDatataking" is meant to mimic actions one would take when
// telling the hardware to start sending data - the uploading of
// values to registers, etc.

void HTG710FixedDMAHardwareInterface::StartDatataking() {
  taking_data_ = true;
  start_time_ = std::chrono::high_resolution_clock::now();
}

void HTG710FixedDMAHardwareInterface::StopDatataking() {
  taking_data_ = false;
  start_time_ = std::numeric_limits<decltype(std::chrono::high_resolution_clock::now())>::max();
}


void HTG710FixedDMAHardwareInterface::FillBuffer(char* buffer, size_t* bytes_read) {

  if (taking_data_) {

    usleep( throttle_usecs_ );

    auto elapsed_secs_since_datataking_start = 
      std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now()
						       - start_time_).count();

    char** argv(0);
    std::string argvs(""); // presuming I won't pass any arguments
    std::stringstream sstream(argvs);
    int len = std::distance(std::istream_iterator<std::string>(sstream), std::istream_iterator<std::string>());
    int argc(len); 

    // Note that this method is meant for a fake hardware interface in which one wants to place fake generated data 
    // into the buffer, which ordinarily comes from the hardware and is thus to remain inviolate. In real life one
    // wouldn't do that. Hence, to write into that hardware buffer we first must memcpy it here. This extra memcpy
    // is time we don't want to spend ordinarily, among other principles being sacrificed here.

    felix_.get_data(argc,argv);
    // This memcpy is mandatory. We crash below at header->anything = blah  with instead the below line.
    // buffer = (char *)(felix_.vaddr);

    //    *bytes_read = BUFSIZE; // sizeof();  // FIXME, EC, 5-Feb-2017

    std::memcpy(buffer, (char*)felix_.vaddr, *bytes_read);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
    if (elapsed_secs_since_datataking_start < change_after_N_seconds_) {
#pragma GCC diagnostic pop

      *bytes_read = sizeof(demo::HTG710FixedDMAFragment::Header) + nADCcounts_ * sizeof(data_t);
    } else {
      if (nADCcounts_after_N_seconds_ >= 0) {
	*bytes_read = sizeof(demo::HTG710FixedDMAFragment::Header) + nADCcounts_after_N_seconds_ * sizeof(data_t);
      } else {
	// Pretend the hardware hangs
	while (true) {
	}
      }
    }
      
    // Make the fake data, starting with the header

    // Can't handle a fragment whose size isn't evenly divisible by
    // the demo::ToyFragment::Header::data_t type size in bytes
    std::cout << "Bytes to read: " << *bytes_read << ", sizeof(data_t): " << sizeof(demo::HTG710FixedDMAFragment::Header::data_t) << std::endl;
    assert( *bytes_read % sizeof(demo::HTG710FixedDMAFragment::Header::data_t) == 0 );


    // Again, this is dumb for a real hardware buffer. We are stuffing a header with values, and blithely acting as if our buffer 
    // has come with a header at the top of known size. We have succeeding in stomping on the data from the HTG710. Boo.
    demo::HTG710FixedDMAFragment::Header* header = reinterpret_cast<demo::HTG710FixedDMAFragment::Header*>(buffer);

    header->event_size = *bytes_read / sizeof(demo::HTG710FixedDMAFragment::Header::data_t) ;
    header->trigger_number = 12;

    // Generate nADCcounts ADC values ranging from 0 to max based on
    // the desired distribution


    /*

    std::function<data_t()> generator;

    switch (distribution_type_) {
    case DistributionType::uniform:
      generator = [&]() {
	return static_cast<data_t>
	((*uniform_distn_)( engine_ ));
      };
      break;

    case DistributionType::gaussian:
      generator = [&]() {

	data_t gen(0);
	do {
	  gen = static_cast<data_t>( std::round( (*gaussian_distn_)( engine_ ) ) );
	} 
	while(gen > maxADCvalue_);                                                                    
	return gen;
      };
      break;

    case DistributionType::monotonic:
      {
	data_t increasing_integer = 0;
	generator = [&]() {
	  increasing_integer++;
	  return increasing_integer > maxADCvalue_ ? 999 : increasing_integer;
	};
      }
      break;

    case DistributionType::uninitialized:
      break;

    default:
      throw cet::exception("HardwareInterface") <<
	"Unknown distribution type specified";
    }

    if (distribution_type_ != DistributionType::uninitialized) {
      // EC: This is the money call that fills buffer 1 byte past header for nADCcounts.
      std::generate_n(reinterpret_cast<data_t*>( reinterpret_cast<demo::HTG710FixedDMAFragment::Header*>(buffer) + 1 ), 
		      nADCcounts_,
		      generator
		      );
    }

    */

    memcpy (reinterpret_cast<data_t*>( reinterpret_cast<demo::HTG710FixedDMAFragment::Header*>(buffer) + 1 ), buffer, *bytes_read);


    // if (taking_data)
  } else {
    throw cet::exception("HTG710FixedDMAHardwareInterface") <<
      "Attempt to call FillBuffer when not sending data";
  }



}

void HTG710FixedDMAHardwareInterface::AllocateReadoutBuffer(char** buffer) {
  
  *buffer = reinterpret_cast<char*>( new uint8_t[ sizeof(demo::HTG710FixedDMAFragment::Header) + maxADCcounts_*sizeof(data_t) ] );
}

void HTG710FixedDMAHardwareInterface::FreeReadoutBuffer(char* buffer) {
  delete [] buffer;
}

// Pretend that the "BoardType" is some vendor-defined integer which
// differs from the fragment_type_ we want to use as developers (and
// which must be between 1 and 224, inclusive) so add an offset

int HTG710FixedDMAHardwareInterface::BoardType() const {
  return static_cast<int>(fragment_type_) + 1000;
}

int HTG710FixedDMAHardwareInterface::NumADCBits() const {

  std::cout << "NumADCBits().EC fragment_type_ is " <<  fragment_type_ << std::endl;
  switch (fragment_type_) {
  case demo::FragmentType::TOY1:
    return 12;
    break;
  case demo::FragmentType::TOY2:
    return 14;
    break;
  case demo::FragmentType::HTG710FIXEDDMA:
    return 14; 
    break;
  default:
    throw cet::exception("HTG710FixedDMAHardwareInterface")
      << "Unknown board type "
      << fragment_type_
      << " ("
      << demo::fragmentTypeToString(fragment_type_)
      << ").\n";
  };

}

int HTG710FixedDMAHardwareInterface::SerialNumber() const {
  return 71; // EC: making up this number
}

