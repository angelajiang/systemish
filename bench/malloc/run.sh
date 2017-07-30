echo "malloc"
time taskset -c 0 ./main

echo ""
echo "tcmalloc"
time taskset -c 0 ./main_tc

echo ""
echo "jemalloc"
time taskset -c 0 ./main_je
