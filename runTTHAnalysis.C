void runTTHAnalysis(std::string board)
{
  // print only messages from warning messages
  gErrorIgnoreLevel = kWarning;

  gROOT->ProcessLine(".L functions.C++");
  gROOT->ProcessLine(".L getTTH.C++");

  std::string cmd = "getTTH(\"" + board + "\")"; 
  gROOT->ProcessLine(cmd.c_str());
}
