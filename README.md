To generate the compile_commands.json have to give the command cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .. within build folder
After that if you are running make it will work but test folders has not been enabled to enable give cmake -DENABLE_TESTS=ON ..


 lcov --capture --directory . \
     --output-file /home/purusottam/gudu/REPO/led-blink-02-test-02/build/coverage.info \
     --include '/home/purusottam/gudu/REPO/led-blink-02-test-02/tests/*' \
     --ignore-errors inconsistent --ignore-errors mismatch


genhtml /home/purusottam/gudu/REPO/led-blink-02-test-02/build/coverage.info \
        --output-directory /home/purusottam/gudu/REPO/led-blink-02-test-02/build/coverage_report
