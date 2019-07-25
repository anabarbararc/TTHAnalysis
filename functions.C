#include "header.h"

// Search window in kHz to detect the 1pe plateau
const double freq_high = 110; 
const double freq_low  = 10;

// Find TTH function
int FindTTH (double tth[], int limit1, int limit2){

  int chosen_index=0;
  chosen_index = int((limit1+limit2)/2);

  return tth[chosen_index];
}

// Function to find maximum for frequency lower than freq_high
int FindLimit1(double freq[], double tth[], int n, int failed_channels) {
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
  max=0;
  for (int i = aux ; i < n; i++) {
    if (freq[i]<freq_high && freq[i] > max) {
      index = i;
      max = freq[i];
    }
    else continue;
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

// Check for failed channels
int check_failed_channels(double freq[]) {
  // 0 = perfect
  // 1 = failed low freq
  // 2 = failed low freq + at least 1 point has f=0kHz
  // 3 = failed high freq
  // 4 = failed more than 15 points with f=0kHz
  // 5 = failed all points with f=0KHz
  int low_freq=0;
  int low_freq2=0;
  int high_freq=0;
  int zero_freq=0;
  int failed=0;

  int chosen_index;

  for (int i=0;i<32;i++){
    if (freq[i]<80) low_freq++;
    if (freq[i]<10) low_freq2++;
    if (freq[i]>500) high_freq++;
    if (freq[i]==0) zero_freq++;
  }

  if (low_freq>=31) failed =1;
  //if (low_freq2>=25) failed = 2;
  if (high_freq>15) failed = 3;
  //if (zero_freq>=15) failed = 4; 
  if (zero_freq>30) failed = 5;

  return failed;
}

// Check for channels with high frequency
bool check_highfreq(TGraph *g) {

  double freq[32];
  double tth[32];
  int high_freq=0;
  bool HasHighFreq;
  int chosen_index;

  // filling up array with freq and tth values
  for (int i=0;i<32;i++)
    g->GetPoint(i,tth[i],freq[i]);

  for (int i=0;i<32;i++){
    if (freq[i]>500) high_freq++;
    else continue;
  }
  if (high_freq>=15) HasHighFreq=true;
  else HasHighFreq=false;

  return HasHighFreq;
}

// Find limit1 function in case low frequency
int FindLimit1_low (double freq[]){

  int chosen_index;
  int limit1=0;

  for (int i=0;i<32;i++){
      if(freq[i]>10){
          limit1 =i; 
          break;
      }
  }
  return limit1;
}
// Find limit2 function in case low frequency
int FindLimit2_low (double freq[]){

  int limit2=0;

  for (int i=31;i>=1;--i){
      if(freq[i]>10 && freq[i-1]>10){
          limit2 = i; 
          break;
      }
  }
  return limit2;
}

// Find limit2 function
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
    if(p1->GetParameter(1) < -2)
      limit2 = i-1;
  }

  return limit2;
}
