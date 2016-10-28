## Run the semidynamic use-case using the config files
python run_config.py examples/xfabric/ls_dctcp_arrivals semi_dynamic_config/config_numfabric
python run_config.py examples/xfabric/ls_dctcp_arrivals semi_dynamic_config/config_dgd
python run_config.py examples/xfabric/ls_dctcp_arrivals semi_dynamic_config/config_rcp

# Parse the results and plot the CDF

more numfabric_semidynamic.out | grep "95TH" | cut -f4 -d" " > nconverge
more rcp_semidynamic.out | grep "95TH" | cut -f4 -d" " > rconverge
more dgd_semidynamic.out | grep "95TH" | cut -f4 -d" " > dconverge
python plot_seabrn.py nconverge dconverge rconverge semidynamic_plot


#Optional 
## The "oracle" rates are required to calculate how long it took the system to converge to the right rates
## We have precalculated the oracle rates for a specific flow arrival pattern for alpha fair utility functions with
## different alphas - and stored them in the folder opt_rates (it is a config parameter in the config file where you specify
## optimal rates file to use ) The flow arrivals and departures are together specified in events_list, flow_arrivals and flow_departures
## files
### To generate oracle values for a new flow arrival/departure pattern, generate the flow arrivals/departures in ns3 logs once
## using 
### python run_config.py examples/xfabric/ls_more_arrivals semi_dynamic_config/config_numfabric_custom,
## where you define the custom config file
### Using the output file thus generated, use the script find_multiple_events.py to generate oracle flow rates

