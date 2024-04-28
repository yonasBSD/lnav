#! /bin/bash

export YES_COLOR=1
export TZ=UTC

cat ${test_dir}/logfile_docker_compose.0 | run_cap_test env TEST_COMMENT="docker-demux-no-ts" \
     ${lnav_test} -n

cat ${test_dir}/logfile_docker_compose_with_ts.0 | run_cap_test env TEST_COMMENT="docker-demux-with-ts" \
     ${lnav_test} -n

run_cap_test ${lnav_test} -n ${test_dir}/logfile_docker_compose_with_ts.0
