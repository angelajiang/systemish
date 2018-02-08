num_virus=10000
rm virus.txt
touch virus.txt

for i in `seq 1 $num_virus`; do
  virus=`pwgen 2 1`
  echo $virus >> virus.txt
done
