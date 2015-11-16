for i in `seq 0 64`; do
	sudo ipcrm -M $i
done
sudo ipcrm -M 3185		#Writer's hash table
sudo ipcrm -M 3186		#Writer's hash table
sudo ipcrm -M 3006		#Writer's hash table
