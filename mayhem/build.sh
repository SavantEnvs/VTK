#!/usr/bin/env bash
# mayhem/build.sh — build VTK's IO modules (ASan+UBSan, static, no rendering backend) and the
# fuzz harnesses + known-answer oracle in mayhem/fuzz/. Idempotent and offline: every dependency
# VTK needs is vendored under ThirdParty/, so a re-run only rebuilds what changed.
set -euo pipefail
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH

: "${SANITIZER_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer}"
: "${DEBUG_FLAGS:=-g -gdwarf-3}"
: "${CC:=clang}" ; : "${CXX:=clang++}" ; : "${LIB_FUZZING_ENGINE:=-fsanitize=fuzzer}"
: "${STANDALONE_FUZZ_MAIN:=/opt/mayhem/StandaloneFuzzTargetMain.c}"
# Default parallelism: nproc, capped at one job per 2 GiB of the container's memory limit. VTK's
# ASan+UBSan+SanCov array-instantiation TUs each need well over 1 GiB, and -j$(nproc) under a 12 GiB
# cgroup limit gets compilers OOM-killed ("ninja: build stopped: subcommand failed").
if [ -z "${MAYHEM_JOBS:-}" ]; then
  MAYHEM_JOBS="$(nproc)"
  mem_max="$(cat /sys/fs/cgroup/memory.max 2>/dev/null || cat /sys/fs/cgroup/memory/memory.limit_in_bytes 2>/dev/null || echo max)"
  mem_total=$(( $(awk '/^MemTotal:/ {print $2}' /proc/meminfo) * 1024 ))
  case "$mem_max" in ''|max|*[!0-9]*) mem_max="$mem_total" ;; esac
  [ "$mem_max" -le "$mem_total" ] || mem_max="$mem_total"
  mem_jobs=$(( mem_max / (2 * 1024 * 1024 * 1024) ))
  [ "$mem_jobs" -ge 1 ] || mem_jobs=1
  [ "$MAYHEM_JOBS" -le "$mem_jobs" ] || MAYHEM_JOBS="$mem_jobs"
fi
: "${SRC:=/mayhem}"
export CC CXX

cd "$SRC"
VTK_BUILD="$SRC/mayhem/build-vtk"     # gitignored by mayhem/.gitignore
FUZZ_BUILD="$SRC/mayhem/build-fuzz"   # gitignored by mayhem/.gitignore
OUT="$SRC/mayhem/out"                 # gitignored by mayhem/.gitignore

# 1) VTK itself, instrumented. Only the reader modules the harnesses use (and their deps).
# SanCov (-fsanitize=fuzzer-no-link) goes on EVERY compile, VTK and harness TUs alike, and stays on
# even when SANITIZER_FLAGS is empty: $LIB_FUZZING_ENGINE is only a link flag here, and without
# SanCov on the compiled code libFuzzer/Mayhem see no edges at all.
COV_FLAGS="-fsanitize=fuzzer-no-link"
FLAGS="$SANITIZER_FLAGS $COV_FLAGS $DEBUG_FLAGS"
mods=(IOLegacy IOXML IOPLY IOGeometry IOImport)
mod_args=()
for m in "${mods[@]}"; do mod_args+=("-DVTK_MODULE_ENABLE_VTK_${m}=YES"); done
grp_args=()
for g in StandAlone Rendering Imaging Views Qt Web MPI; do grp_args+=("-DVTK_GROUP_ENABLE_${g}=DONT_WANT"); done

cmake -S "$SRC" -B "$VTK_BUILD" -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_FLAGS="$FLAGS -O1" \
  -DCMAKE_CXX_FLAGS="$FLAGS -O1" \
  -DCMAKE_C_FLAGS_DEBUG="" -DCMAKE_CXX_FLAGS_DEBUG="" \
  -DCMAKE_EXE_LINKER_FLAGS="$SANITIZER_FLAGS $COV_FLAGS" \
  -DBUILD_SHARED_LIBS=OFF \
  -DVTK_BUILD_TESTING=OFF -DVTK_BUILD_EXAMPLES=OFF -DVTK_BUILD_DOCUMENTATION=OFF \
  -DVTK_ENABLE_WRAPPING=OFF -DVTK_WRAP_PYTHON=OFF -DVTK_WRAP_JAVA=OFF \
  -DVTK_SMP_IMPLEMENTATION_TYPE=Sequential \
  -DVTK_ENABLE_REMOTE_MODULES=OFF \
  -DVTK_USE_X=OFF -DVTK_OPENGL_HAS_EGL=OFF \
  "${grp_args[@]}" "${mod_args[@]}"
cmake --build "$VTK_BUILD" -j"$MAYHEM_JOBS"

# 2) Harnesses: each linked twice (libFuzzer binary + run-once standalone reproducer), plus the
#    known-answer oracle driver used by mayhem/test.sh.
VTK_DIR="$(dirname "$(ls -d "$VTK_BUILD"/lib/cmake/vtk-*/vtk-config.cmake | head -1)")"
cmake -S "$SRC/mayhem/fuzz" -B "$FUZZ_BUILD" -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_FLAGS="$FLAGS -O1" \
  -DCMAKE_CXX_FLAGS="$FLAGS -O1" \
  -DCMAKE_C_FLAGS_DEBUG="" -DCMAKE_CXX_FLAGS_DEBUG="" \
  -DCMAKE_EXE_LINKER_FLAGS="$SANITIZER_FLAGS $COV_FLAGS" \
  -DVTK_DIR="$VTK_DIR" \
  -DLIB_FUZZING_ENGINE="$LIB_FUZZING_ENGINE" \
  -DSTANDALONE_FUZZ_MAIN="$STANDALONE_FUZZ_MAIN" \
  -DLSAN_OFF="$SRC/mayhem/lsan_off.cc" \
  -DOUT_DIR="$OUT"
cmake --build "$FUZZ_BUILD" -j"$MAYHEM_JOBS"
ls -l "$OUT"
