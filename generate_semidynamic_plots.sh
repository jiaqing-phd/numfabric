python run_config.py examples/xfabric/ls_dctcp_arrivals semi_dynamic_config/config_numfabric
python run_config.py examples/xfabric/ls_dctcp_arrivals semi_dynamic_config/config_dgd
python run_config.py examples/xfabric/ls_dctcp_arrivals semi_dynamic_config/config_rcp
more numfabric_semidynamic.out | grep "95TH" | cut -f4 -d" " > nconverge
more rcp_semidynamic.out | grep "95TH" | cut -f4 -d" " > rconverge
more dgd_semidynamic.out | grep "95TH" | cut -f4 -d" " > dconverge
python plot_seabrn.py nconverge dconverge rconverge semidynamic_plot

