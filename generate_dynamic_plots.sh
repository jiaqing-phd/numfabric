# run the three algorithms to generate flow arrivals according to the load specified in the config file
python run_config.py examples/xfabric/ls_dynamic dynamic_configs/config_numfabric_websearch
python run_config.py examples/xfabric/ls_dynamic dynamic_configs/config_rcp_websearch
python run_config.py examples/xfabric/ls_dynamic dynamic_configs/config_numfabric_websearch

#calculate ideal rates
./get_ideal_dynamic_rates.sh numfabric_websearch.out > oracle_rates_websearch.out 2> oracle_rates_websearch.err

# calculate the deviations
python get_rate_error_vectors.py numfabric_websearch.out oracle_rates_websearch.out > numfabric_websearch_errors
python get_rate_error_vectors.py dgd_websearch.out oracle_rates_websearch.out > dgd_websearch_errors
python get_rate_error_vectors.py rcp_websearch.out oracle_rates_websearch.out > rcp_websearch_errors

#plot the error vectors
python combined_err_bdp.py numfabric_websearch_errors dgd_websearch_errors rcp_websearch_errors websearch_errors

## NOTE: TAKES HOURS TO RUN. 

## NOTE: It is important to run this experiment long enough to generate atleast ~50K flows so that the
## the system ramps up the load specified (80% link load) in this case
## Lower number of flows don't stress the system enough and desired propoerties may not be exhibited
