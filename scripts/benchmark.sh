#!/bin/bash

set -u

# ======================================
# Report
# ======================================

RESULT_DIR="results"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
REPORT_FILE="${RESULT_DIR}/benchmark_${TIMESTAMP}.txt"

mkdir -p "${RESULT_DIR}"

# Terminal output + TXT report
exec > >(tee -a "${REPORT_FILE}") 2>&1

echo "Report file: ${REPORT_FILE}"
echo "Start time : $(date)"
echo

# ======================================
# Configuration
# ======================================

HOST="127.0.0.1"
PORT=8080
URL="http://${HOST}:${PORT}"

THREADS=$(nproc)

WARMUP_DURATION=60
DURATION=30
STABILITY_DURATION=300

# ======================================
# Connection staircase
# ======================================

CONNECTIONS=(
    100
    200
    400
    600
    800
    1000
    1200
    1400
    1600
    1800
    2000
    2200
    2400
    2600
    2800
    3000
    4000
    5000
    7000
    10000
    15000
    20000
    30000
    40000
    50000
    60000
    70000
    80000
    90000
    100000
    110000
    120000
    130000
    140000
    150000
)

BEST_CONNECTIONS=0
BEST_RPS=0

# ======================================
# Header
# ======================================

echo "======================================"
echo "   Evolving C++ Network Library"
echo "   HTTP Benchmark"
echo "======================================"

echo
echo "Server : ${URL}"
echo "Threads: ${THREADS}"
echo "Warmup : ${WARMUP_DURATION}s"
echo "Test   : ${DURATION}s"
echo "Stable : ${STABILITY_DURATION}s"
echo

# ======================================
# Check dependencies
# ======================================

echo "[1] Checking dependencies..."

if ! command -v wrk >/dev/null 2>&1; then
    echo "ERROR: wrk is not installed"
    exit 1
fi

if ! command -v curl >/dev/null 2>&1; then
    echo "ERROR: curl is not installed"
    exit 1
fi

echo "Dependencies OK."

# ======================================
# Check server
# ======================================

echo
echo "[2] Checking server..."

if ! curl -s --max-time 5 "${URL}" >/dev/null; then
    echo "ERROR: server is not running"
    exit 1
fi

echo "Server is available."

# ======================================
# Warmup
# ======================================

echo
echo "======================================"
echo "   Warmup"
echo "======================================"

echo
echo "Duration    : ${WARMUP_DURATION}s"
echo "Connections : 100"

wrk \
    --latency \
    -t"${THREADS}" \
    -c100 \
    -d"${WARMUP_DURATION}s" \
    "${URL}"

# ======================================
# Baseline Benchmark
# ======================================

echo
echo "======================================"
echo "   Baseline Benchmark"
echo "======================================"

echo
echo "Duration    : ${DURATION}s"
echo "Connections : 100"

wrk \
    --latency \
    -t"${THREADS}" \
    -c100 \
    -d"${DURATION}s" \
    "${URL}"

# ======================================
# Connection Staircase
# ======================================

echo
echo "======================================"
echo "   Connection Staircase"
echo "======================================"

for CONN in "${CONNECTIONS[@]}"
do

    echo
    echo "--------------------------------------"
    echo "Connections: ${CONN}"
    echo "--------------------------------------"

    RESULT_FILE=$(mktemp)

    if wrk \
        --latency \
        -t"${THREADS}" \
        -c"${CONN}" \
        -d"${DURATION}s" \
        "${URL}" >"${RESULT_FILE}" 2>&1
    then

        # ----------------------------------
        # Print raw wrk result
        # ----------------------------------

        cat "${RESULT_FILE}"

        # ----------------------------------
        # Requests/sec
        # ----------------------------------

        RPS=$(awk '
            /Requests\/sec:/ {
                print $2
                exit
            }
        ' "${RESULT_FILE}")

        # ----------------------------------
        # Average latency
        # ----------------------------------

        AVG_LATENCY=$(awk '
            $1 == "Latency" && $2 != "Distribution" {
                print $2
                exit
            }
        ' "${RESULT_FILE}")

        # ----------------------------------
        # P99 latency
        # ----------------------------------

        P99=$(awk '
            $1 == "99%" {
                print $2
                exit
            }
        ' "${RESULT_FILE}")

        # ----------------------------------
        # Socket errors
        # ----------------------------------

        SOCKET_ERRORS=$(awk '
            /Socket errors:/ {
                print
                exit
            }
        ' "${RESULT_FILE}")

        # ----------------------------------
        # Summary
        # ----------------------------------

        echo
        echo "Summary:"
        echo "  Connections : ${CONN}"
        echo "  Requests/sec: ${RPS:-N/A}"
        echo "  Avg Latency : ${AVG_LATENCY:-N/A}"
        echo "  P99 Latency : ${P99:-N/A}"

        # ----------------------------------
        # Socket error handling
        # ----------------------------------

        if [[ -n "${SOCKET_ERRORS}" ]]; then

            echo "  ${SOCKET_ERRORS}"

            CONNECT_ERRORS=$(echo "${SOCKET_ERRORS}" | awk '
    {
        for (i = 1; i <= NF; ++i) {
            if ($i == "connect") {
                value = $(i + 1)
                gsub(/[^0-9]/, "", value)
                print value
                exit
            }
        }
    }
')

            if [[ -n "${CONNECT_ERRORS}" &&
                  "${CONNECT_ERRORS}" -gt 0 ]]; then

                echo
                echo "======================================"
                echo "   Connection Limit Reached"
                echo "======================================"

                echo "Connect errors: ${CONNECT_ERRORS}"
                echo "Stopping connection staircase."

                rm -f "${RESULT_FILE}"

                break
            fi
        fi

        # ----------------------------------
        # Record best throughput
        # ----------------------------------

        if [[ -n "${RPS}" ]]; then

            RPS_VALUE=$(printf "%.0f" "${RPS}")

            if (( RPS_VALUE > BEST_RPS )); then

                BEST_RPS=${RPS_VALUE}
                BEST_CONNECTIONS=${CONN}

            fi
        fi

        # ----------------------------------
        # Remove temporary result
        # ----------------------------------

        rm -f "${RESULT_FILE}"

    else

        echo
        echo "======================================"
        echo "   Connection Limit Reached"
        echo "======================================"

        cat "${RESULT_FILE}"

        echo
        echo "Failed at ${CONN} connections."
        echo "Stopping connection staircase."

        rm -f "${RESULT_FILE}"

        break

    fi

done

# ======================================
# Benchmark Summary
# ======================================

echo
echo "======================================"
echo "   Benchmark Summary"
echo "======================================"

if (( BEST_CONNECTIONS > 0 )); then

    echo
    echo "Best throughput:"
    echo "  Connections : ${BEST_CONNECTIONS}"
    echo "  Requests/sec: ${BEST_RPS}"

else

    echo
    echo "No valid benchmark result."

    echo
    echo "End time: $(date)"

    exit 1

fi

# ======================================
# Stability Test
# ======================================

echo
echo "======================================"
echo "   Stability Test"
echo "======================================"

echo
echo "Connections : ${BEST_CONNECTIONS}"
echo "Duration    : ${STABILITY_DURATION}s"

STABILITY_RESULT=$(mktemp)

if wrk \
    --latency \
    -t"${THREADS}" \
    -c"${BEST_CONNECTIONS}" \
    -d"${STABILITY_DURATION}s" \
    "${URL}" >"${STABILITY_RESULT}" 2>&1
then

    cat "${STABILITY_RESULT}"

    STABILITY_RPS=$(awk '
        /Requests\/sec:/ {
            print $2
            exit
        }
    ' "${STABILITY_RESULT}")

    STABILITY_AVG=$(awk '
        $1 == "Latency" && $2 != "Distribution" {
            print $2
            exit
        }
    ' "${STABILITY_RESULT}")

    STABILITY_P99=$(awk '
        $1 == "99%" {
            print $2
            exit
        }
    ' "${STABILITY_RESULT}")

    STABILITY_ERRORS=$(awk '
        /Socket errors:/ {
            print
            exit
        }
    ' "${STABILITY_RESULT}")

    echo
    echo "Stability Summary:"
    echo "  Connections : ${BEST_CONNECTIONS}"
    echo "  Requests/sec: ${STABILITY_RPS:-N/A}"
    echo "  Avg Latency : ${STABILITY_AVG:-N/A}"
    echo "  P99 Latency : ${STABILITY_P99:-N/A}"

    if [[ -n "${STABILITY_ERRORS}" ]]; then
        echo "  ${STABILITY_ERRORS}"
    fi

else

    echo
    echo "Stability test failed."

    cat "${STABILITY_RESULT}"

fi

rm -f "${STABILITY_RESULT}"

# ======================================
# Finished
# ======================================

echo
echo "======================================"
echo "   Benchmark finished."
echo "======================================"

echo
echo "End time   : $(date)"
echo "Report file: ${REPORT_FILE}"
echo