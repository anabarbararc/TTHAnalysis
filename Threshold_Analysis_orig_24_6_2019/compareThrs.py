import numpy as np
from array import array
from ROOT import TH1F, TH2F, TCanvas, TGraph, TLegend, TF1, TMath, TMultiGraph, TFile, TTree, TNtuple, TIter, TObjLink, TColor, TPad, TLine, TStyle
import ROOT
import sys

dirComp = [a for a in sys.argv if (a!="-i" and "compareThrs.py" not in a and not isinstance(a, int))]
chList = [int(a) for a in sys.argv if (a!="-i" and "compareThrs.py" not in a and isinstance(a, int))]

print dirComp
print chList
"""
for d in dirComp:
    fullpath = d + "/" + "root_data.root"
    # Get Vbd
    f = TFile(fullpath,"r")
    print("Analyzing file: " + fullpath)
    # Get the canvas
    c = f.Get("Correlated noise")
    # Get the graphs on the canvas
    obj = ROOT.TObject()
    Next = ROOT.TIter(c.GetListOfPrimitives())
    obj = Next()
    #~ gTot = Next() # total correlated noise graph
    #~ gDirXtalk = Next() # direct cross-talk graph
    #~ gAP = Next() # afterpulse graph
    #~ gDelXtalk = Next() # delayed cross-talk graph
    
    graphs = {}
    for corr_noise_type in corr_noise_types:
        graphs[corr_noise_type] = Next()
    
    for oppt in op_pts:
        
        for corr_noise_type in corr_noise_types:
            
            mean_corr_noise[oppt][corr_noise_type]["mean"] += graphs[corr_noise_type].Eval(oppt)
            mean_corr_noise[oppt][corr_noise_type]["rms"] += graphs[corr_noise_type].Eval(oppt)*graphs[corr_noise_type].Eval(oppt)
            mean_corr_noise[oppt][corr_noise_type]["n"] += 1
            if graphs[corr_noise_type].Eval(oppt) > mean_corr_noise[oppt][corr_noise_type]["max"]:
                mean_corr_noise[oppt][corr_noise_type]["max"] = graphs[corr_noise_type].Eval(oppt)
                mean_corr_noise[oppt][corr_noise_type]["detector_max"] = det
            if graphs[corr_noise_type].Eval(oppt) < mean_corr_noise[oppt][corr_noise_type]["min"]:
                mean_corr_noise[oppt][corr_noise_type]["min"] = graphs[corr_noise_type].Eval(oppt)
                mean_corr_noise[oppt][corr_noise_type]["detector_min"] = det

    
    corr_tree.Fill(graphs["Tot"].Eval(distr_pt), graphs["DirXtalk"].Eval(distr_pt), graphs["DelXtalk"].Eval(distr_pt), graphs["AP"].Eval(distr_pt))

for op_pt in op_pts:
    print("OP {0:.1f}".format(op_pt))
    for corr_noise_type in corr_noise_types:
        mean_corr_noise[op_pt][corr_noise_type]["mean"] = mean_corr_noise[op_pt][corr_noise_type]["mean"]/mean_corr_noise[op_pt][corr_noise_type]["n"]
        mean_corr_noise[op_pt][corr_noise_type]["rms"] = TMath.Sqrt((mean_corr_noise[op_pt][corr_noise_type]["rms"]/mean_corr_noise[op_pt][corr_noise_type]["n"])-(mean_corr_noise[op_pt][corr_noise_type]["mean"]*mean_corr_noise[op_pt][corr_noise_type]["mean"]))
        mean_corr_noise[op_pt][corr_noise_type]["err_max"] = max((mean_corr_noise[op_pt][corr_noise_type]["mean"]-mean_corr_noise[op_pt][corr_noise_type]["min"]),(mean_corr_noise[op_pt][corr_noise_type]["max"]-mean_corr_noise[op_pt][corr_noise_type]["mean"]))
        
        # gives which detector had minimum/maximum correlated noise
        print('\033[91m'+"maximum {} (corr.noise)".format(corr_noise_type)+" of {0:.1f}".format(mean_corr_noise[op_pt][corr_noise_type]["max"])+" at detector {}".format(mean_corr_noise[op_pt][corr_noise_type]["detector_max"])+'\033[0m')
        print('\033[92m'+"minimum {} (corr.noise)".format(corr_noise_type)+" of {0:.1f}".format(mean_corr_noise[op_pt][corr_noise_type]["min"])+" at detector {}".format(mean_corr_noise[op_pt][corr_noise_type]["detector_min"])+'\033[0m')

gMean = {corr_noise_type: ROOT.TGraphAsymmErrors(len(op_pts)) for corr_noise_type in corr_noise_types}

for i,op_pt in enumerate(op_pts):
    for corr_noise_type in corr_noise_types:
        gMean[corr_noise_type].SetPoint(i,op_pt,mean_corr_noise[op_pt][corr_noise_type]["mean"])
        #gMean[corr_noise_type].SetPointError(i,0,mean_corr_noise[op_pt][corr_noise_type]["err_max"])
        #gMean[corr_noise_type].SetPointError(i,0,0,mean_corr_noise[op_pt][corr_noise_type]["mean"]-mean_corr_noise[op_pt][corr_noise_type]["min"],mean_corr_noise[op_pt][corr_noise_type]["max"]-mean_corr_noise[op_pt][corr_noise_type]["mean"])
        gMean[corr_noise_type].SetPointError(i,0,0,mean_corr_noise[op_pt][corr_noise_type]["rms"],mean_corr_noise[op_pt][corr_noise_type]["rms"])

cnoise = TCanvas("MeanCorrNoise","MeanCorrNoise",800,600)
frame = TH2F("frame","frame",100,op_pts[0]-0.5,op_pts[-1]+0.5,int(float(max_range)/0.1),0,max_range)
frame.SetTitle("Correlated noise averaged over " + str(len(detectors_noise)) + " channels")
frame.GetXaxis().SetTitle("#DeltaV [V]")
frame.GetYaxis().SetTitle("Correlated noise [%]")
frame.Draw()

cnoisedistr = TCanvas("CorrelatedNoiseDistr","CorrelatedNoiseDistr",800,600)
framedistr = TH2F("framedistr","framedistr",int(float(corr_tree.GetMaximum("Tot")*1.2)/0.1),0,corr_tree.GetMaximum("Tot")*1.2,150,0,70)
framedistr.SetTitle("Correlated noise at #DeltaV="+str(distr_pt)+"V for " + str(len(detectors_noise)) + " channels")
framedistr.GetXaxis().SetTitle("Correlated noise [%]")
framedistr.GetYaxis().SetTitle("Number of entries")
framedistr.Draw()

leg = ROOT.TLegend(0.19,0.65,0.53,0.89)
#leg.SetFillColor(0)
legdistr = ROOT.TLegend(0.55,0.7,0.89,0.89)
#legdistr.SetFillColor(0)

for corr_noise_type in corr_noise_types:
    gMean[corr_noise_type].SetMarkerStyle(24)
    
    color = 0
    legent = ""
    if corr_noise_type=="Tot":
        color = 2
        legent = "Total"
    
    if corr_noise_type=="DirXtalk":
        color = 4
        legent = "Direct cross-talk"
    
    if corr_noise_type=="DelXtalk":
        color = ROOT.kGreen+2
        legent = "Delayed cross-talk"
    
    if corr_noise_type=="AP":
        color = ROOT.kOrange+8
        legent = "After-pulse"
    
    cnoise.cd()
    gMean[corr_noise_type].SetMarkerColor(color)
    gMean[corr_noise_type].SetLineColor(color)
    gMean[corr_noise_type].SetFillStyle(3003)
    gMean[corr_noise_type].SetFillColor(color)
    #leg.AddEntry(gMean[corr_noise_type],legent,"pl")
    leg.AddEntry(gMean[corr_noise_type],legent,"f")
    gMean[corr_noise_type].Draw("PL+3")
    
    cnoisedistr.cd()
    corr_tree.Draw(corr_noise_type,"","same")
    htempnoise = cnoisedistr.GetPrimitive("htemp")
    htempnoise.SetName("distr"+corr_noise_type)
    htempnoise.SetLineColor(color)
    htempnoise.SetLineWidth(2)
    htempnoise.SetFillColor(color)
    # ~ htempnoise.SetFillStyle(3001)
    htempnoise.SetFillStyle(3005)
    legdistr.AddEntry(htempnoise,legent,"f")
    htempnoise.Rebin(rebin)

cnoise.cd()
frame.SetStats(0)
cnoise.SetGrid(1)
ROOT.gPad.Update()
ROOT.gPad.RedrawAxis()
l = ROOT.TLine()
l.DrawLine(ROOT.gPad.GetUxmin(), ROOT.gPad.GetUymax(), ROOT.gPad.GetUxmax(), ROOT.gPad.GetUymax());
l.DrawLine(ROOT.gPad.GetUxmax(), ROOT.gPad.GetUymin(), ROOT.gPad.GetUxmax(), ROOT.gPad.GetUymax());
l.DrawLine(ROOT.gPad.GetUxmin(), ROOT.gPad.GetUymin(), ROOT.gPad.GetUxmax(), ROOT.gPad.GetUymin());
leg.Draw()
cnoise.SaveAs("cnoise_" + str(len(detectors_noise)) + "det.pdf")
cnoisedistr.cd()
legdistr.Draw()
framedistr.SetStats(0)
cnoisedistr.SetGrid(1)
cnoisedistr.SaveAs("cnoisedistr_" + str(len(detectors_noise)) + "det.pdf")
"""
