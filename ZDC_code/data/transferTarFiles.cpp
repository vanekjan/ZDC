#include <iostream>
#include <string>
using namespace std;

int transferTarFiles()
{
	bool Debug = false;
	bool Output_List_File = true;
	bool Execute_Command  = true;
	bool Delete_List_File = true;

	int RunNumber	= 20048001;
// ***********************************************************************
	TString Energy  = "lkramarik";
	TString trgSetup= "ZdcCalibration";

//	TString Energy	= "pp500";
//	TString Energy	= "AuAu15";
//	TString trgSetup= "pp500_production_2013";
//	TString trgSetup= "production_15GeV_2014";
//	TString trgSetup= "pedestal_rhicclock";
//	TString trgSetup= "pedAsPhys";
//	TString trgSetup= "ZdcPolarimetry";

	TString WorkDir = "/gpfs/mnt/gpfs01/star/pwg/lkramarik/ZDC/ZDC_code/data";
	TString OutputFileName = Form("%s/file.list",WorkDir.Data());
// ***********************************************************************
	cout<<"================================================"<<endl;
// ***********************************************************************
	int Year	= (int(RunNumber/1000000))-1+2000;
	int RunYear = (int(RunNumber/1000000))-1;
	int RunDay	=  int((int(RunNumber%1000000))/1000);
	TString RunDayTStr=0;
	if(RunDay<100)	RunDayTStr = Form("0%d",RunDay);
	else			RunDayTStr = Form("%d",RunDay);
	TString FileFrom = 0;
	TString FileTo	 = 0;
	TString CMD = 0;

	CMD = Form("rm -rf *.list");gSystem->Exec(CMD.Data());

	if(Output_List_File){ofstream ofile;ofile.open(OutputFileName.Data(),ios::out);if(!ofile){cerr<<"ofile.open failed"<<endl;return 0;}}
	CMD = Form("mkdir -p %s/run%d.%s.%s/%d/",WorkDir.Data(),RunYear,trgSetup.Data(),Energy.Data(),RunNumber);gSystem->Exec(CMD.Data());

	cout<<" Generating list file"<<endl;
	FileFrom = Form("/home/starsink/raw/daq/%d/%s/%d/run%d.tgz",Year,RunDayTStr.Data(),RunNumber,RunNumber);
	FileTo	 = Form("%s/run%d.%s.%s/%d/run%d.tgz",WorkDir.Data(),RunYear,trgSetup.Data(),Energy.Data(),RunNumber,RunNumber);

	if(Output_List_File)
	{
	  ofile<<FileFrom<<" "<<FileTo<<endl;
	}
	cout<<"================================================"<<endl;
	if(Output_List_File){ofile.close();}

	cout<<" Executing hpss_user.pl -f"<<OutputFileName<<endl;
	cout<<"================================================"<<endl;
	if(Execute_Command){CMD = Form("hpss_user.pl -f %s",OutputFileName.Data());gSystem->Exec(CMD.Data());}
	cout<<"================================================"<<endl;
	if(Delete_List_File){CMD = Form("rm -rf %s",OutputFileName.Data());gSystem->Exec(CMD.Data());}

	CMD = Form("rm -f *.list");gSystem->Exec(CMD.Data());
	CMD = Form("rm -f ls.csh");gSystem->Exec(CMD.Data());
	ofstream ofileCSH;
	ofileCSH.open("./ls.csh",ios::out);
	ofileCSH<<"#!/bin/csh"<<endl;
	ofileCSH<<endl;
	ofileCSH<<"ls "<<WorkDir<<"/run"<<RunYear<<"."<<trgSetup<<"."<<Energy<<"/"<<RunNumber<<"/*.dat > ";
	ofileCSH<<"run"<<RunYear<<"."<<trgSetup<<"."<<Energy<<".list"<<endl;
	ofileCSH.close();
	CMD = Form("chmod 755 ls.csh");gSystem->Exec(CMD.Data());

	return 0;
}
