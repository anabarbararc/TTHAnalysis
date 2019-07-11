import math, ROOT, os, sys, yaml
import numpy as np
from ROOT import TGraph,TF1, TMath
import re

with open('./configTEST.yml','r') as ymlfile:
	cfg=yaml.load(ymlfile)

scanFile=cfg['files']['input']
dataFile=cfg['files']['output']
logFile=cfg['files']['log']
chips = cfg['parameters']['chips']
biases = cfg['parameters']['biases']
#yaml.dump(cfg)
#print(cfg)

def biasThresholds():
	global scanFile,dataFile,logFile,chips,biases
	channels=64
	log=open(logFile,'w')
	inFile=open(scanFile,'r')
	log.write('\n')  #I use appending mode later so I have to rewrite it now
	for bias in range(1,biases+1):
		thresholds=[]
		for chip in range(chips):
			chan=[]
			for channel in range(channels): #channels
				data=getData(scanFile,chip,channel,bias)
				chan.append((channel,chip,fitCenter(data)))
			thresholds.append(chan)
		writeResults(dataFile,logFile,thresholds,bias) #Keep this line commented
				# out unless you actually want to rewrite files!
			#Write for every bias
	#writeFailed(findFailures()) #This is only for channels in which no bias
		#produced a valid threshold; it is not used in the final analysis
		# but I'm keeping it here because it could be useful 

def writeResults(dataFile,logFile,thresholds,bias):
	file=open(dataFile+'%d_out.txt' % bias,'w') #Separate files based on biases
	log=open(logFile,'a') #keep track of biases that don't produce plateau
	for stic in thresholds:
		chan=str(stic[0][0])
		if len(chan)==1:chan='0'+chan #align text
		for channel in stic:
			tth=channel[2]
			if tth==None:
				file.write('stic= %s 	ch=%s	tth=???	DAC_INPUTBIAS= 0%d\n' %
									(channel[1],channel[0],bias))
				log.write('stic= %s 	ch= %s	DAC_INPUTBIAS= 0%d\n' %
									(channel[1],channel[0],bias))
			else:file.write('stic= %s 	ch=%s	tth=%d	DAC_INPUTBIAS= 0%d\n' %
									(channel[1],channel[0],tth,bias))

def writeFailed(failed):
	failedFile=open('totalFailures.txt','w')
	for fail in failed:
		failedFile.write('\n%s\t%s' % (fail[0],fail[1]))

def getData(scanFile,chip,channel,bias):
	scanFile2=open(scanFile)
	points=[]
	freq_temp=[]
	data_list_freq=scanFile2.readlines()
	scanFile2.close()
	for i in range (0,len(data_list_freq)):
		list_split=re.split(r'\t+',data_list_freq[i])
		for i in range(len(list_split[chip+3])):
			if list_split[chip+3][i]=='=':
				freq_temp.append(float(list_split[chip+3][i+1:])) 
	points=freq_temp[((5*channel+bias-1)*32):((5*channel+bias)*32)] # Calculates
		# the location in the file of the points we need
	return points

def isCenter(tth,points,center):
    rangeLo,rangeHi=18,100
    div1,div2=max(4,center/12.),max(7,center/8.)
    #max. acceptable difference between frequencies one and two points away
    #12 and 9 chosen after determining what looks flat when points are plotted
    if not rangeLo<=points[tth]<=rangeHi:return False
    if tth==31:
        return (abs(points[tth]-points[tth-1]) < div1 and
                abs(points[tth]-points[tth-2]) < div2)
    elif tth==0:
        return (abs(points[tth]-points[tth+1]) < div1 and
                abs(points[tth]-points[tth-2]) < div2)
    else:
        return (abs(points[tth]-points[tth-1]) < div1 and
                abs(points[tth]-points[tth+1]) < div1 and
                abs(points[tth-1]-points[tth+1]) < div2)

def getCenters(points):
	rangeLo,rangeHi=18,100
	centers=[]
	for tth in range(2,len(points)-2):
		if not rangeLo<points[tth]<rangeHi:continue
		if not (abs(points[tth]-points[tth-1]) < max(points[tth]*.05,3)
				and abs(points[tth]-points[tth+1])< max(points[tth]*.05,3)):
			continue
		for i in range(-2,4):
			if i==3: #5 points in range that do not differ too much
				centers.append((tth,points[tth]))
				break
			if not (rangeLo<points[tth+i]<rangeHi):break
	return centers

def getRange(data):
	possCen=getCenters(data) #My new algorithm checks all possible centers
			# and uses the one with the longest range to avoid rejecting a channel
			# with two groupings in the 1-photon range
	if possCen==[]:return None,None,31
	maxRange,start,end=-1,None,None
	for point in possCen:
		tempStart,tempEnd=None,None
		for tth in range(point[0],0,-1):
    	    #loop down and up from center to ensure correct range
			if isCenter(tth,data,point[1]):
				tempStart=max(0,tth-1) #Cannot have negative tth
			else:break
		for tth in range(point[0],len(data)):
			if isCenter(tth,data,point[1]):
				tempEnd=min(31,tth+1) #The next point is part of the range
				# (isCenter rejects the edge point), but you cannot have TTH=32
			else:break
		if (isinstance(tempEnd,int) and isinstance(tempStart,int) and 
			tempEnd-tempStart>maxRange):
			start,end,maxRange=tempStart,tempEnd,tempEnd-tempStart
	return data,start,end  
    
def fitCenter(data):
	points,start,end=getRange(data)
	if start==None or end-start<5:
		return None
	maxSlope=3.0 #chosen based on what "looks flat"
	tth,freq=list(),list()
	for i in range(start,end):
		tth.append(i);freq.append(points[i])
	tth=1.0*np.asarray(tth);freq=np.asarray(freq) #tth must be floats
	gr=TGraph(len(tth),tth,freq) 
	line=TF1('line','pol1',tth[0],tth[-1])
	stats=gr.Fit('line','Q','',tth[0],tth[-1])
	slope,stderr=line.GetParameter(1),line.GetParError(1)
	if abs(slope)>maxSlope and abs(slope/stderr)>3: # I only reject cases in
		#which there is statistical difference and a large slope value
        #because small slopes with small errors are still flat enough
		return None
	else:
		return round((start+end)/2)

def findFailures():
	fails,totalFails=[],[]
	data=open('/home/localuser/Desktop/TestingROOT/failedBias.txt')
	data_list_freq=data.readlines()
	data.close()
	for i in range (0,len(data_list_freq)):
		list_split=re.split('\t+',data_list_freq[i])
		list_split.pop()
		fails.append(list_split)
	for fail in fails:
		if fails.count(fail)==5 and fail not in totalFails:
			totalFails.append(fail)
	return totalFails

				
biasThresholds()
