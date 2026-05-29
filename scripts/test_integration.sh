#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT_DIR/build"
EXECUTABLE="$BUILD_DIR/smart_door_ecu"

ARTIFACT_DIR="$ROOT_DIR/artifacts/integration"
ECU_LOG="$ARTIFACT_DIR/ecu.log"
ECU_RESTART_LOG="$ARTIFACT_DIR/ecu_restart.log"
CANDUMP_LOG="$ARTIFACT_DIR/candump.log"

STATE_FILE="$ROOT_DIR/runtime/smartdoor_state.txt"
EVENT_FILE="$ROOT_DIR/runtime/smartdoor_events.log"

ECU_PID=""
CANDUMP_PID=""

mkdir -p "$ARTIFACT_DIR"
mkdir -p "$ROOT_DIR/runtime"

cleanup() {
    if [[ -n "${ECU_PID}" ]] && kill -0 "${ECU_PID}" 2>/dev/null; then
        kill "${ECU_PID}" 2>/dev/null || true
        wait "${ECU_PID}" 2>/dev/null || true
    fi

    if [[ -n "${CANDUMP_PID}" ]] && kill -0 "${CANDUMP_PID}" 2>/dev/null; then
        kill "${CANDUMP_PID}" 2>/dev/null || true
        wait "${CANDUMP_PID}" 2>/dev/null || true
    fi
}
trap cleanup EXIT

print_banner() {
    echo
    echo "============================================================"
    echo "$1"
    echo "============================================================"
}

fail() {
    echo
    echo "[FAIL] $1"
    echo
    echo "---- ECU LOG ----"
    [[ -f "$ECU_LOG" ]] && cat "$ECU_LOG"
    echo
    echo "---- ECU RESTART LOG ----"
    [[ -f "$ECU_RESTART_LOG" ]] && cat "$ECU_RESTART_LOG"
    echo
    echo "---- CANDUMP LOG ----"
    [[ -f "$CANDUMP_LOG" ]] && cat "$CANDUMP_LOG"
    echo
    echo "---- EVENT LOG FILE ----"
    [[ -f "$EVENT_FILE" ]] && cat "$EVENT_FILE"
    exit 1
}

require_command() {
    command -v "$1" >/dev/null 2>&1 || fail "Required command not found: $1"
}

get_match_count() {
    local file="$1"
    local pattern="$2"

    if [[ ! -f "$file" ]]; then
        echo 0
        return
    fi

    grep -Ec "$pattern" "$file" || true
}

wait_for_pattern() {
    local file="$1"
    local pattern="$2"
    local timeout_sec="$3"

    local elapsed=0
    while (( elapsed < timeout_sec )); do
        if grep -Eq "$pattern" "$file"; then
            return 0
        fi
        sleep 1
        ((elapsed+=1))
    done

    return 1
}

wait_for_count_increase() {
    local file="$1"
    local pattern="$2"
    local previous_count="$3"
    local timeout_sec="$4"

    local elapsed=0
    while (( elapsed < timeout_sec )); do
        local current_count
        current_count=$(get_match_count "$file" "$pattern")
        if (( current_count > previous_count )); then
            return 0
        fi
        sleep 1
        ((elapsed+=1))
    done

    return 1
}

assert_pattern() {
    local file="$1"
    local pattern="$2"
    local description="$3"

    if wait_for_pattern "$file" "$pattern" 8; then
        echo "[PASS] $description"
    else
        fail "$description (pattern not found: $pattern)"
    fi
}

assert_new_match_after_action() {
    local file="$1"
    local pattern="$2"
    local description="$3"
    local timeout_sec="$4"

    local before_count
    before_count=$(get_match_count "$file" "$pattern")

    if wait_for_count_increase "$file" "$pattern" "$before_count" "$timeout_sec"; then
        echo "[PASS] $description"
    else
        fail "$description (new match not found: $pattern)"
    fi
}

assert_no_new_match_after_command() {
    local file="$1"
    local pattern="$2"
    local command_to_run="$3"
    local description="$4"

    local before_count
    before_count=$(get_match_count "$file" "$pattern")

    eval "$command_to_run"
    sleep 1

    local after_count
    after_count=$(get_match_count "$file" "$pattern")

    if [[ "$before_count" == "$after_count" ]]; then
        echo "[PASS] $description"
    else
        fail "$description (unexpected new match found for pattern: $pattern)"
    fi
}

send_command() {
    local byte0="$1"
    echo "[INFO] Sending command byte: $byte0"
    cansend vcan0 "101#${byte0}00000000000000"
    sleep 1
}

assert_status_after_command() {
    local command_byte="$1"
    local pattern="$2"
    local description="$3"

    local before_count
    before_count=$(get_match_count "$CANDUMP_LOG" "$pattern")

    send_command "$command_byte"

    if wait_for_count_increase "$CANDUMP_LOG" "$pattern" "$before_count" 8; then
        echo "[PASS] $description"
    else
        fail "$description (expected status not found: $pattern)"
    fi
}

start_ecu() {
    local logfile="$1"
    : > "$logfile"

    "$EXECUTABLE" >"$logfile" 2>&1 &
    ECU_PID=$!
    sleep 1

    if ! kill -0 "${ECU_PID}" 2>/dev/null; then
        fail "ECU process failed to start"
    fi
}

start_candump() {
    : > "$CANDUMP_LOG"

    candump vcan0 >"$CANDUMP_LOG" 2>&1 &
    CANDUMP_PID=$!
    sleep 1

    if ! kill -0 "${CANDUMP_PID}" 2>/dev/null; then
        fail "candump failed to start"
    fi
}

stop_ecu() {
    if [[ -n "${ECU_PID}" ]] && kill -0 "${ECU_PID}" 2>/dev/null; then
        kill "${ECU_PID}" 2>/dev/null || true
        wait "${ECU_PID}" 2>/dev/null || true
    fi
    ECU_PID=""
}

stop_candump() {
    if [[ -n "${CANDUMP_PID}" ]] && kill -0 "${CANDUMP_PID}" 2>/dev/null; then
        kill "${CANDUMP_PID}" 2>/dev/null || true
        wait "${CANDUMP_PID}" 2>/dev/null || true
    fi
    CANDUMP_PID=""
}

print_banner "Checking prerequisites"

require_command cmake
require_command candump
require_command cansend
require_command ip
require_command grep

if [[ ! -f "$ROOT_DIR/CMakeLists.txt" ]]; then
    fail "Project root not detected correctly"
fi

if ! ip link show vcan0 >/dev/null 2>&1; then
    fail "vcan0 not found. Please create it first:
  sudo modprobe vcan
  sudo ip link add dev vcan0 type vcan
  sudo ip link set up vcan0"
fi

if ! ip link show vcan0 | grep -q "UP"; then
    fail "vcan0 exists but is not UP. Run:
  sudo ip link set up vcan0"
fi

print_banner "Cleaning old artifacts"

rm -f "$ECU_LOG" "$ECU_RESTART_LOG" "$CANDUMP_LOG" "$STATE_FILE" "$EVENT_FILE"

print_banner "Building project"

cmake -S "$ROOT_DIR" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR"

if [[ ! -x "$EXECUTABLE" ]]; then
    fail "Executable not found after build: $EXECUTABLE"
fi

print_banner "Starting ECU and candump"

start_ecu "$ECU_LOG"
start_candump

assert_pattern "$ECU_LOG" "Smart Door ECU Simulation Started" "ECU startup banner"
assert_pattern "$ECU_LOG" "ECU running continuously" "ECU entered runtime loop"

print_banner "Phase 1: Functional behavior test"

# 1. LOCK
assert_status_after_command "01" "201.*01 00 00 00" "LOCK response frame"

# 2. DOOR_OPEN
assert_status_after_command "03" "201.*01 01 01 00" "DOOR_OPEN response frame"

# 3. LOCK while door open -> fault
assert_status_after_command "01" "201.*01 01 01 01" "LOCK rejected while door open"

# 4. RESET_FAULT
assert_status_after_command "05" "201.*01 01 01 00" "RESET_FAULT clears fault"

# 5. DOOR_CLOSE -> lamp still ON immediately
assert_status_after_command "04" "201.*01 00 01 00" "DOOR_CLOSE immediate response"

print_banner "Waiting for lamp timeout"

# 6. Lamp timeout should send automatic OFF frame after ~3 sec
assert_new_match_after_action "$CANDUMP_LOG" "201.*01 00 00 00" "Lamp timeout automatic OFF frame" 8

print_banner "Phase 1B: Edge scenario tests"

# Edge 1: Unsupported CAN ID should be ignored
assert_no_new_match_after_command \
    "$CANDUMP_LOG" \
    "201.*" \
    'cansend vcan0 555#0102030405060708' \
    "Unsupported CAN ID is ignored"

# Edge 2: Empty frame on valid command ID should be ignored
assert_no_new_match_after_command \
    "$CANDUMP_LOG" \
    "201.*" \
    'cansend vcan0 101#' \
    "Empty command frame is ignored"

# Edge 3: Unknown command byte should be ignored
assert_no_new_match_after_command \
    "$CANDUMP_LOG" \
    "201.*" \
    'cansend vcan0 101#9900000000000000' \
    "Unknown command byte is ignored"

# Edge 4: Repeated LOCK should be ignored
assert_status_after_command "02" "201.*00 00 00 00" "Precondition: unlocked state"
assert_status_after_command "01" "201.*01 00 00 00" "Precondition: locked state"
assert_status_after_command "01" "201.*01 00 00 00" "Repeated LOCK keeps same status"

# Edge 5: Repeated UNLOCK should be ignored
assert_status_after_command "02" "201.*00 00 00 00" "UNLOCK response"
assert_status_after_command "02" "201.*00 00 00 00" "Repeated UNLOCK keeps same status"

# Edge 6: Repeated DOOR_OPEN should be ignored
assert_status_after_command "03" "201.*00 01 01 00" "DOOR_OPEN response"
assert_status_after_command "03" "201.*00 01 01 00" "Repeated DOOR_OPEN keeps same status"

# Edge 7: Repeated DOOR_CLOSE should be ignore-safe
assert_status_after_command "04" "201.*00 00 01 00" "DOOR_CLOSE immediate response"
assert_new_match_after_action "$CANDUMP_LOG" "201.*00 00 00 00" "Lamp timeout after DOOR_CLOSE" 8
assert_status_after_command "04" "201.*00 00 00 00" "Repeated DOOR_CLOSE keeps same status"

# Edge 8: RESET_FAULT when no fault is active
assert_status_after_command "05" "201.*00 00 00 00" "RESET_FAULT with no active fault keeps state unchanged"

print_banner "Phase 1C: Fault priority test"

# Door-open rule must take priority over repeated LOCK logic
assert_status_after_command "02" "201.*00 00 00 00" "Reset to unlocked baseline"
assert_status_after_command "01" "201.*01 00 00 00" "LOCK baseline"
assert_status_after_command "03" "201.*01 01 01 00" "DOOR_OPEN while locked"
assert_status_after_command "01" "201.*01 01 01 01" "Door-open rule takes priority over repeated LOCK"

print_banner "Checking ECU debug log messages"

assert_pattern "$ECU_LOG" "LOCK ignored: already locked" "ECU log shows repeated LOCK ignored"
assert_pattern "$ECU_LOG" "UNLOCK ignored: already unlocked" "ECU log shows repeated UNLOCK ignored"
assert_pattern "$ECU_LOG" "LOCK rejected: door is open" "ECU log shows lock rejection when door open"
assert_pattern "$ECU_LOG" "RESET_FAULT applied: fault cleared" "ECU log shows RESET_FAULT applied"
assert_pattern "$ECU_LOG" "DOOR_OPEN ignored: already open" "ECU log shows repeated DOOR_OPEN ignored"
assert_pattern "$ECU_LOG" "DOOR_CLOSE ignored: already closed" "ECU log shows repeated DOOR_CLOSE ignored"

print_banner "Phase 2: Persistence restart test"

# Bring ECU into a known persisted LOCKED state:
# open -> close -> wait lamp timeout -> lock
assert_status_after_command "03" "201.*01 01 01 00" "Set state: DOOR_OPEN while locked"
assert_status_after_command "04" "201.*01 00 01 00" "Set state: DOOR_CLOSE immediate response"
assert_new_match_after_action "$CANDUMP_LOG" "201.*01 00 00 00" "Set state: lamp timeout completed" 8
assert_status_after_command "01" "201.*01 00 00 00" "Persist LOCKED state before restart"

stop_ecu

start_ecu "$ECU_RESTART_LOG"

assert_pattern "$ECU_RESTART_LOG" \
    "Restored persisted lock state: LOCKED" \
    "Persistence restart restored LOCKED state"

print_banner "Phase 2B: Corrupted persistence file fallback test"

stop_ecu

echo "BROKEN_STATE" > "$STATE_FILE"

start_ecu "$ECU_RESTART_LOG"

assert_pattern "$ECU_RESTART_LOG" \
    "No persisted state found. Starting with default UNLOCKED state." \
    "Corrupted persistence file falls back to default UNLOCKED state"

stop_ecu

rm -f "$STATE_FILE"

start_ecu "$ECU_RESTART_LOG"

assert_pattern "$ECU_RESTART_LOG" \
    "No persisted state found. Starting with default UNLOCKED state." \
    "Missing persistence file also falls back to default UNLOCKED state"

print_banner "Phase 3: Persistent event log verification"

assert_pattern "$EVENT_FILE" "ECU SESSION START" "Event file contains session start marker"
assert_pattern "$EVENT_FILE" "ECU initialized successfully" "Event file contains initialization entry"
assert_pattern "$EVENT_FILE" "Command received: LOCK" "Event file contains command-history entry"
assert_pattern "$EVENT_FILE" "Status sent \\(LOCK\\)" "Event file contains status transmission entry"

print_banner "All integration tests passed"

echo "[PASS] Full integration test suite completed successfully."
echo "[INFO] Artifact directory : $ARTIFACT_DIR"
echo "[INFO] State file         : $STATE_FILE"
echo "[INFO] Event log file     : $EVENT_FILE"