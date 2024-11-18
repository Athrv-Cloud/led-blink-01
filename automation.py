import os
import subprocess
import sys


def run_command(command, cwd=None, ignore_errors=False):
    """
    Runs a shell command and handles errors.
    """
    try:
        print(f"Running: {' '.join(command)}")
        subprocess.run(command, cwd=cwd, check=True)
    except subprocess.CalledProcessError as e:
        if ignore_errors:
            print(f"Warning: Command failed with error: {e}")
        else:
            print(f"Error: Command failed with error: {e}")
            sys.exit(1)


def check_coverage_files(build_dir):
    """
    Check if .gcno and .gcda files are generated.
    """
    gcno_files = subprocess.run(["find", build_dir, "-name", "*.gcno"], stdout=subprocess.PIPE)
    gcda_files = subprocess.run(["find", build_dir, "-name", "*.gcda"], stdout=subprocess.PIPE)

    if not gcno_files.stdout:
        print("Error: No .gcno files found. Ensure coverage flags are enabled during compilation.")
        sys.exit(1)

    if not gcda_files.stdout:
        print("Error: No .gcda files found. Ensure tests have been run to generate coverage data.")
        sys.exit(1)


def main():
    # Directories
    project_dir = os.path.abspath(os.path.dirname(__file__))
    build_dir = os.path.join(project_dir, "build")
    coverage_report_dir = os.path.join(build_dir, "coverage_report")

    # Ensure a clean build
    print("Cleaning build directory...")
    if os.path.exists(build_dir):
        subprocess.run(["rm", "-rf", build_dir])
    os.makedirs(build_dir, exist_ok=True)

    # Step 1: Configure and build the project
    print("\nConfiguring the project...")
    run_command(["cmake", "-DCOVERAGE=ON", ".."], cwd=build_dir)

    print("\nBuilding the project...")
    run_command(["make"], cwd=build_dir)

    # Step 2: Run tests
    print("\nRunning tests...")
    test_executable = os.path.join(build_dir, "tests", "ledblink_tests")
    if not os.path.exists(test_executable):
        print(f"Error: Test executable not found at {test_executable}")
        sys.exit(1)
    run_command([test_executable], cwd=os.path.join(build_dir, "tests"))

    # Step 3: Verify coverage files
    print("\nChecking coverage files...")
    check_coverage_files(build_dir)

    # Step 4: Generate coverage report
    print("\nGenerating coverage report...")
    coverage_info = os.path.join(build_dir, "coverage.info")
    coverage_clean_info = os.path.join(build_dir, "coverage_clean.info")

    # Capture coverage data with error handling
    run_command([
        "lcov", "--capture", "--directory", ".", "--output-file", coverage_info,
        "--ignore-errors", "inconsistent"
    ], cwd=build_dir)

    # Remove unnecessary coverage data (e.g., system files or test framework files)
    run_command([
        "lcov", "--remove", coverage_info, "/usr/*", "*/CppUTest/*", "--output-file", coverage_clean_info
    ], cwd=build_dir)

    # Generate HTML report
    run_command(["genhtml", coverage_clean_info, "--output-directory", coverage_report_dir], cwd=build_dir)

    print("\nCoverage report generated successfully.")
    print(f"Open the report using: xdg-open {os.path.join(coverage_report_dir, 'index.html')}")


if __name__ == "__main__":
    main()

