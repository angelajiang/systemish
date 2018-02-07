# Print the current CPU GHz for all physical cores in this system
for i in `seq 0 32`; do
	freq=`sudo cat /sys/devices/system/cpu/cpu$i/cpufreq/scaling_cur_freq`
	echo "CPU $i: $freq KHz"
done

