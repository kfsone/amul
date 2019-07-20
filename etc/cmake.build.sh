#! /bin/bash
#
# Build a custom cmake for the AMUL build environment, because the current
# ubuntu image uses cmake 3.10 vs the 3.15 I have here.

function die () {
  echo "ERROR: $@"
  exit 22
}

if [ "${PWD}" != "/tmp" ]; then cd /tmp; fi
if [ "${PWD}" != "/tmp" ]; then
  die "Must run from /tmp"
fi

# Make sure there's just one file
cmake_src_count=$(ls -1 ./cmake*.t*z | wc -l)
if [ $cmake_src_count -lt 1 ]; then die "No 'cmake' source tarball in ${PWD}"; fi
if [ $cmake_src_count -gt 1 ]; then die "More than one 'cmake' source tarball in ${PWD}"; fi
cmake_src_tar=$(ls -1 cmake*.t*z)

cmake_src_dir="/tmp/cmake-src"
if [ -d "${cmake_src_dir}" ]; then
  echo "Removing previous ${cmake_src_dir}"
  rm -rf "${cmake_src_dir}" || die "Cannot remove extant ${cmake_src_dir}"
fi
mkdir -p "${cmake_src_dir}" || die "Cannot create ${cmake_src_dir}: $?"

echo "Extracting ${cmake_src_tar} -> ${cmake_src_dir}"
tar xf "${cmake_src_tar}" -C "${cmake_src_dir}" || die "Extract failed: $?"

if [ $(ls -1 "${cmake_src_dir}" | wc -l) -ne 1 ]; then
  die "Expected single cmake source directory, got: $(ls -l "${cmake_src_dir}")"
fi

build_root="$(ls -1d ${cmake_src_dir}/cmake-*)"
cd "${build_root}" || die "Couldn't cd to build root: ${build_root}: $?"

./bootstrap --parallel=9 && make -j9 && make install

rm -rf "${cmake_src_dir}"

