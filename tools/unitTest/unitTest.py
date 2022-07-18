from os import listdir, getcwd
from os.path import isfile, join, dirname, basename, splitext, exists
from sys import argv
from multiprocessing import Pool
from termcolor import colored
import re
import subprocess

def runTests(inputData):
    executablePath, testSuite, test = inputData

    if not exists(executablePath):
        return

    try:
        subprocess.run([executablePath, test], stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True)
        print(" ", colored("PASSED", "green"), "{}::{}".format(testSuite, test))
    except subprocess.CalledProcessError as e:
        print(" ", colored("FALILED", "red"), "{}::{} --".format(testSuite, test), e.stderr.decode('utf-8'))

if __name__ == '__main__':
    # Get each file
    baseDirs = argv[1:]
    testDirs = [join(d, 'test') for d in baseDirs]
    filePaths = [join(d, f) for d in testDirs
        for f in listdir(d) if isfile(join(d, f))]

    print("Starting Tests...")

    for filePath in filePaths:
        tests = list()
        testSuite = ""

        # Open the file and get all the tests
        with open(filePath, 'r') as file:
            lines = file.readlines()
            for line in lines:
                testMatch = re.search(r".*TEST\(\"(.*)\"\).*", line)
                testSuiteMatch = re.search(r".*TEST_SUITE\(\"(.*)\"\).*", line)
                if testMatch != None:
                    tests.append(testMatch.group(1))
                if testSuiteMatch != None:
                    testSuite = testSuiteMatch.group(1)
    
        fileDir = dirname(filePath)
        fileExecutable = splitext(basename(filePath))[0] + ".exe"
        executablePath = join(fileDir, "bin", fileExecutable)

        # Run each test on a different process
        with Pool(10) as p:
            p.map(runTests, [(executablePath, testSuite, test) for test in tests])
