COUNTER=0
while [  $COUNTER -lt 1000000 ]; do
    echo $COUNTER
    let COUNTER=COUNTER+1
    sleep 1
done

