#include <iostream>
#include <fstream>
#include <string>
#include <TGraph.h>
using namespace std;

int FindTTH (TGraph *g);
int FindTTH2 (TGraph *g);
int FindTTH3 (TGraph *g);
int FindTTH4 (TGraph *g);
int FindTTH5 (TGraph *g);
TGraph *Interpolate (TGraph *g, int n);
double largest(double arr[], int n);

int get_TTH_mod()
{
  TFile *f = new TFile("histograms.root","recreate");
  //system("sed -i 's/=/= /g' ../Data_TTH_telescope/daq_board_25/tth_scan/stic_0.txt");

  //string filename = "../Data_TTH_telescope/daq_board_25/tth_scan/stic_" + to_string(i) + ".txt"
  ifstream file( "../Data_TTH_telescope/daq_board_25/tth_scan/stic_0.txt", ios::in );

  char aux1[4];
  char aux2[5];
  char aux3[6];
  int ch, tth;
  double freq;

  double data[64][32]={0};

  if( !file )
	cerr << "Cant open file! " << endl;
  while(file >> aux1  >> ch >> aux2 >> tth >> aux2  >> freq){
	cout << ch << " " << tth << " " << freq << endl;
	data[ch][tth] = freq;	
  }

  f->cd();
  gDirectory->mkdir("stic0");
  f->cd("stic0");

  TGraph *g = new TGraph[64];
  TGraph *g_copy = new TGraph();

  int value;
  vector<int> chosen_tth;

  //cout << "OI \n" <<endl;
  for (int i=0; i<64; i++){
	cout << "==> channel = " << i << endl;
	for (int j=0; j<32; j++){
	  g[i].SetPoint(j,j,data[i][j]);
	  //cout << "tth = " << j << "  freq = " << data[i][j] << endl;
	}
	g[i].Draw("ALP");
    //g_copy=&g[i];
	value = FindTTH5(&g[i]);
	chosen_tth.push_back(value);
	//cout << "value = " << value << endl;
  }	

  TCanvas* c[64] ;
  std::string graphicName;
  string print_value;
  for (int i=0; i<64; i++){
	c[i] = new TCanvas() ;
	graphicName = "chip"+to_string(0)+"channel"+to_string(i);
	c[i]->SetName(graphicName.c_str());
 	g[i].SetLineWidth(2);
 	g[i].GetYaxis()->SetTitle("Frequency (kHz)");
 	g[i].GetXaxis()->SetTitle("TTH");
	g[i].SetMarkerStyle(4);
	g[i].Draw("ACP");
    Float_t ymin = g[i].GetHistogram()->GetMinimum();
    Float_t ymax = g[i].GetHistogram()->GetMaximum();
	TLine *line = new TLine(chosen_tth.at(i),ymin,chosen_tth.at(i),ymax);
	line->SetLineColor(kRed);
	line->SetLineWidth(4);
    line->Draw("same");
	print_value = "TTH = " + to_string(chosen_tth.at(i));
	TLatex *text = new TLatex(0.7,0.85,print_value.c_str());
	text->SetNDC();
	text->SetTextSize(0.05);
    text->Draw();
	c[i]->Modified();
    c[i]->SetLogy() ;
	c[i]->Modified();
	c[i]->Write();
  }

  file.close();
  return 0;

}

int FindTTH2 (TGraph *g){
  double freq[32];
  double tth[32];
  double average=0;
  int n_tth=0;
  for (int i=0;i<32;i++){
	g->GetPoint(i,tth[i],freq[i]);
	if(freq[i]>20 && freq[i]<110) {
		n_tth++;
		average +=tth[i];  
	}
  }
  int chosen_index;
  if (average/n_tth > 80) chosen_index = average/n_tth - 5;
  if (average/n_tth < 50) chosen_index = average/n_tth + 2;
  return tth[chosen_index];
}

int FindTTH3 (TGraph *g){
  double freq[32];
  double tth[32];
  double average=0;
  double sum=0;
  double average2=0;
  int n_tth=0;
  int n_tth2=0;
  for (int i=0;i<32;i++){
	g->GetPoint(i,tth[i],freq[i]);
	if(freq[i]>10 && freq[i]<120) {
	  n_tth++;
	  sum +=tth[i];
	}
  }
  average = sum/n_tth;
  sum = 0;
  n_tth = 0;
  for (int i=0;i<32;i++){
	if(freq[i]>10 && freq[i]<120) {
	  if ((freq[i]-average)>45){
		n_tth++;
		sum += tth[i];
	  }
	}
  }
  average = sum/n_tth;
  int chosen_index = (int)average;
  return tth[chosen_index];
}
int FindTTH4 (TGraph *g){
  double freq[32];
  double new_freq[32*5];
  double tth[32];
  double average=0;
  double sum=0;
  double average2=0;
  int n_tth=0;
  int n_tth2=0;

  for (int i=0;i<32*3;i++){
    g->GetPoint(i,tth[i],freq[i]);
    if(freq[i]>10 && freq[i]<120) {
      n_tth++;
      sum +=tth[i];
    }
  }
  average = sum/n_tth;
  sum = 0;
  n_tth = 0;
  for (int i=0;i<32*4;i++){
	double x = double(i);
    new_freq[i] = g->Eval(x/4, 0, ""); 
    if(freq[i]>10 && freq[i]<120) {
      n_tth++;
      sum +=tth[i];
    }
  }
  average = sum/n_tth/4;
  int chosen_index = (int)average;
  return tth[chosen_index];
}
double largest(double arr[], int n) 
{ 
    int i; 
     
    // Initialize maximum element 
    double max = arr[0]; 
  
    // Traverse array elements from second and 
    // compare every element with current max   
    for (i = 1; i < n; i++) 
        if (arr[i] > max) 
            max = arr[i]; 
  
    return max; 
}

int FindTTH5 (TGraph *g){
  double freq[32];
  double tth[32];
  int chosen_index;
  int limit1=0;
  int limit2=0;

  double dx[31];
  double dy[31];
  double ddy[30];
  double tan[31];
  double tan2[30];
  
  for (int i=0;i<32;i++)
	g->GetPoint(i,tth[i],freq[i]);
  for (int i=0;i<31;i++){
	  dy[i]=abs(freq[i+1]-freq[i]);
	  dx[i]=tth[i+1]-tth[i];
      tan[i]=dy[i]/dx[i];
	  cout <<  i << "\t " << tth[i] << "\t " << freq[i] << "\t " << tan[i] << endl;
  }
  for (int i=0;i<30;i++){
      ddy[i]=abs(dy[i+1]-dy[i]);
      tan2[i]=ddy[i]/dx[i];
  }
  
  cout << largest(tan,29) << " AQUI"  << endl;
  for (int i=0;i<30-1;i++){
      if( tan[i]==largest(tan,29) ) limit1 = i;
      if(i > limit1+4 && i < 28) {
        if((tan[i]-tan[i+1])>15) limit2 = i;
      }
      cout <<  i << "\t "<< tth[i] << "\t" << dx[i] << "\t " << dy[i] << "\t" << limit1 << "\t " << limit2<< endl;
      if (limit2!=0) break;
  }
  TCanvas *c1 = new TCanvas(); 
  TGraph *gdev1 = new TGraph(31,tth,tan);
  gdev1->SetMarkerStyle(21);
  gdev1->SetMarkerSize(0.6);
  c1->SetLogy();
  gdev1->Draw("APC");

  TCanvas *c2 = new TCanvas(); 
  TGraph *gdev2 = new TGraph(30,tth,tan2);
  gdev2->SetMarkerStyle(21);
  gdev2->SetMarkerSize(0.6);
  c2->SetLogy();
  gdev2->Draw("APC");

  int count=0;
  int sum_tth=0;
  for (int i=0;i<30;i++){
    if (freq[i]>10 && freq[1]<110){
      if (tan[i] <10 && i>limit1 && i<limit2) {sum_tth+=tth[i]; count++;}
    }
    else continue;
  }
  if(sum_tth>0) chosen_index = sum_tth / count -1;
  else chosen_index=0;
  //cout << "media = " << average << "  sqdiff = " << sqdiff << endl;

  return tth[chosen_index];

}

TGraph *Interpolate (TGraph *g, int n){
  double freq[n];
  double tth[n];

  for (int i=0;i<n;i++)
	g->GetPoint(i,tth[i],freq[i]);

  double x[4*n];
  double y[4*n];
  for(int i=0;i<n;i++){
	x[4*i]=tth[i];
	x[4*i+2]=(tth[i]+tth[i+1])/2;
	x[4*i+1]=(tth[i]+(tth[i]+tth[i+1])/2)/2;
	x[4*i+3]=(tth[i+1]+(tth[i]+tth[i+1])/2)/2;

	y[4*i]=freq[i];
	y[4*i+2]=(freq[i]+freq[i+1])/2;
	y[4*i+1]=(freq[i]+(freq[i]+freq[i+1])/2)/2;
	y[4*i+3]=(freq[i+1]+(freq[i]+freq[i+1])/2)/2;
  }

  TGraph *g1 = new TGraph(4*n,x,y); 
  return g1;
}

int FindTTH (TGraph *g){
  double freq[32];
  double tth[32];
  for (int i=0;i<32;i++)
	g->GetPoint(i,tth[i],freq[i]);

  double average;
  double sqdiff;
  double min;
  int chosen_index;
  double par[9];
  double diff[4]={0};
  double diff_max=0;

  for (int i=0;i<32-8;i++){
	average=0;
	sqdiff=0;
	par[0]=freq[i+0];
	par[1]=freq[i+1];
	par[2]=freq[i+2];
	par[3]=freq[i+3];
	par[4]=freq[i+4];
	par[5]=freq[i+5];
	par[6]=freq[i+6];
	par[7]=freq[i+7];
	par[8]=freq[i+8];

	for (int j=0;j<9;j++)
	  average += par[j]/9;
	for (int j=0;j<9;j++)
	  sqdiff += pow((par[j]-average),4);

	double slope1=0;
	double slope2=0;
	if(i>=4){
	  TGraph *g = new TGraph();
	  for(int k=0;k<4;k++){
		g->SetPoint(k,i-(k+4),freq[i-(k+4)]);
	  }
	  g->Fit("pol1");
	  slope1 = g->GetFunction("pol1")->GetParameter(1);
	  cout << "================== " << slope1 << endl;
	}  
	if(i<=28){
	  TGraph *g = new TGraph();
	  for(int k=0;k<4;k++){
		g->SetPoint(k,i+k,freq[i+k]);
	  }
	  g->Fit("pol1");
	  slope2 = g->GetFunction("pol1")->GetParameter(1);
	}
	diff[0] = abs(par[1]-par[0]);
	diff[1] = abs(par[2]-par[1]);
	diff[2] = abs(par[8]-par[7]);
	diff[3] = abs(par[6]-par[7]);
	//if (diff[0]  > diff_max) diff_max = diff[0];	
	//if (diff[1]  > diff_max) diff_max = diff[1];	
	//if (diff[2]  > diff_max) diff_max = diff[2];	
	//if (diff[3]  > diff_max) diff_max = diff[3];	

	if (i==0) {min = sqdiff; chosen_index = i;}
	//else if (g->GetHistogram()->GetEntries()==0) {continue;
	//else if (min > sqdiff && freq[i]>10 && freq[i]<110 && diff[0]<3 && diff[1]<5 && diff[2]<3 && diff[3]<5) {min = sqdiff; chosen_index = i;}
	else if (min > sqdiff && freq[i]>10 && freq[i]<110 && diff[0]<3 && diff[1]<5 && diff[2]<3 && diff[3]<5) {min = sqdiff; chosen_index = i;}
	else continue;

	//cout << "media = " << average << "  sqdiff = " << sqdiff << endl;

  }
  return tth[chosen_index];

  }

  //double calc_average(double par){
  //  double average=0;
  //  for(int i=0;i<5;i++)
  //	average+=par[i]/5;
  //  return average;
  //}
  //
  //double calc_sqdiff(double par, double average){
  //  double sqdiff;
  //  for(int i=0;i<5;i++)
  //  return sqdiff;
  //}
