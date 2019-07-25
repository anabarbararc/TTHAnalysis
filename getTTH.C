#include "header.h"

int getTTH(string board){

  // new path to save directly in config directory
  //string path = "/srv/nfs/rootfs/root/fpga_app/config/Board" + board;
  string path = "Data_TTH/daq_board_" + board;

  // root file with the plt freq vs tth for all channels and stic
  string rootfilename = path + "/freq_vs_tth.root";
  TFile *f = new TFile(rootfilename.c_str(),"recreate");

  // loop in the stics
  for (int stic=0; stic<8; stic++){
    cout << "Getting TTH of channels on STiC" << stic << endl;

    string infilename = path + "/tth_scan_v1/stic_" + to_string(stic) + ".txt";

    // cmd to apply a space btw = sign and numbers in the sticx.txt files
    // this makes them readable
    string cmd = "sed -i -e 's/=/= /g' " + infilename;
    int systemcmd1 = system(cmd.c_str());

    // set input file
    ifstream file(infilename.c_str(), ios::in );

    // set path
    string create_tthdir = "mkdir -p " + path + "/tth";
    string create_faileddir = "mkdir -p " + path + "/tth_failed";
    
    // set output file in tth directory
    int systemcmd2 = system(create_tthdir.c_str());
    string outfilename = path + "/tth/stic_" + to_string(stic) + ".txt";
    ofstream outfile(outfilename.c_str(), ios::out );
    
    // set log files
    int systemcmd3 = system(create_faileddir.c_str());
    string logfilename = path + "/tth_failed/stic_" + to_string(stic) + ".log";
    ofstream logfile(logfilename.c_str(), ios::out );

    // if any of the system cmds fail, error message appears
    if(systemcmd1 == -1){
      cout << "cmd: " << cmd << " FAILED." << endl; 
    }
    if(systemcmd2 == -1){
      cout << "cmd: " << create_tthdir << " FAILED." << endl; 
    }
    if(systemcmd3 == -1){
      cout << "cmd: " << create_faileddir << " FAILED." << endl; 

    }
    // auxiliar parameters
    char aux1[4];
    char aux2[5];
    char aux3[6];
    int ch, tth;
    double freq;
    int failed_channels=0;

    double data[64][32]={0};
    int tthvalue;
    int limit1 = 0;
    int limit2 = 0;
    vector <int>lim1;
    vector <int>lim2;

    // check if file exists
    if( !file )
      cerr << "Cant open file! " << endl;
    while(file >> aux1  >> ch >> aux2 >> tth >> aux2  >> freq){
      data[ch][tth] = freq;
    }

    // naming directory for each stic
    string sticnumber = "stic" + to_string(stic);
    f->cd();
    gDirectory->mkdir(sticnumber.c_str());
    f->cd(sticnumber.c_str());

    // defining graphs for each channel
    TGraph *g = new TGraph[64];
    TGraph *g_copy = new TGraph();

    // vector with the chosen tth value for each channel
    vector<int> chosen_tth;
    double tth_array[32]={0};
    double freq_array[32]={0};

    // loop in the channels
    for (int i=0; i<64; i++){

      failed_channels=0;

      for (int j=0; j<32; j++){
        g[i].SetPoint(j,j,data[i][j]);
        tth_array[j]=j;
        freq_array[j]=data[i][j];
      }
      g[i].Draw("ALP");
      g_copy=&g[i];

      //lim2.push_back(FindLimit2(g_copy));

      outfile << "ch= " << i << "\t tth= " << tthvalue << endl;

      // check for failed channels
      failed_channels = check_failed_channels(freq_array);

      if(failed_channels==1 || failed_channels==2){
          limit1 = FindLimit1_low(freq_array);
          limit2 = FindLimit2_low(freq_array);
      }
      else{
          limit1 = FindLimit1(freq_array,tth_array,31,failed_channels);
          //limit1 = FindLimit1(freq_array);
          limit2 = FindLimit2(freq_array,limit1);
      }
      lim1.push_back(limit1);
      lim2.push_back(limit2);

      // get tth values
      tthvalue = FindTTH(tth_array,limit1,limit2);
      chosen_tth.push_back(tthvalue);

      //if (limit1==0){
      //  logfile << "ch= " << i << "\t Limit left edge = 0! Check channel!"<< endl;
      //}
      //if (failed_channels==1){
      //  logfile << "ch= " << i << "\t Frequencies lower than 80 kHz"<< endl;
      //}
      //if (failed_channels==2){
      //  logfile << "ch= " << i << "\t Frequencies lower than 10 kHz"<< endl;
      //}
      //if (failed_channels==3){
      //  logfile << "ch= " << i << "\t Frequencies higher than 500 kHz"<< endl;
      //}
      //if (failed_channels==4){
      //  logfile << "ch= " << i << "\t > 15 points with f=0kHz kHz"<< endl;
      //}
      if (failed_channels==5){
        logfile << "ch= " << i << "\t All points have f=0kHz"<< endl;
      }
      else if (tthvalue<=1){
        logfile << "ch= " << i << "\t TTH <= 1"<< endl;
      }
      else if (freq_array[tthvalue]>500 || freq_array[tthvalue]==0){
        logfile << "ch= " << i << "\t Freq(chosen_TTH) is larger than 500 kHz or 0 kHz"<< endl;
      }
      //if (lim2.at(i)<=lim1.at(i)+4){
      //  logfile << "ch= " << i << "\t Fault of method: check channel!"<< endl;
      //}
      else if (tthvalue != int((lim1.at(i)+lim2.at(i))/2)){
        logfile << "ch= " << i << "\t Wrong tth! "<< lim1.at(i) << " " << lim2.at(i) << " " << tthvalue << endl;
      }
      //else if( (freq_array[lim1.at(i)] > 1000 && freq_array[lim2.at(i)] < 200) || (abs(freq_array[lim1.at(i)]-freq_array[lim2.at(i)]) > 800)){
      //  logfile << "ch= " << i << "\t Check channel!"<< endl;
      //}



    }
    outfile.close();

    // canvas for every channel
    TCanvas* c[64] ;
    std::string graphicName;
    string print_tth;

    for (int i=0; i<64; i++){
      c[i] = new TCanvas() ;
      graphicName = "chip "+to_string(stic)+"  channel "+to_string(i);
      c[i]->SetTitle(graphicName.c_str());
      c[i]->SetName(graphicName.c_str());
      g[i].SetLineWidth(2);
      g[i].GetYaxis()->SetTitle("Frequency (kHz)");
      g[i].GetXaxis()->SetTitle("TTH");
      g[i].SetMarkerStyle(4);
      g[i].Draw("ACP");
      // set range for drawing line
      Float_t ymin = g[i].GetHistogram()->GetMinimum();
      Float_t ymax = g[i].GetHistogram()->GetMaximum();
      TLine *line = new TLine(chosen_tth.at(i),ymin,chosen_tth.at(i),ymax);
      TLine *line1 = new TLine(lim1.at(i),ymin,lim1.at(i),ymax);
      TLine *line2 = new TLine(lim2.at(i),ymin,lim2.at(i),ymax);
      line->SetLineColor(kRed);
      line->SetLineWidth(4);
      line1->SetLineColor(kBlue);
      line1->SetLineWidth(4);
      line2->SetLineColor(kBlue);
      line2->SetLineWidth(4);
      line->Draw("same");
      line1->Draw("same");
      line2->Draw("same");
      // print tth value
      print_tth = "TTH = " + to_string(chosen_tth.at(i));
      TLatex *text = new TLatex(0.7,0.85,print_tth.c_str());
      text->SetNDC();
      text->SetTextSize(0.05);
      text->Draw();
      // save canvas
      c[i]->Modified();
      c[i]->SetLogy() ;
      c[i]->Modified();
      c[i]->Write();
    }
    file.close();
  }

  return 0;

}
