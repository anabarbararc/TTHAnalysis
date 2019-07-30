#include <iostream>
#include <fstream>
#include <string>

#include <TGraph.h>
#include <TFile.h>
#include <TLine.h>
#include <TLatex.h>
#include <TAxis.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TH1.h>

using namespace std;

// Find TTH fucntion
int FindTTH (double arr[], int lim1, int lim2);

// Function to find maximum within the frequency range
int find_maximum(double arr[], int n, int failed_channels);

// check for failed channels
int check_failed_channels(double arr[]);

// check if more than 15 channels have High Frequency (>500kHz)
bool check_highfreq(TGraph *g);

// find limit on the left edge
int FindLimit1(double arr[], double arr1[], int n, int failed_channels);

// find limit on the right edge
int FindLimit2 (double arr[], int lim1);

// get TTH function
int getTTH(string board);

// find limit on the left edge
int FindLimit1_outWindow (double arr[]);

// find limit on the right edge
int FindLimit2_outWindow (double arr[], int lim1);
