import os
import subprocess

import subprocess
import platform

buildType = "internal"
projectName = "PEngineRuntime"

buildPlatform = platform.system().lower()
buildArgument = f"{buildPlatform}-{buildType}"

rootDir = os.getcwd()


def buildPathFromRoot(*subdirs):
    return os.path.join(rootDir, *subdirs)


binDir = buildPathFromRoot("build", buildArgument)
binPath = buildPathFromRoot(binDir, projectName)

print(binPath)
subprocess.run([binPath])
