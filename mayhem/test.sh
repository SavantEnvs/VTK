#!/usr/bin/env bash
# mayhem/test.sh — known-answer tests for the VTK readers the harnesses fuzz. Runs the oracle driver
# built by mayhem/build.sh (mayhem/out/vtk_oracle) on the committed sample files in mayhem/samples/
# and compares what each reader parsed (blocks, point/cell counts, bounds, arrays) with golden
# values. Emits a CTRF report; exits non-zero iff any case fails.
set -uo pipefail
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH
: "${SRC:=/mayhem}"
cd "$SRC"

emit_ctrf() {
  local tool="$1" passed="$2" failed="$3" skipped="${4:-0}" pending="${5:-0}" other="${6:-0}"
  local tests=$(( passed + failed + skipped + pending + other ))
  cat > "${CTRF_REPORT:-$SRC/ctrf-report.json}" <<JSON
{
  "results": {
    "tool": { "name": "$tool" },
    "summary": {
      "tests": $tests,
      "passed": $passed,
      "failed": $failed,
      "pending": $pending,
      "skipped": $skipped,
      "other": $other
    }
  }
}
JSON
  printf 'CTRF {"results":{"tool":{"name":"%s"},"summary":{"tests":%d,"passed":%d,"failed":%d,"pending":%d,"skipped":%d,"other":%d}}}\n' \
    "$tool" "$tests" "$passed" "$failed" "$pending" "$skipped" "$other"
  [ "$failed" -eq 0 ]
}

ORACLE="$SRC/mayhem/out/vtk_oracle"
S="$SRC/mayhem/samples"
if [ ! -x "$ORACLE" ]; then
  echo "FATAL: $ORACLE missing — run mayhem/build.sh first" >&2
  emit_ctrf vtk-known-answer 0 1
  exit 1
fi

passed=0 failed=0
check() { # check <kind> <file> <expected output line>
  local got
  got="$("$ORACLE" "$1" "$S/$2" 2>/dev/null)"
  if [ "$got" == "$3" ]; then
    passed=$((passed + 1)); echo "PASS $1 $2"
  else
    failed=$((failed + 1)); echo "FAIL $1 $2: expected '$3', got '$got'"
  fi
}

check legacy tri.vtk  "blocks=1 points=4 cells=2 bounds=0,1,0,1,0,2 point_arrays=1"
check vtu    tet.vtu  "blocks=1 points=5 cells=2 bounds=0,3,0,3,0,3 point_arrays=1"
check ply    quad.ply "blocks=1 points=4 cells=1 bounds=0,2,0,3,0,1 point_arrays=0"
check stl    tri.stl  "blocks=1 points=4 cells=2 bounds=0,1,0,1,0,0 point_arrays=0"
check obj    cube.obj "blocks=1 points=6 cells=2 bounds=0,1,0,1,0,5 point_arrays=2"
check gltf   tri.gltf "blocks=1 points=3 cells=1 bounds=0,1,0,1,1,1"
check gltf   tri.glb  "blocks=1 points=3 cells=1 bounds=0,1,0,1,1,1"

emit_ctrf vtk-known-answer "$passed" "$failed"
