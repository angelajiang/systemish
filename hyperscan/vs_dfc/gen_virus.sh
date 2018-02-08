num_virus=10000

for i in `seq 1 $num_virus`; do
  virus=`pwgen 2 1`
  echo $virus >> virus.txt
done
