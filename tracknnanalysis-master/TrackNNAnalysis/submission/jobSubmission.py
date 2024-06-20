import abc
import os
import argparse
import copy
from itertools import chain
import shutil
import shlex
import subprocess
import time

############################
## Base Class
############################
class submission:
    def __init__(self, args):
        self.args = args
        self.jobList = []

    def addJob(self, job):
        # store the Jobs in a list
        self.jobList.append(job)

    def _remergeReplitSubjobs(self, size):
        # if not resplit subjobs
        if(size == -1):
            mergedSubjobList = []
            for job in self.jobList:
                mergedSubjobList.append(job)
            return mergedSubjobList


        # flatten the list
        flattenJobList = []
        for job in self.jobList:
            flattenJobList.append(job)

        # print flattenJobList
        # extra the base path
        basePath = flattenJobList[0]["jobOutPath"]
        # basePath = basePath[0:basePath.index('sJob') + 4]

        # create new folder for output jobsLIst
        os.system("mkdir -p jobList/%s/%s" % (basePath, "mergeJob"))
        import random
        random.shuffle(flattenJobList)

        # Merge commands a vector to run co-currently 
        counter     = 0
        mergeJobTmp = []
        tmp         = []
        for job in flattenJobList:
            jobName = "source jobList/%s/%s.sh" %(job["jobOutPath"],job["jobOutName"])

            if(counter >= size):
                mergeJobTmp.append(tmp)
                tmp = []
                counter = 0

            counter += 1
            tmp.append(jobName)


        if(len(tmp) > 0): mergeJobTmp.append(tmp)


        # write the new jobs list
        counter = 0
        filePath = "jobList/%s/%s/" % (basePath, "mergeJob")
        mergedSubjobList = []

        for job in mergeJobTmp:
            fileNameBase = "mergeJob" + str(counter)
            fileName = filePath + "/" +  fileNameBase + ".sh"
            fileObj = open(fileName, 'w')
            fileObj.write("#!/bin/bash\n\n")

            for subJob in job:
                fileObj.write(subJob + "\n")

            fileObj.write("\n\n")

            fileObj.close()

            os.system("chmod u+x " + fileName)
            outJob = {}
            outJob["jobOutPath"] = "%s/%s/" % (basePath, "mergeJob")
            outJob["jobOutName"] = fileNameBase

            mergedSubjobList.append(outJob)

            counter += 1

        return mergedSubjobList

    def _getJobList(self):
        # if the user wants to remerge everything then do a new splitting, otherwise do a default split of 1
        if(self.args.remergeResplit):
            print("You are using an 'expert' option that is useful in some cases")
            print("Every subjob you created will be remerged and resplit based on the grouping you gave")
            print("This is useful if you have many small short scans and you dont want to submit each as separate job")
            print("remergeResplitSize controls how many jobs in each subjob")
            finalJobList = self._remergeReplitSubjobs(self.args.remergeResplitSize)
        else:
            finalJobList = self._remergeReplitSubjobs(-1)

        print finalJobList
        return finalJobList

    @abc.abstractmethod
    def process(self):
        pass

    @abc.abstractmethod
    def submitJob(self):
        pass

    def _cleanFolder(self, path):
        return
        # shutil.rmtree(path, ignore_errors=True)

############################
## Dry Run
############################
class doDryRun(submission):
    def __init__(self, args):
        submission.__init__(self, args)
        self.flatJobList = []

    def process(self):
        self.flatJobList = self.jobList

    def submitJob(self):
        # print self.flatJobList
        for subJob in self.flatJobList:
            jobList = "jobList/" + subJob["jobOutPath"] + "/" + subJob["jobOutName"] + ".sh"
            print jobList
               
############################
## Condor
############################
class submitCondor(submission):
    def __init__(self, args):
        submission.__init__(self, args)
        self.submitCommands = []

    def _makeCondorExecutable(self, subDir):
        executableName = "condor/" + subDir + "/condorExe.sh"

        fileObj = open(executableName, 'w')
        fileObj.write("#!/bin/bash\n\n")
        # cd into the folder
        fileObj.write("cd %s\n" %os.getcwd())

        ##setup root
        ##fileObj.write("source setup.sh\n")

        # run the command which is a input argument
        fileObj.write("source $1\n")

        fileObj.close()

        os.system("chmod u+x " + executableName)

        return executableName 

    def _makeCondorSteer(self, currJob, subDir):
        executableName = "condor/" + subDir + "/condorSteer.sub"

        fileObj = open(executableName, 'w')
        fileObj.write("Executable = condor/%s/condorExe.sh \n" % subDir)
        fileObj.write("Universe = vanilla \n")
        fileObj.write("Notification = Error \n")
        fileObj.write("GetEnv = True \n\n")

        for subJob in currJob:
            condorPath = "condor/" + subJob["jobOutPath"]
            # if the folder doesn't exist create it
            if(not os.path.isdir(condorPath)):
                os.system("mkdir -p %s" % (condorPath))
            
            fileObj.write("Arguments = jobList/%s/%s.sh \n" %(subJob["jobOutPath"],subJob["jobOutName"]))
            fileObj.write("Output    = condor/%s/%s.out \n" %(subJob["jobOutPath"],subJob["jobOutName"]))
            fileObj.write("Error     = condor/%s/%s.err \n" %(subJob["jobOutPath"],subJob["jobOutName"]))
            fileObj.write("Log       = condor/%s/%s.log \n" %(subJob["jobOutPath"],subJob["jobOutName"]))
            fileObj.write("Queue \n\n")

        os.system("chmod u+x " + executableName)

    # remerge list based on output folder to reduce number of jobs
    def _remergeListForCondor(self, inList):

        remergeMap = {}
        for subJob in inList:
            # print subJob["jobOutPath"]
            fName = subJob["jobOutPath"].split("/")[0]
            if fName in remergeMap:
                remergeMap[fName].append(subJob)
            else:
                remergeMap[fName] = [subJob]

        remergeList = []
        for l in remergeMap:
            remergeList.append(remergeMap[l])

        return remergeList

    def process(self):

        directory = "condor"
        finalJobList = self._getJobList()
        finalJobList = self._remergeListForCondor(finalJobList)
        
        ## The printing of the options to lists
        ## makes the condor steer and exe for each job grouping
        for job in finalJobList:
            subFolderName = directory + "/" + job[0]["jobOutPath"].split("/")[0]
            ## remove previous condor scripts
            self._cleanFolder(subFolderName)

            subDir = job[0]["jobOutPath"]
            os.system("mkdir -p %s/%s" % (directory, subDir))
            
            self._makeCondorSteer(job, subDir)
            self._makeCondorExecutable(subDir)

            condorCmd = "condor_submit -batch-name " + job[0]["jobOutPath"].split("/")[0] + " " + directory + "/" + subDir + "/condorSteer.sub"
            self.submitCommands.append(condorCmd)

        print("Condor submission scripts written in condor/")

    def submitJob(self):
        if(self.args.dryRun):
            print "Dry run of your condor jobs. Here are the submit commands, the condor jobs are configured in condor/"
            for cmd in self.submitCommands:
                print cmd

        else:
            for cmd in self.submitCommands:
                # print cmd
                os.system(cmd)
            print("Condor submission completed")

############################
## Run Local
############################
class runLocal(submission):
    def __init__(self, args):
        submission.__init__(self, args)
        self.flatJobList = []

    def process(self):
        self.directory = "localRunLogs"

        self.flatJobList = self.jobList
        subFolderName = self.directory + "/" + self.flatJobList[0]["jobOutPath"].split("/")[0]

        # remove previous condor scripts
        self._cleanFolder(subFolderName)

    def submitJob(self):
        if(self.args.dryRun):
            print "Dry run of your slurm jobs. Here are the submit commands, the slurm jobs are configured in slurm/"
            for job in self.flatJobList:
                jobListPath = "jobList/" + job["jobOutPath"] +"/" + job["jobOutName"] +  ".sh"
                print(jobListPath)
            return

        # actually start the combine process
        processes = set()
        procList = []
        max_processes = self.args.maxProcs

        ## get the unique directories in the flatendList
        uniqueFolders = list(set(dic["jobOutPath"] for dic in self.flatJobList)) 

        ## create only those
        for folder in uniqueFolders:
            os.system("mkdir -p %s/%s" % (self.directory, folder) )

        i = 0
        totalJobs = len(self.flatJobList)
        ## loop through all the subjobs
        for job in self.flatJobList:
            baseDir = self.directory + "/" + job["jobOutPath"]

            jobListPath = "jobList/" + job["jobOutPath"] +"/" + job["jobOutName"] +  ".sh"

            # print command
            i = i + 1
            if(i%5 == 0):
                precentDone = float(i)/totalJobs * 100
                print('Running command i: ',i, ' percentage done: ' , precentDone)

            shellCommand = "source %s " %jobListPath
            # shellCommand = "echo %s " %jobListPath
            arguments = shlex.split(shellCommand)
            logFile = baseDir +"/" + job["jobOutName"] + ".txt"


            w = open(logFile,'w')

            # start the process
            p = subprocess.Popen(shellCommand, stdout=w, stderr=w, shell=True)


            # store for backup and for the queue
            processes.add(p)

            # check if the current number of processes exceed the maximum number
            if len(processes) >= max_processes:
                #print 'holding since max process exceeded limit'            
                while True:
                    processes.difference_update([p for p in processes if p.poll() is not None])
                    if len(processes) < max_processes:
                        break
                    # sleep for a bit to save polling time
                    time.sleep(10)

        print('Polling for the last few jobs')
        # poll till all the processes are done
        while True:
            processes.difference_update([p for p in processes if p.poll() is not None])
            if len(processes) == 0:
                break
            pass
            # sleep for a bit to save polling time
            time.sleep(5)
        pass
        
        print('Waiting for output files to get written to disk')
        # to wait for the last few files to be written
        time.sleep(5)


############################
## Run Interactive
############################
class runInteractive(submission):
    def __init__(self, args):
        submission.__init__(self, args)
        self.flatJobList = []

    def process(self):
        self.directory = "localRunLogs"

        self.flatJobList = self.jobList
        subFolderName = self.directory + "/" + self.flatJobList[0]["jobOutPath"].split("/")[0]

        # remove previous condor scripts
        self._cleanFolder(subFolderName)

    def submitJob(self):
        if(self.args.dryRun):
            print "Dry run of your slurm jobs. Here are the submit commands, the slurm jobs are configured in slurm/"
            for job in self.flatJobList:
                jobListPath = "jobList/" + job["jobOutPath"] +"/" + job["jobOutName"] +  ".sh"
                print(jobListPath)
            return

        # actually start the combine process
        processes = set()
        procList = []
        max_processes = self.args.maxProcs

        ## get the unique directories in the flatendList
        uniqueFolders = list(set(dic["jobOutPath"] for dic in self.flatJobList)) 

        ## create only those
        for folder in uniqueFolders:
            os.system("mkdir -p %s/%s" % (self.directory, folder) )

        i = 0
        totalJobs = len(self.flatJobList)
        ## loop through all the subjobs
        for job in self.flatJobList:
            baseDir = self.directory + "/" + job["jobOutPath"]

            jobListPath = "jobList/" + job["jobOutPath"] +"/" + job["jobOutName"] +  ".sh"

            shellCommand = "source %s " %jobListPath
            print(shellCommand)
            os.system(shellCommand)


        
        print('Waiting for output files to get written to disk')
        # to wait for the last few files to be written
        time.sleep(5)

############################
## From Cedar Container
############################
class submitCedarContainer(submission):
    def __init__(self, args):
        submission.__init__(self, args)
        self.submitCommands = []

    def _makeCedarContainerExecutable(self, subJob, subDir):
        fileName = "slurm/" + subDir + "/" + subJob["jobOutName"] + ".sh"
        fileObj = open(fileName, 'w')
        outFileName = "stdout_" + subJob["jobOutName"] + ".txt"
        CWD = os.getcwd()
        jobList = CWD + "/jobList/" + subDir + "/" + subJob["jobOutName"] + ".sh"
        create_script = "batchScript \"source {}\" -O {}".format(jobList, fileName)
        os.system(create_script)
        os.system("chmod -R 775 " + fileName)

        return fileName

    def process(self):
        directory = "slurm"
        finalJobList = self._getJobList()
        subFolderName = directory + "/" + finalJobList[0]["jobOutPath"].split("/")[0]

        # remove previous slurm scripts
        self._cleanFolder(subFolderName)

        for job in finalJobList:
            subDir = finalJobList[0]["jobOutPath"]
            os.system("mkdir -p %s/%s" % (directory, subDir))

            submissionFile =  self._makeCedarContainerExecutable(job, subDir);
            time = '12:00:00'
            if("cedarTime" in self.args): time = self.args.cedarTime

            slurmCmd = "sbatch --time={} --mem=4000M {}".format(time, submissionFile)
            self.submitCommands.append(slurmCmd)

        print("Submission scripts for Cedar container written in slurm/")


    def submitJob(self):
        if(self.args.dryRun):
            print "Dry run of your slurm jobs. Here are the submit commands, the slurm jobs are configured in slurm/"
            for cmd in self.submitCommands:
                print(cmd)

        else:
            for cmd in self.submitCommands:
                os.system(cmd)
                #print(cmd)
            print("Cedar container submission completed")
        
