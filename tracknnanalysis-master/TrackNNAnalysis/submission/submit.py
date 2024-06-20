#!/usr/bin/env python

import os, sys
import argparse
from jobSubmission import *

parser = argparse.ArgumentParser()
parser.add_argument("--queue",                      default = "workday", type = str,      help = "Queue to submit to.")
parser.add_argument("--dryRun",                     default = False, action='store_true',   help = "Does not submit to anything") 
parser.add_argument("--runLocal",                   default = False, action='store_true',   help = "Run in parallel") 
parser.add_argument("--runInteractive",             default = False, action='store_true',   help = "Run in interactive move") 
parser.add_argument("--submitCondor",               default = False, action='store_true',   help = "Submit jobs to the batch") 
parser.add_argument("--submitSlurm",                default = False, action='store_true',   help = "Submit jobs to the batch") 
parser.add_argument("--submitCedarContainer",       default = False, action='store_true',   help = "submission for cedarContainer")
parser.add_argument("--cedarTime",                  default = "6:00:00", type = str,        help = "Time for a job to run on Cedar. Needs to be in format hh:mm:ss")
parser.add_argument("--maxProcs",                   default = 4,     type=int,              help = "Number of parallel processed")

parser.add_argument("--remergeResplit",             default = False, action='store_true',   help = "expert option to merge and split jobs")
parser.add_argument("--remergeResplitSize",         default = 1,     type = int,            help = "expert option to find how many to merge jobs with")


parser.add_argument("--doPickle",                   default = False, action='store_true',   help = "Run jobs for pickling") 
parser.add_argument("--doEval",                     default = False, action='store_true',   help = "Run jobs for eval") 
parser.add_argument("--doExtrap",                   default = False, action='store_true',   help = "Run jobs for extrap eval") 
parser.add_argument("--doExtrapLayers",             default = False, action='store_true',   help = "Run jobs for extrap eval") 
parser.add_argument("--doPredError",                default = False, action='store_true',   help = "Run jobs for evaluating the error on the prediction") 
parser.add_argument("--doCircleFit",                default = False, action='store_true',   help = "Run jobs for evaluating the error on the prediction") 

args = parser.parse_args()


submissionJobList = []

if args.doPickle:
    import jobListPickle
    submissionJobList += [jobListPickle.PickleList]

if args.doEval:
    import jobListEval
    submissionJobList += [jobListEval.HTTEval]

if args.doExtrap:
    import jobListExtrap
    j = jobListExtrap.Eval
    #del j['zUncertainty']
    #del j['phiUncertainty']
    submissionJobList += [j]

if args.doExtrapLayers:
    import jobListExtrap
    submissionJobList += [jobListExtrap.Eval]

if args.doPredError:
    import jobListPredError
    submissionJobList += [jobListPredError.Eval]

if args.doCircleFit:
    import jobListCircleFit
    submissionJobList += [jobListCircleFit.Eval]

def main():
    submissionManager = submissionFactory(args)
    index = 0

    ##############################################################
    ##                     Configure the jobs                   ##
    ##############################################################
    for submissionJob in submissionJobList:
        for job in submissionJob:

            bsubBaseDir = "bsub/"
            if(args.submitCondor): bsubBaseDir = "condor/" 
            elif(args.submitSlurm or args.submitCedarContainer):   bsubBaseDir     = "slurm/" 

            bsubBaseDir += "/" + job['baseName']
            os.system("mkdir -vp " + bsubBaseDir)

            cmd = ""
            if(job["type"] == "HTTEff"):
                index += 1 
                exePath = "evalEffHTT"
                cmd = _getevalHTTCommand(exePath, job) 
            elif(job["type"] == "Pickle"):
                index += 1 
                exePath = "processForPickle"
                cmd = _getPickleCommand(exePath, job) 
            elif(job["type"] == "PickleHTT"):
                index += 1 
                exePath = "processForPickleHTT"
                cmd = _getHTTPickleCommand(exePath, job) 
            elif(job["type"] == "PickleACTS"):
                index += 1 
                exePath = "processForPickleACTS"
                cmd = _getACTSPickleCommand(exePath, job) 
            elif(job["type"] == "RDOTraining"):
                index += 1 
                exePath = "prepareTrainingInputs_ATLASRDO"
                cmd = _getRDOTrainingCommand(exePath, job) 
            elif(job["type"] == "EvalExtrapEff"):
                index += 1 
                exePath = "evalEffExtrapThreads"
                cmd = _getEvalExtrapCommand(exePath, job) 
            elif(job["type"] == "evalEffExtrapThreadsLayer"):
                index += 1
                exePath = "evalEffExtrapThreadsLayer"
                cmd = _getEvalExtrapLayersCommand(exePath, job)
            elif(job["type"] == "EvalPredError"):
                index += 1 
                exePath = "evalPredError"
                cmd = _getEvalPredCommand(exePath, job) 
            elif(job["type"] == "CircleFit"):
                index += 1
                exePath = "testCircleFit"
                cmd = _getCircleFitCommand(exePath, job)
            else: 
                print("Type not recognized, ", job["type"] )
                exit(1)

            cInfo = {}
            cInfo["jobOutPath"] = job["baseName"]
            cInfo["jobOutName"] = job['name']

            os.system("mkdir -p jobList/%s" % cInfo["jobOutPath"])
            _writeJobList(cInfo["jobOutPath"], cInfo["jobOutName"], cmd);

            submissionManager.addJob(cInfo)


    ##############################################################
    ##                      Submit the jobs                     ##
    ##############################################################
    ## This is the actual submission to different systems, e.g. condor, batch, grid, local
    ## Information from the jobManager is re-organized here 
    submissionManager.process()
    submissionManager.submitJob()

def _writeJobList(jobOutPath, jobOutName, cmd):
    fileName = "jobList/" + jobOutPath + "/" + jobOutName + ".sh"
    fileObj = open(fileName, 'w')
    fileObj.write("#!/bin/bash\n\n")
    if(not (args.runLocal or args.runInteractive)): 
        CWD = os.getcwd()
        fileObj.write("cd " + CWD + "\n")
        fileObj.write("cd ../build" + "\n")
        fileObj.write("export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase " + "\n")
        fileObj.write("source $ATLAS_LOCAL_ROOT_BASE/user/atlasLocalSetup.sh" + "\n")
        fileObj.write("asetup --restore" + "\n")
        fileObj.write("cd " + CWD + "\n")
        fileObj.write("source ../build/x86_64-*-opt/setup.sh\n")
        # fileObj.write("source ../source/H4lMassWorkspace/setup_lxplus.sh \n")
    fileObj.write(cmd)
    fileObj.write("\n")
    fileObj.close()

    os.system("chmod u+x " + fileName)

def submissionFactory(args):
    if(args.submitCondor):           return submitCondor(args)
    elif(args.submitCedarContainer): return submitCedarContainer(args)
    elif(args.runLocal):             return runLocal(args)
    elif(args.runInteractive):       return runInteractive(args)
    elif(args.dryRun):               return doDryRun(args)
    else:
        print("You did not enter where to submit to")
        print("-------------------------------------------------")
        exit(1)

def _getEvalExtrapCommand(exePath, currJob):
    cmd =  exePath + " "
    cmd += ' --inputFile %s' %(currJob['inputFile'])
    cmd += ' --outputFile %s' %(currJob['outputFile'])
    cmd += ' --uncertainty %s' %(currJob['uncertainty'])
    cmd += ' --windowSize %s' %(currJob['windowSize'])
    cmd += ' --onnxFile %s' %(currJob['onnxFile'])
    cmd += ' --minHits %s' %(currJob['minHits'])
    cmd += ' --LayerRegion %s' %(currJob['LayerRegion'])
    cmd += ' --outputManagerFile %s' %(currJob["outputManagerFile"])
    cmd += ' --fracOutputTracks %s' %(currJob["fracOutputTracks"])
    cmd += ' --hitType %s' %(currJob["hitType"])
    cmd += ' --deltaROverlap %s' %(currJob["deltaROverlap"])

    if "pileupOnly" in currJob.keys(): cmd += ' --pileupOnly'
    if "muonOnly" in currJob.keys(): cmd += ' --muonOnly'
    if "saveOutputTracks" in currJob.keys(): cmd += ' --saveOutputTracks'
    
    cmd += ' --NNCutVal %s' %(currJob["NNCutVal"])
    cmd += ' --nHitOverlapThres %s' %(currJob["nHitOverlapThres"])

    return cmd

def _getEvalExtrapLayersCommand(exePath, currJob):
    cmd =  exePath + " "
    cmd += ' --inputFile %s' %(currJob['inputFile'])
    cmd += ' --outputFile %s' %(currJob['outputFile'])
    cmd += ' --zUncertainty %s' %(currJob['zUncertainty'])
    cmd += ' --phiUncertainty %s' %(currJob['phiUncertainty'])
    cmd += ' --windowSize %s' %(currJob['windowSize'])
    cmd += ' --onnxFile %s' %(currJob['onnxFile'])
    cmd += ' --minHits %s' %(currJob['minHits'])
    cmd += ' --LayerRegion %s' %(currJob['LayerRegion'])

def _getEvalPredCommand(exePath, currJob):
    cmd =  exePath + " "
    cmd += ' --inputFile %s' %(currJob['inputFile'])
    cmd += ' --outputFile %s' %(currJob['outputFile'])
    cmd += ' --onnxFile %s' %(currJob['onnxFile'])


    return cmd


def _getevalHTTCommand(exePath, currJob):
    cmd =  exePath + " "
    cmd += ' --inputFile %s' %(currJob['inputFile'])
    cmd += ' --outputFile %s' %(currJob['outputFile'])

    if('onnxFile' in currJob):
        cmd += ' --onnxFile %s' %(currJob['onnxFile'])
    if('nHits' in currJob):
        cmd += ' --nHits %s' %(currJob['nHits'])
    if('NNCutVal' in currJob):
        cmd += ' --NNCutVal %s' %(currJob['NNCutVal'])  
    if('nHitOverlapThres' in currJob):
        cmd += ' --nHitOverlapThres %s' %(currJob['nHitOverlapThres'])  
    if('trackType' in currJob):
        cmd += ' --trackType %s' %(currJob['trackType'])  
    if('lwrPtLim' in currJob):
        cmd += ' --lwrPtLim %s' %(currJob['lwrPtLim'])  
    if('uprPtLim' in currJob):
        cmd += ' --uprPtLim %s' %(currJob['uprPtLim'])
    if('LayerRegion' in currJob):
        cmd += ' --LayerRegion %s' %(currJob['LayerRegion'])     
    if('hitType' in currJob):
        cmd += ' --hitType %s' %(currJob['hitType'])             
    if('order' in currJob):
        cmd += ' --order %s' %(currJob['order'])     
    if('pileupOnly' in currJob):
        if (currJob["pileupOnly"] == True):
            cmd += ' --pileupOnly'

    if('muonOnly' in currJob):
        if (currJob["muonOnly"] == True):
            cmd += ' --muonOnly'

    if('doACTS' in currJob):
        if (currJob["doACTS"] == True):
            cmd += ' --doACTS'
    if('scaled' in currJob):
        if (currJob["scaled"] == True):
            cmd += ' --scaled'


    if('extraOpts' in currJob):
       cmd += currJob['extraOpts']

    return cmd

def _getRDOTrainingCommand(exePath, currJob):
    cmd =  exePath + " "
    cmd += ' --inputFile %s' %(currJob['inputFile'])
    cmd += ' --outputFile %s' %(currJob['outputFile'])  
    cmd += ' --hitType %s' %(currJob['hitType'])       
  
    if('nEvents' in currJob):
        cmd += ' --nEvents %s' %(currJob['nEvents']) 

    if('extraOpts' in currJob):
       cmd += currJob['extraOpts']

    return cmd

def _getPickleCommand(exePath, currJob):
    cmd =  exePath + " "
    cmd += ' --inputFile %s' %(currJob['inputFile'])
    cmd += ' --outputFile %s' %(currJob['outputFile'])
    cmd += ' --LayerRegion %s' %(currJob['LayerRegion'])       
    cmd += ' --HTTLayerHits %s' %(currJob['HTTLayerHits'])       
    cmd += ' --hitType %s' %(currJob['hitType'])       

    if('lwrEtaLim' in currJob):
        cmd += ' --lwrEtaLim %s' %(currJob['lwrEtaLim']) 
    if('uprEtaLim' in currJob):
        cmd += ' --uprEtaLim %s' %(currJob['uprEtaLim'])     
    if('nEvents' in currJob):
        cmd += ' --nEvents %s' %(currJob['nEvents']) 

    if('extraOpts' in currJob):
       cmd += currJob['extraOpts']

    return cmd

def _getHTTPickleCommand(exePath, currJob):
    cmd =  exePath + " "
    cmd += ' --inputFile %s' %(currJob['inputFile'])
    cmd += ' --outputFile %s' %(currJob['outputFile'])
    cmd += ' --LayerRegion %s' %(currJob['LayerRegion'])   
    cmd += ' --hitType %s' %(currJob['hitType'])     
    if('nEvents' in currJob):
        cmd += ' --nEvents %s' %(currJob['nEvents']) 

    if('doSubSample' in currJob):
        if (currJob["doSubSample"] == True):
            cmd += ' --doSubSample'


    if('extraOpts' in currJob):
       cmd += currJob['extraOpts']

    return cmd



def _getACTSPickleCommand(exePath, currJob):
    cmd =  exePath + " "
    cmd += ' --inputFile %s' %(currJob['inputFile'])
    cmd += ' --outputFile %s' %(currJob['outputFile'])
    cmd += ' --LayerRegion %s' %(currJob['LayerRegion']) 
    cmd += ' --hitType %s' %(currJob['hitType'])      
    if('nEvents' in currJob):
        cmd += ' --nEvents %s' %(currJob['nEvents']) 
    if('extraOpts' in currJob):
       cmd += currJob['extraOpts']

    return cmd

def _getCircleFitCommand(exePath, currJob):
    cmd =  exePath + " "
    cmd += ' --inputFile %s' %(currJob['inputFile'])
    cmd += ' --outputFile %s' %(currJob['outputFile'])
    cmd += ' --LayerRegion %s' %(currJob['LayerRegion'])

    return cmd

if __name__ == '__main__':
  sys.exit( main() )
