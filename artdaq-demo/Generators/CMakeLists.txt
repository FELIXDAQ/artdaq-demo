simple_plugin(V172xFileReader "generator"
  artdaq-core-demo_Overlays
  artdaq_Application
  artdaq_DAQdata
  artdaq-core_Utilities
  artdaq-core_Data
  ${CANVAS}_Utilities
  ${FHICLCPP}
  ${CETLIB}
  )

simple_plugin(V172xSimulator "generator"
  artdaq-core-demo_Overlays
  artdaq_Application
  artdaq_DAQdata
  artdaq-core_Utilities
  artdaq-core_Data
  ${CANVAS}_Utilities
  ${FHICLCPP}
  ${CETLIB}
  )

simple_plugin(HTG710FixedDMA "generator"
  artdaq-demo_Generators_HTG710FixedDMAHardwareInterface
  artdaq-core-demo_Overlays
  artdaq_Application
  artdaq_DAQdata
  artdaq-core_Utilities
  artdaq-core_Data
  ${CANVAS}_Utilities
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  ${CETLIB}
  FlxCard
  DFDebug
  regmap2
  cmem_rcc
  rcc_error
  )

simple_plugin(AsciiSimulator "generator"
  artdaq-core-demo_Overlays
  artdaq_Application
  artdaq_DAQdata
  artdaq-core_Utilities
  artdaq-core_Data
  ${FHICLCPP}
  ${CETLIB}
  )

simple_plugin(UDPReceiver "generator"
    artdaq-core-demo_Overlays
    artdaq_Application
    artdaq_DAQdata
    artdaq-core_Utilities
    artdaq-core_Data
  ${CANVAS}_Utilities
    ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
    ${CETLIB}
    )

add_subdirectory(ToyHardwareInterface)
add_subdirectory(HTG710FixedDMAHardwareInterface)
add_subdirectory(flx)
