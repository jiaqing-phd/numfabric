#python run_config.py examples/xfabric/ls_dynamic dynamic_configs/config_numfabric_websearch
#python run_config.py examples/xfabric/ls_dynamic dynamic_configs/config_rcp_websearch
#python run_config.py examples/xfabric/ls_dynamic dynamic_configs/config_numfabric_websearch

#calculate ideal rates
#./get_ideal_dynamic_rates.sh numfabric_websearch.out > oracle_rates_websearch.out 2> oracle_rates_websearch.err
python get_rate_error_vectors.py numfabric_websearch.out oracle_rates_websearch.out > numfabric_websearch_errors
python get_rate_error_vectors.py dgd_websearch.out oracle_rates_websearch.out > dgd_websearch_errors
python get_rate_error_vectors.py rcp_websearch.out oracle_rates_websearch.out > rcp_websearch_errors

python combined_err_bdp.py numfabric_websearch_errors dgd_websearch_errors rcp_websearch_errors websearch_errors
