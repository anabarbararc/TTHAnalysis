#include "header.h"

const double freq_high = 100;
const double freq_low  = 20;

// Find TTH function
int FindTTH (TGraph *g){

  double freq[32];
  double tth[32];

  int chosen_index=0;

  // tth limits to calculate average of tth
  int limit1=0;
  int limit2=0;

  // auxiliar parameters
  double tan[31]; // dy/dx

  // filling up array with freq and tth values
  for (int i=0;i<32;i++)
    g->GetPoint(i,tth[i],freq[i]);

  int failed_channels = check_failed_channels(g);

  // filling up dy and tan
  for (int i=0;i<31;i++){
    tan[i]=abs(freq[i+1]-freq[i]);
    //cout <<  i << "\t " << tth[i] << "\t " << freq[i] << "\t " << tan[i] << endl;
  }

  // set value for limit1
  //if(failed_channels==0 || failed_channels==2) limit1 = tth[find_maximum(freq,31,failed_channels)];
  if(failed_channels==1) limit1 = 1;
  else limit1 = tth[find_maximum(freq,31,failed_channels)];

  limit2 = FindLimit2(g);
  // if limit2 0, set value for 31
  if (limit2==0) limit2=31;

  chosen_index = int((limit2+limit1)/2);

  return tth[chosen_index];
}

// Function to find maximum for frequency lower than freq_high
int find_maximum(double a[], int n, int failed_channels) {
  int index=0;
  int aux=0;
  double max;

  max = a[0];
  index = 0;


  for (int i = 0; i < n; i++) {
    if (a[i] > max) {
      aux = i;
      max = a[i];
    }
    else continue;
  }
  max=0;
  for (int i = aux ; i < n; i++) {
    if (a[i]<freq_high && a[i] > max) {
      index = i;
      max = a[i];
    }
    else continue;
  }
  max=0;
  if (index==0){
    for (int i = 0 ; i < n; i++) {
      if (a[i] > max) {
        index = i;
        max = a[i];
      }
      else continue;
    }
  }

  if(failed_channels==1) index = 1;

  return index;
}
// Check for failed channels
int check_failed_channels(TGraph *g) {
  // 0 = not failed
  // 1 = failed low freq
  // 2 = failed low freq + at least 1 point has f=0kHz
  // 3 = failed high freq
  // 4 = failed more than 15 points with f=0kHz
  // 5 = failed all points with f=0KHz
  double freq[32];
  double tth[32];
  int low_freq=0;
  int low_freq2=0;
  int high_freq=0;
  int zero_freq=0;
  int failed=0;

  int chosen_index;

  // filling up array with freq and tth values
  for (int i=0;i<32;i++)
    g->GetPoint(i,tth[i],freq[i]);

  for (int i=0;i<32;i++){
    if (freq[i]<35 && freq[i]>0) low_freq++;
    if (freq[i]<10 && freq[i]>0) low_freq2++;
    else if (freq[i]>500) high_freq++;
    else if (freq[i]==0) zero_freq++;
    else continue;
  }
  //if (low_freq>=31 || high_freq>15 || zero_freq>=15) failed=true; 
  if (low_freq>=30) failed =1;
  if (low_freq2>=25) failed = 2;
  if (high_freq>15) failed = 3;
  if (zero_freq>=15) failed = 4; 
  if (zero_freq==32) failed = 5;
  //cout << low_freq << "\t" << high_freq << "\t" << failed << endl;

  return failed;
}
// Check for failed channels
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
  //cout << low_freq << "\t" << high_freq << "\t" << failed << endl;

  return HasHighFreq;
}


// Find TTH function
int FindLimit2 (TGraph *g){

  double freq[32];
  double tth[32];

  int chosen_index;

  // tth limits to calculate average of tth
  int limit1=0;
  int limit2=0;

  // auxiliar parameters
  double tan[31]; // dy/dx

  // filling up array with freq and tth values
  for (int i=0;i<32;i++)
    g->GetPoint(i,tth[i],freq[i]);

  int failed_channels = check_failed_channels(g);

  // filling up dy and tan
  for (int i=0;i<31;i++){
    tan[i]=abs(freq[i+1]-freq[i]);
    //cout <<  i << "\t " << tth[i] << "\t " << freq[i] << "\t " << tan[i] << endl;
  }

  // set value for limit1
  //if(failed_channels==0 || failed_channels==2) limit1 = tth[find_maximum(freq,31,failed_channels)];
  if(failed_channels==1) limit1 = 1;
  else limit1 = tth[find_maximum(freq,31,failed_channels)];

  // set value for limit2
  // set value for limit2 (check if derivatives change more than 15 btw points i, i+1, i+2 and i+3)
  TGraph *g_test = new TGraph();
  TF1 *p1 = new TF1("p1","pol1",0,31);
  for (int i=limit1+4;i<32-3;i++){
    g_test->SetPoint(0,i,freq[i]);
    g_test->SetPoint(1,i+1,freq[i+1]);
    g_test->SetPoint(2,i+2,freq[i+2]);
    g_test->SetPoint(3,i+3,freq[i+3]);
    g_test->Fit(p1,"QR+");
    if(p1->GetParameter(1) < -2)
      limit2 = i-1;
  }

  // if limit2 still 0, set value for 31
  if (limit2==0 || limit2<=(limit1+4)) limit2=31;

  //cout << limit1 << "\t" << limit2 << endl;
  return limit2;
}
