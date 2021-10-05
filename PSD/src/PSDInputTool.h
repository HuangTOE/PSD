#ifndef PSDINPUTTOOL_H
#define PSDINPUTTOOL_H

#include "SniperKernel/ToolBase.h"
#include <vector>
#include <string>

#include "TVector3.h"

#include "Event/CalibHeader.h"
#include "Event/RecHeader.h"

class PSDInputTool: public ToolBase{
    public:
        PSDInputTool(const std::string &name);
        ~PSDInputTool();

        bool extractHitInfo(JM::CalibEvent*, JM::CDRecEvent*);
        void alignTime(std::string algnmethod = "");

        //get functions
        std::vector<double>& getHitTime(){return v_hitTime;}
        std::vector<double>& getHitCharge(){return v_hitCharge;}

    private:
        //-----------options---------
        bool b_weightOpt;

        //-----------private functions---------
        double calTOF(int);

        //-----------private members---------
        std::vector<TVector3> v_PMTPosi;
        std::vector<double> v_hitTime;
        std::vector<double> v_hitCharge;
        double d_vtxX;double d_vtxY;double d_vtxZ;
};
#endif
