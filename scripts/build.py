import os
import subprocess
import sys

rootDir = os.getcwd()


def buildPathFromRoot(*subdirs):
    return os.path.join(rootDir, *subdirs)


if __name__ == "__main__":
    preset: str = sys.argv[1]
    buildDir = buildPathFromRoot("build", preset)
    
    if not os.path.exists(os.path.join(buildDir, "CMakeCache.txt")):
        subprocess.run(["cmake", "--preset", preset, "-S", rootDir])
    
    
    subprocess.run(["cmake", "--build", buildDir])
