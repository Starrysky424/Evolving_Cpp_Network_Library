#!/bin/bash

# TCP Network Library Benchmark Script
# 适配 Evolving_Cpp_Network_Library

set -e


# ==================== 颜色 ====================

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'


# ==================== 路径 ====================

ROOT_DIR=$(cd "$(dirname "$0")/.." && pwd)

SERVER="$ROOT_DIR/build/main"

CLIENT="$ROOT_DIR/build/benchmark/tcp_benchmark"


PORT=8080


REPORT_FILE="benchmark_report_$(date +%Y%m%d_%H%M%S).txt"



echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}TCP Network Benchmark${NC}"
echo -e "${BLUE}========================================${NC}"

echo "Server : $SERVER"
echo "Client : $CLIENT"
echo "Port   : $PORT"
echo "Report : $REPORT_FILE"


# ==================== 启动服务器 ====================


echo -e "\n${YELLOW}[1/5] Starting TCPserver${NC}"


SERVER_LOG="$ROOT_DIR/server.log"

$SERVER > $SERVER_LOG 2>&1 &


SERVER_PID=$!


echo "TCPserver PID: $SERVER_PID"


# ==================== 等待端口 ====================


echo -e "\n${YELLOW}Waiting TCP server...${NC}"


for i in {1..10}
do

    if ss -lnt | grep -q ":$PORT"
    then
        echo -e "${GREEN}✓ TCPserver ready${NC}"
        break
    fi


    sleep 1


done



if ! ss -lnt | grep -q ":$PORT"
then

    echo -e "${RED}TCPserver start failed${NC}"

    cat server.log

    exit 1

fi



# ==================== 初始化报告 ====================


cat > $REPORT_FILE << EOF

===================================================
        TCP Benchmark Report
===================================================

Time:
$(date)

Server:
$SERVER

Port:
$PORT


Connection    QPS        P50        P99        Errors

EOF



# ==================== 预热 ====================


echo -e "\n${YELLOW}[2/5] Warm up${NC}"


$CLIENT 100 1000 64 > /dev/null


echo -e "${GREEN}✓ Warm up finished${NC}"



# ==================== 压力测试 ====================


echo -e "\n${YELLOW}[3/5] Load test${NC}"



CONNECTIONS=(

100
200
400
600
800
1000
2000
5000
10000


)



RESULTS=()



for CONN in "${CONNECTIONS[@]}"
do


echo -e "\n${BLUE}---- Testing $CONN connections ----${NC}"



OUTPUT=$(
$CLIENT \
$CONN \
5000 \
64 2>&1
)



echo "$OUTPUT"



QPS=$(echo "$OUTPUT" \
| grep "QPS" \
| awk '{print $3}')



P50=$(echo "$OUTPUT" \
| grep "P50" \
| awk '{print $3}')



P99=$(echo "$OUTPUT" \
| grep "P99" \
| awk '{print $3}')



ERRORS=$(echo "$OUTPUT" \
| grep "Errors" \
| awk '{print $3}')



ERRORS=${ERRORS:-0}



printf "%-12s %-12s %-12s %-12s %-8s\n" \
"$CONN" \
"$QPS" \
"$P50" \
"$P99" \
"$ERRORS"



printf "%-12s %-12s %-12s %-12s %-8s\n" \
"$CONN" \
"$QPS" \
"${P50}ms" \
"${P99}ms" \
"$ERRORS" \
>> $REPORT_FILE



RESULTS+=("$CONN:$QPS")



sleep 2


done




# ==================== 找最高QPS ====================


echo -e "\n${YELLOW}[4/5] Analyze${NC}"



BEST_CONN=0

BEST_QPS=0



for R in "${RESULTS[@]}"
do

IFS=':' read CONN QPS <<< "$R"


if (( $(echo "$QPS > $BEST_QPS" | bc -l) ))
then

BEST_QPS=$QPS

BEST_CONN=$CONN

fi


done



echo "
Best connection:
$BEST_CONN

Best QPS:
$BEST_QPS
" >> $REPORT_FILE




echo -e "${GREEN}"
echo "Best:"
echo "$BEST_CONN connections"
echo "$BEST_QPS QPS"
echo -e "${NC}"



# ==================== 稳定性测试 ====================


echo -e "\n${YELLOW}[5/5] Stability test${NC}"



STABILITY=$(
$CLIENT \
$BEST_CONN \
300000 \
64 2>&1
)



echo "$STABILITY" >> $REPORT_FILE



STABLE_QPS=$(echo "$STABILITY" \
| grep "QPS" \
| awk '{print $3}')



echo "
Stability QPS:
$STABLE_QPS
" >> $REPORT_FILE



# ==================== 清理 ====================


echo -e "\n${YELLOW}Stopping TCPserver${NC}"


kill $SERVER_PID



echo -e "\n${GREEN}✓ Benchmark finished${NC}"

echo "Report:"
echo "$REPORT_FILE"


tail -20 $REPORT_FILE