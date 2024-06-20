import os

## find files in a directory of a given pattern
def findFile(dirName, pattern="", regex=""):
    fileList = []
    import os
    import re
    if regex:
        regexPattern = re.compile(regex)

    for files in os.listdir(dirName):
        #print files
        #if pattern in files:
        if ".root" in files  and pattern in files:
            if regex:
                if regexPattern.search(files):
                    fileList.append(dirName + "/" + files)
            else:
                fileList.append(dirName + "/" + files)
    return fileList


def merge_two_dicts(x, y):
    z = x.copy()   # start with keys and values of x
    z.update(y)    # modifies z with keys and values of y
    return z
