#!/usr/bin/env bash

pip install --user coverage

tee coverage <<EOF
#!/usr/bin/env bash
`which coverage` run \$@
EOF
chmod +x coverage

cmake .. -DBUILD_GO=OFF -DCMAKE_BUILD_TYPE=Coverage
make

# list all tests
ctest -N

function run_and_report() {
    local group=$1
    local tests=$2
    ctest -R "${tests}"
    bash ../bin/record-coverage.sh . ..
    bash <(curl -s https://codecov.io/bash) -F ${group}

    # c/cpp
    find \( -name "*.gcov" -o -name "*.gcda" \) -exec rm {} \;
    # python
    find -name "*.coverage" -exec rm {} \;
}

run_and_report "c" "c-object-tests|c-message-tests|c-engine-tests|c-parse-url-tests|c-refcount-tests|c-event-tests|c-data-tests|c-condition-tests|c-connection-driver-tests|c-proactor-tests|c-fdlimit-tests"
run_and_report "cpp" "cpp-codec_test|cpp-connection_driver_test|cpp-interop_test|cpp-message_test|cpp-map_test|cpp-scalar_test|cpp-value_test|cpp-container_test|cpp-url_test|cpp-reconnect_test"
run_and_report "python" "python-test"
run_and_report "c-examples" "c-example-tests"
run_and_report "cpp-examples" "cpp-example-container|cpp-example-container-ssl"
