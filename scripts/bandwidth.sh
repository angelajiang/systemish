#!/bin/bash
while [[ $# -gt 1 ]]
do
key="$1"

case $key in
    --B)
    bytes="$2"
    shift # past argument
    ;;
    --KB)
    bytes=`expr $2 * 1024`
    shift # past argument
    ;;
    --MB)
    bytes=`expr $2 * 1024 * 1024`
    shift # past argument
    ;;
    --ns)
    ns="$2"
    shift # past argument
    ;;
    --us)
    ns=`expr $2 * 1000`
    shift # past argument
    ;;
    --ms)
    ns=`expr $2 * 1000 * 1000`
    shift # past argument
    ;;
    --Mbps)
    Bps=``
    shift # past argument
    ;;
    --Gbps)
    Gbps="$2"
    shift # past argument
    ;;
    --MBps)
    MBps="$2"
    shift # past argument
    ;;
    --GBps)
    GBps="$2"
    shift # past argument
    ;;
    --default)
    DEFAULT=YES
    ;;
    *)
            # unknown option
    ;;
esac
shift # past argument or value
done

echo B = "${B}"
echo KB = "${KB}"
echo MB = "${MB}"
echo ms = "${ms}"
echo us = "${ms}"
echo Mbps = "${Mbps}"
echo Gbps = "${Gbps}"
echo MBps = "${MBps}"
echo GBps = "${GBps}"

