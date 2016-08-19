
def generateAggregator(totalFRs, totalEBs, bunchSize, fragSizeWords,
                       xmlrpcClientList, fileSizeThreshold, fileDuration,
                       fileEventCount, queueDepth, queueTimeout, onmonEventPrescale,
                       aggHost, aggPort, agType)

agConfig = String.new( "\
daq: {
  max_fragment_size_words: %{size_words}
  aggregator: {
    mpi_buffer_count: %{buffer_count}
    first_event_builder_rank: %{total_frs}
    event_builder_count: %{total_ebs}
    expected_events_per_bunch: %{bunch_size}
    print_event_store_stats: true
    event_queue_depth: %{queue_depth}
    event_queue_wait_time: %{queue_timeout}
    onmon_event_prescale: %{onmon_event_prescale}
    xmlrpc_client_list: \"%{xmlrpc_client_list}\"
    file_size_MB: %{file_size}
    file_duration: %{file_duration}
    file_event_count: %{file_event_count}
    %{ag_type_param_name}: true
  }

  metrics: {
    aggFile: {
      metricPluginType: \"file\"
      level: 3
      fileName: \"/tmp/aggregator/agg_%UID%_metrics.log\"
      uniquify: true
    }
  }

  transfer_to_dispatcher: {

    transferPluginType: shmem

    max_fragment_size_words: %{size_words}
    first_event_builder_rank: %{total_frs}
    
    \# Variables below this in the monitoring_transfer table are only relevant
    \# if transferPluginType, above, is set to multicast. You'll need to 
    \# figure out what the local_address to use for your system is

    multicast_address: \"224.0.0.1\"
    multicast_port: 30001   

    local_address: \"10.226.9.16\"  \# mu2edaq01
\#      local_address: \"10.226.9.19\"  \# mu2edaq05

    receive_buffer_size: 100000000

    subfragment_size: 6000
    subfragments_per_send: 10

  }

  transfer_to_monitors: {

    transferPluginType: multicast
    shm_key: 0x40470001

    max_fragment_size_words: %{size_words}
    first_event_builder_rank: %{total_frs}
    
    \# Variables below this in the monitoring_transfer table are only relevant
    \# if transferPluginType, above, is set to multicast. You'll need to 
    \# figure out what the local_address to use for your system is

    multicast_address: \"224.0.0.1\"
    multicast_port: 30001   

    local_address: \"10.226.9.16\"  \# mu2edaq01
    \#  local_address: \"10.226.9.19\"  \# mu2edaq05

    receive_buffer_size: 100000000

    subfragment_size: 6000
    subfragments_per_send: 10

  }

}" )

  agConfig.gsub!(/\%\{size_words\}/, String(fragSizeWords))
  agConfig.gsub!(/\%\{buffer_count\}/, String(totalEBs*4))
  agConfig.gsub!(/\%\{total_ebs\}/, String(totalEBs))
  agConfig.gsub!(/\%\{bunch_size\}/, String(bunchSize))  
  agConfig.gsub!(/\%\{queue_depth\}/, String(queueDepth))  
  agConfig.gsub!(/\%\{queue_timeout\}/, String(queueTimeout))  
  agConfig.gsub!(/\%\{onmon_event_prescale\}/, String(onmonEventPrescale))
  agConfig.gsub!(/\%\{xmlrpc_client_list\}/, String(xmlrpcClientList))
  agConfig.gsub!(/\%\{file_size\}/, String(fileSizeThreshold))
  agConfig.gsub!(/\%\{file_duration\}/, String(fileDuration))
  agConfig.gsub!(/\%\{file_event_count\}/, String(fileEventCount))
  if agType == "online_monitor"
    #agConfig.gsub!(/\%\{ag_type_param_name\}/, "is_online_monitor")
    agConfig.gsub!(/\%\{ag_type_param_name\}/, "is_dispatcher")
  else
    agConfig.gsub!(/\%\{ag_type_param_name\}/, "is_data_logger")
  end

  return agConfig
end
