#include "header.h"

// Search window in kHz to detect the 1pe plateau
const double freq_high = 110; 
const double freq_low  = 10;

///////////////////////////////////////////////////////////////////////////////
// Find TTH function
///////////////////////////////////////////////////////////////////////////////
int FindTTH (double tth[], int limit1, int limit2){

  int chosen_index=0;
  chosen_index = int((limit1+limit2)/2);

  return tth[chosen_index];
}

///////////////////////////////////////////////////////////////////////////////
// Check for failed channels
// change this function! I need a fancy one that returns all types of failed channels
///////////////////////////////////////////////////////////////////////////////
vector <bool> check_failed_channels(double freq[]) {
  // [0] = failed low freq (all points < 80 kHz)
  // [1] = failed low freq (at least 25 points < 10 kHz)
  // [2] = failed high freq (at least 15 points > 500 kHz)
  // [3] = dead TTH (all points f=0KHz)
  // [4] = might be TTH dead (5 points or less f>0KHz)
  // [5] = any high freq (f>500KHz) and zero points low freq (f<80kHz)
  vector <bool> failed;

  int low_freq=0;
  int low_freq2=0;
  int high_freq=0;
  int high_freq2=0;
  int zero_freq=0;

  int chosen_index;

  for (int i=0;i<32;i++){
    if (freq[i]<80)  low_freq++;
    if (freq[i]<10)  low_freq2++;
    if (freq[i]>500) high_freq++;
    if (freq[i]>100) high_freq2++;
    if (freq[i]==0)  zero_freq++;
  }

  if (low_freq>=31)  failed.push_back(1);
  else failed.push_back(0);
  if (low_freq2>=25) failed.push_back(1);
  else failed.push_back(0);
  if (high_freq>=15) failed.push_back(1);
  else failed.push_back(0);
  if (zero_freq>30)  failed.push_back(1);
  else failed.push_back(0);
  if (zero_freq>=25 && zero_freq<30 && high_freq2>0) failed.push_back(1);
  else failed.push_back(0);
  if (high_freq2>0 && low_freq==0) failed.push_back(1);
  else failed.push_back(0);

  return failed;
}

///////////////////////////////////////////////////////////////////////////////
// Function to find limit1 for frequencies lower than freq_high
///////////////////////////////////////////////////////////////////////////////
int FindLimit1(double freq[], double tth[], int n, bool refine) {
  int index=0;
  int aux=0;
  double max;

  max = freq[0];
  index = 0;


  for (int i = 0; i < n; i++) {
    if (freq[i] > max) {
      aux = i;
      max = freq[i];
    }
    else continue;
  }
  //max=0;
  if (refine==false){
      for (int i = aux ; i < n; i++) {
          //if (freq[i]<freq_high && freq[i] > max) {
          if (freq[i]<freq_high) {
              index = i;
              max = freq[i];
              break;
          }
          else continue;
      }
  }
  if (refine==true){
      for (int i = aux ; i < n; i++) {
          //if (freq[i]<freq_high && freq[i] > max) {
          if (freq[i]<200) {
              index = i;
              max = freq[i];
              break;
          }
          else continue;
      }
  }
      max=0;
      if (index==0){
          for (int i = 0 ; i < n; i++) {
              if (freq[i] > max) {
                  index = i;
                  max = freq[i];
              }
              else continue;
          }
      }

      return tth[index];
  }

  ///////////////////////////////////////////////////////////////////////////////
  // Find limit2 function
  ///////////////////////////////////////////////////////////////////////////////
  int FindLimit2 (double freq[], int limit1){

      int chosen_index=0;
      int limit2=0;

      TGraph *g_test = new TGraph();
      TF1 *p1 = new TF1("p1","pol1",0,31);
      for (int i=limit1+3;i<32-3;i++){
          g_test->SetPoint(0,i,freq[i]);
          g_test->SetPoint(1,i+1,freq[i+1]);
          g_test->SetPoint(2,i+2,freq[i+2]);
          g_test->SetPoint(3,i+3,freq[i+3]);
          g_test->Fit(p1,"QR+");
          if(p1->GetParameter(1) < -2 && freq[i]>10)
              limit2 = i;
      }
      if (limit2<limit1) limit2=limit1;

      return limit2;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // Find limit1 function in case low frequency or only f > 100 kHz
  ///////////////////////////////////////////////////////////////////////////////
  int FindLimit1_outWindow (double freq[]){

      int chosen_index;
      int limit1=0;

      for (int i=0;i<32;i++){
          if(freq[i]>10 && freq[i]<500 && freq[i+1]<500){
              limit1 =i; 
              break;
          }
      }
      if (limit1==0){
          for (int i=0;i<32;i++){
              if(freq[i]>5){
                  limit1 =i; 
                  break;
              }
          }
      }
      return limit1;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // Find limit2 function in case low frequency or only f > 100 kHz
  ///////////////////////////////////////////////////////////////////////////////
  int FindLimit2_outWindow (double freq[], int limit1){

      int limit2=0;

      for (int i=31;i>=1;--i){
          if(freq[i]>10 && freq[i-1]>10){
              limit2 = i; 
              break;
          }
      }
      if (limit2<limit1) limit2=limit1;
      return limit2;
  }
