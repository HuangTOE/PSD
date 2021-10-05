#include "PSDInputTool.h"

#include "Identifier/Identifier.h"
#include "Identifier/CdID.h"

#include "SniperKernel/ToolFactory.h"

#include "TH1.h"
#include "TFile.h"
#include "TTree.h"
#include "TMath.h"

DECLARE_TOOL(PSDInputTool);

PSDInputTool::PSDInputTool(const std::string &name):ToolBase(name){
    b_weightOpt = false;
    v_PMTPosi.clear();
    v_hitTime.clear();
    v_hitCharge.clear();
    d_vtxX = 0;
    d_vtxY = 0;
    d_vtxZ = 0;
}

PSDInputTool::~PSDInputTool(){
}

bool PSDInputTool::extractHitInfo(JM::CalibEvent *calibevent, JM::CDRecEvent *recevent){
    v_hitTime.clear();
    v_hitTime.reserve(10000);
    v_hitCharge.clear();
    v_hitCharge.reserve(10000);
    d_vtxX = 0;d_vtxY = 0;d_vtxZ = 0;

    //read vertex from RecEvent
    d_vtxX = recevent->x();
    d_vtxY = recevent->y();
    d_vtxZ = recevent->z();

    //read time and charge from CalibEvent
    const std::list<JM::CalibPMTChannel*>& l_pmtcol = calibevent->calibPMTCol();
    for (std::list<JM::CalibPMTChannel*>::const_iterator it = l_pmtcol.begin();it != l_pmtcol.end();++it){
        unsigned int pmtid = (*it)->pmtId();
        Identifier id = Identifier(pmtid);
        if (not CdID::is20inch(id)) {
            continue;
        }

        int hitno = (*it)->size();
        const std::vector<double>& hittime = (*it)->time();
        const std::vector<double>& charge = (*it)->charge();
        for (int i = 0;i<hitno;i++){
            //v_pmtID.push_back(CdID::module(id));
            v_hitTime.push_back(hittime.at(i)-calTOF(CdID::module(id)));
            v_hitCharge.push_back(charge.at(i));
        }
    }
    return true;
}

void PSDInputTool::alignTime(std::string algnmethod){
    //-------shift the time by t0------------
    if (!algnmethod.compare("noShift")) return;
    double t0=0;
    if (!algnmethod.compare("alignPeak")){
        TH1D *myhist=new TH1D("myhist","title",550,-100,1000);
        int isize=v_hitTime.size();
        for (int i=0;i<isize;i++){
            double dweight=1;
            if (b_weightOpt) dweight=v_hitCharge.at(i);
            myhist->Fill(v_hitTime.at(i),dweight);
        }
        int binno=myhist->GetMaximumBin();
        t0=myhist->GetBinCenter(binno)-100;//the peak aligned to 100
        delete myhist;
    }
    else if (!algnmethod.compare("alignMean")){
        double sum=0;
        double totcharge=0;
        int isize=v_hitTime.size();
        for (int i=0;i<isize;i++){
            double dweight=1;
            if (b_weightOpt) dweight=v_hitCharge.at(i);
            totcharge+=dweight;
            sum+=v_hitTime.at(i)*dweight;
        }
        sum/=totcharge;
        t0=sum-100;
    }
    for (std::vector<double>::iterator it=v_hitTime.begin();it!=v_hitTime.end();++it){
        *it-=t0;
    }
}

double PSDInputTool::calTOF(int id){
    if (v_PMTPosi.size() == 0){
        //read the position info from the root file
        v_PMTPosi.reserve(18000);
        TFile *thisfile = TFile::Open("/cvmfs/juno.ihep.ac.cn/centos7_amd64_gcc830/Pre-Release/J20v2r0-Pre1/data/Simulation/ElecSim/PmtData_Lpmt.root");
        TTree *pmtdata=dynamic_cast<TTree*>(thisfile->Get("PmtData_Lpmt"));
        double pmtPosX=0;
        double pmtPosY=0;
        double pmtPosZ=0;
        pmtdata->SetBranchAddress("pmtPosX", &pmtPosX);
        pmtdata->SetBranchAddress("pmtPosY", &pmtPosY);
        pmtdata->SetBranchAddress("pmtPosZ", &pmtPosZ);
        int totEntries=pmtdata->GetEntries();
        for (int ith=0;ith<totEntries;ith++){
            pmtdata->GetEntry(ith);
            TVector3 tmpv(pmtPosX, pmtPosY, pmtPosZ);
            v_PMTPosi.push_back(tmpv);
        }
    }

    //calculate the time of flight
	double PMT_R = 19.434;
	double Ball_R = 19.18;
    TVector3 pmtposi=Ball_R/PMT_R*v_PMTPosi.at(id);
    //std::cout<<id<<":("<<pmtposi.X()<<", "<<pmtposi.Y()<<", "<<pmtposi.Z()<<")"<<std::endl;
    double dx=(d_vtxX-pmtposi.X())/1000.;
    double dy=(d_vtxY-pmtposi.Y())/1000.;
    double dz=(d_vtxZ-pmtposi.Z())/1000.;
	double dist = TMath::Sqrt(dx*dx+dy*dy+dz*dz);
    double m_neff=1.54;
	double lightspeed = 299792458/1e9;
    double tof=dist*m_neff/lightspeed;
    //std::cout<<"TOF:"<<tof<<std::endl;
	return tof;
}
