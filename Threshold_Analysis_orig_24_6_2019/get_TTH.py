import math, ROOT, os, sys, yaml
import numpy as np
from ROOT import TGraph, TF1, TMath
from ROOT import TCanvas, TMultiGraph, TLegend, TFile
import re

# search ranges for DCR
# sometimes needs to be changed
rangeLo,rangeHi=20,120 # changed 19.09.2018 for recalibration

with open('./config.yml','r') as ymlfile:
	cfg=yaml.load(ymlfile)

ROOT.gROOT.SetBatch(True)

def smooth(data): # smoothen data to improve reproducibility
	data_smooth = []
	data_smooth.append(0.5*(data[0]+data[1]))
	for n in range (1,(len(data)-1)):
		#print len(data), ' ',
		data_smooth.append((data[n-1]+data[n]+data[n+1])/3.)
	data_smooth.append(0.5*(data[len(data)-2]+data[-1]))
	return data_smooth

def biasThresholds():
	global cfg
	channels=64
	#for chip in range(cfg['parameters']['chips']): #chips
	for chip in cfg['parameters']['stic_en_id']:
		stic,scanFile=[],cfg['dirs']['input']+str(chip)+'.txt'
		stic_smooth,scanFile=[],cfg['dirs']['input']+str(chip)+'.txt'
		dataFile,logFile=cfg['dirs']['output']+str(chip)+'.txt',cfg['dirs']['log']+str(chip)+'.log'
		for channel in range(channels): #channels
			data=getData(scanFile,chip,channel)
			data_smooth = smooth(data)
			#tth,found=fitCenter(data)
			tth,found=find_tth_value(data)
			tth_smooth,found_smooth=fitCenter(data_smooth) #found is boolean of whether or not the script
			#found the tth through the algorithm or if it is chosen to be 1 or 30
			stic.append((channel,tth,found))
			stic_smooth.append((channel,tth_smooth,found_smooth))
		#print(stic)
		writeResults(dataFile,logFile,stic_smooth) #Keep this line commented
			# out unless you actually want to rewrite files!
		makePlots(chip,stic,stic_smooth)

def writeResults(dataFile,logFile,stic):
	file=open(dataFile,'w')
	log=open(logFile,'w') #keep track of channels that don't produce plateau
	for chan in stic:
		ch=str(chan[0])
		if len(ch)==1:ch='0'+ch #align text
		tth=int(chan[1])
		if not chan[2]: #Boolean stating whether the algorithm set tth 
			log.write('ch= %s\n' % (chan[0]))
		file.write('ch= %s\ttth= %d\n' % (chan[0],chan[1]))

def getData(scanFile,chip,channel):
	scanFile2=open(scanFile)
	points=[]
	data_list_freq=scanFile2.readlines()
	scanFile2.close()
	for i in range (len(data_list_freq)):
		list_split=re.split(r'\t+',data_list_freq[i])
		for j in range(len(list_split[2])):
			if list_split[2][j]=='=':
				points.append(float(list_split[2][j+1:]))
	return points[32*channel:32*(channel+1)]

def isCenter(tth,points,center):
	div1,div2=max(4,center/12.),max(7,center/8.) #center/2.5; center/2. if data is messy
    #max. acceptable difference between frequencies one and two points away
	#12 and 9 chosen after determining what looks flat when points are plotted
	#print '\n', len(points), ' ', tth, '\n'
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
	centers=[]
	for tth in range(2,len(points)-2):
		if not rangeLo<points[tth]<rangeHi:continue
		if not (abs(points[tth]-points[tth-1]) < max(points[tth]*.05,3)
				and abs(points[tth]-points[tth+1])< max(points[tth]*.05,3)):
			continue
		#Comment out the above three lines if data is messy
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
		return bruteForce(data),False
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
		return bruteForce(data),False
	else:
		return round((start+end)/2),True

def find_tth_value(data):
	points,start,end=getRange(data)
	if start==None or end-start<5:
		return bruteForce(data),False
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
		return bruteForce(data),False
	else:
		return round((start+end)/2),True

def bruteForce(data): # If the normal algorithm fails, set to 1 or 30
	#based on what looks like the tth
	#rangeLo,rangeHi=12,100 # changed 19.09.2018 for recalibration
	for freq in data[len(data)-5:]:
		if rangeLo<freq<rangeHi:return 30 # The plateau is at the end of the tth range
	return  1 # Most of the time we default to 1 unless it's obvious that the
		# plateau is at the end

def makePlots(chip,stic,stic_smooth):
	global cfg,f
	scanFile=cfg['dirs']['input']+str(chip)+'.txt'
	for channel in range(64):
		data=getData(scanFile,chip,channel)
		#print len(data)
		data_smooth = smooth(data)
		TTH=int(stic[channel][1])
		TTH_smooth=int(stic_smooth[channel][1])
		points,g,Multi=[],None,None
		tth=1.0*np.asarray(list(range(0,32)))
		freq=1.0*np.asarray(data)
		g=TGraph(len(tth),tth,freq)
		#print data, '\n', data_smooth, '\n', '\n'
		Multi=TMultiGraph("Scurve_Channel%s" % channel,"")
		Multi.Add(g)
		
		g_smooth=TGraph(len(tth),tth,np.asarray(data_smooth))
		g_smooth.SetLineColor(4)
		Multi.Add(g_smooth)
		
		c1=TCanvas('c1_chip%s_channel%s' % (chip,channel),'Frequencies',1200,800)
		Multi.Draw('AL')
		leg=TLegend()
		leg.AddEntry(g,'Freq vs. TTH Stic %d Channel %d Bias=2' % (chip,channel))
		leg.AddEntry(g_smooth,'Smooth S-curve')
		leg.Draw()
		c1.SetTitle("Channel %d" % channel)
		c1.SetLogy()
		g.SetLineColor(1)
		g.SetLineWidth(3)
		Multi.GetXaxis().SetTitle('TTH')
		Multi.GetYaxis().SetTitle('Frequency (kHz)')
		c1.Update()
		line=TGraph(2,1.0*np.asarray([TTH,TTH]),1.0*np.asarray([0.001,10000]))
		line.SetLineColor(2)
		line.SetLineWidth(3)
		line.Draw('SAME') #Vertical line showing position of TTH value
		line_s=TGraph(2,1.0*np.asarray([TTH_smooth,TTH_smooth]),1.0*np.asarray([0.001,10000]))
		line_s.SetLineColor(3)
		line_s.SetLineWidth(3)
		line_s.Draw('SAME') 
		leg.AddEntry(line,'TTH = %d' % TTH)
		leg.AddEntry(line_s,'TTH smooth = %d' % TTH_smooth)
		
		leg.Draw()


		f.cd("stic_%d" % chip)
		c1.Write()
	plotAll(chip,scanFile)

def plotAll(chip,scanFile):
	global f
	graphs=TMultiGraph()
	graphList=[None]*64 #https://root-forum.cern.ch/t/array-of-tgrapherrors/17441
		#This list is to ensure that TGraphs do not get deleted before adding them
		#to the TMultiGraph
	for channel in range(64):
		graphList[channel]=TGraph(32,1.0*np.asarray(list(range(0,32))),
			1.0*np.asarray(getData(scanFile,chip,channel)))
		# Appending doesn't work; it deletes the TGraphs
	for channel in range(64):graphs.Add(graphList[channel])
	c=TCanvas('All_Channels','Frequencies',1200,800)
	c.SetLogy()
	c.SetTitle('Chip %d' % chip)
	graphs.Draw('AL')
	graphs.GetXaxis().SetTitle('TTH')
	graphs.GetYaxis().SetTitle('Frequency (kHz)')

	c.Update()
	f.cd('stic_%d' % chip)
	c.Write()

rootFile,rootMode=cfg['root']['dir'],cfg['root']['mode']
f = TFile(rootFile,rootMode)

#print cfg['parameters']['stic_en_id']
for chip in cfg['parameters']['stic_en_id']:f.mkdir("stic_%d" % chip)
#for chip in range(cfg['parameters']['chips']):f.mkdir("stic_%d" % chip)

biasThresholds()

