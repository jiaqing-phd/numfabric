base_dir=/cluster/u/csandeep/scratch/knagaraj/FINAL_DATA/20150909
base_dir=$PWD/xfabric_util2_test
base_dir=ss_xf2

results_file=$base_dir.results.txt
min_results=$base_dir.min.results.txt
collated_results_min=$base_dir.min.collated.txt
collated_results_max=$base_dir.max.collated.txt
rm $results_file
rm $min_results
rm $collated_results_min
rm $collated_results_max

min_plot=min.plot.jpg
max_plot=max.plot.jpg

for alpha in 0.1 0.125 0.25 0.5 1.0 2.0 4.0 8.0; do
	base_file=$base_dir'_'$alpha.out

	echo "alpha $alpha" >> $results_file
	echo "alpha $alpha" >> $min_results

	cat $base_file | grep "TotalRate" | awk '{OFS="\t"; if($1 > 1.15) print $1, $2, $3}' | cut -f3 | ave stdin | grep "average" >> $results_file

    # processRate results
    # processRate instantaneous_rate 3336.11 flow 10.1.5.2:10.1.9.2:5001 node 6 d0+dt 7.029e-05 m_cWnd 29694 inter_arrival 3597 1000242214 bytes_acked 1500 rtt 77935

    # DestRate flowid 1 1.01438 4066.73 0.000240396 

    cat $base_file | grep "DestRate" | awk '{OFS="\t"; if($4 > 1.15) print $1, $2, $3, $4, $5}' | cut -f5 | ave stdin | grep "min" >> $min_results

    python get_data_graph45.py $base_file $alpha $collated_results_min $collated_results_max 
done

python plot_graph45.py $collated_results_min $collated_results_max $min_plot $max_plot
