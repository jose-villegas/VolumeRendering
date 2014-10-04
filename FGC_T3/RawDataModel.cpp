#include "RawDataModel.h"


RawDataModel::RawDataModel(void)
{
    sModelName = "";
    width = height = numCuts = 0;
}


RawDataModel::~RawDataModel(void)
{
}

void RawDataModel::Load(std::string sFilePath, int width, int height, int numCuts)
{
}
