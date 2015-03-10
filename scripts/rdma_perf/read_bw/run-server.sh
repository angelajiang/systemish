# Run servers on port #1
for i in `seq 0 19`; do
	port=`expr 20000 + $i`
	ib_read_bw --ib-port=1 --size=32 --run_infinitely --duration=1 --port=$port &
done

# Run servers on port #1
for i in `seq 0 19`; do
	port=`expr 30000 + $i`
	ib_read_bw --ib-port=2 --size=32 --run_infinitely --duration=1 --port=$port &
done
