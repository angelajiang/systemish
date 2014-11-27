# A function to echo in blue color
function blue() {
	es=`tput setaf 4`
	ee=`tput sgr0`
	echo "${es}$1${ee}"
}

blue "Removing shm used for pointer chasing"
sudo ipcrm -M 3185

blue "Running pointer-chasing with ISPC"
sudo taskset -c 0 ./main 1

