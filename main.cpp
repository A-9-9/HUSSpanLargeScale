#include <iostream>
#include <set>
#include <fstream>
#include <map>

class UtilityChain
{
public:
    int sequenceCount;
//    int *chainSize;
    int *chainIndex;
    //[sequence][tid][tid, acu, ru]
    double ***chain;
    std::map<int, int> SequenceId2IndexMap;
    UtilityChain(){};
    UtilityChain(int sequenceCount, std::map<int, int> SequenceId2IndexMap)
    {
        this->sequenceCount = sequenceCount;
        chainIndex = new int[sequenceCount]{0};
        chain = new double**[sequenceCount];
        this->SequenceId2IndexMap=SequenceId2IndexMap;
    }
};

class UtilityMatrix
{
public:
    double**utilityMatrix;
    double**remainingUtilityMatrix;
    std::map<int, int> item2transactionIdMap;
    int itemSize;
    int transactionSize;
    UtilityMatrix(double **utilityMatrix, double **remainingUtilityMatrix, std::map<int, int> item2transactionIdMap, int transactionSize, int itemSize)
    {
        this->utilityMatrix=utilityMatrix;
        this->remainingUtilityMatrix=remainingUtilityMatrix;
        this->item2transactionIdMap=item2transactionIdMap;
        this->transactionSize = transactionSize;
        this->itemSize = itemSize;
    }
    UtilityMatrix(){};
};
using namespace std;

int main() {
    ifstream readUtility;
    readUtility.open("C:\\Users\\Ken\\CLionProjects\\HUSSpanLargeData\\foodmartData\\foodmart_until1998_utb.txt");
//    readUtility.open("C:\\Users\\Ken\\CLionProjects\\HUSSpanLargeData\\paperData\\originalUtility.txt");
    ifstream readData;
    readData.open("C:\\Users\\Ken\\CLionProjects\\HUSSpanLargeData\\foodmartData\\foodmart_until1998.txt");
//    readData.open("C:\\Users\\Ken\\CLionProjects\\HUSSpanLargeData\\paperData\\originalData.txt");
    ifstream readData2;
    readData2.open("C:\\Users\\Ken\\CLionProjects\\HUSSpanLargeData\\foodmartData\\foodmart_until1998.txt");
//    readData2.open("C:\\Users\\Ken\\CLionProjects\\HUSSpanLargeData\\paperData\\originalData.txt");
    if (!readUtility.is_open() || !readData.is_open() || !readData2.is_open()) {
        cerr << "Failed to read data.\n";
        return 1;
    }

    //Construct external utility
    int itemSize = 1559;
//    int *utility = new int[itemSize]{0};
    double *utility = new double[itemSize]{0};
    int itemInput;
    double utilityInput;
    while (readUtility >> itemInput >> utilityInput)
    {
        utility[itemInput-1] = utilityInput;
    }

    //Construct utility matrix
//    int sequenceSize = 5;
    int sequenceSize = 8842;
//    double ***utilityTable = new double**[sequenceSize];
    UtilityMatrix *utilityMatrix = new UtilityMatrix[sequenceSize];
    set<int> utilityChainCount[itemSize];
    int sid, tid, item;
    double quantity;
    set<int> s;
    int tidSize = 0;
    int sidCount = -1;
    while (readData >> sid >> tid >> item >> quantity)
    {
        if (sidCount == -1)
        {
            tidSize = 0;
            s.clear();
            sidCount = sid;
        } else if (sidCount != sid){
            double **tempUtilityMatrix = new double*[tidSize];
            double **tempRemainingUtilityMatrix = new double*[tidSize];
            for (int i = 0; i < tidSize; i++)
            {
                tempUtilityMatrix[i] = new double[s.size()]{0};
                tempRemainingUtilityMatrix[i] = new double[s.size()]{0};
            }
            map<int, int> itemToIndex;
            int index = 0;
            for (set<int>::iterator iterator=s.begin(); iterator!=s.end(); ++iterator)
            {
                itemToIndex[*iterator] = index++;
            }
            utilityMatrix[sid-2] = UtilityMatrix(tempUtilityMatrix, tempRemainingUtilityMatrix, itemToIndex, tidSize, s.size());
            tidSize = 0;
            s.clear();
            sidCount = sid;
        }
        tidSize = max(tidSize, tid);
        s.insert(item);
        utilityChainCount[item-1].insert(sid-1);
    }

    double **tempUtilityMatrix = new double*[tidSize];
    double **tempRemainingUtilityMatrix = new double*[tidSize];
    for (int i = 0; i < tidSize; i++)
    {
        tempUtilityMatrix[i] = new double[s.size()]{0};
        tempRemainingUtilityMatrix[i] = new double[s.size()]{0};
    }
    map<int, int> itemToIndex;
    int index = 0;
    for (set<int>::iterator iterator=s.begin(); iterator!=s.end(); ++iterator)
    {
        itemToIndex[*iterator] = index++;
    }
    utilityMatrix[sid-1]=UtilityMatrix(tempUtilityMatrix, tempRemainingUtilityMatrix, itemToIndex, tidSize, s.size());

    while (readData2 >> sid >> tid >> item >> quantity)
    {
        utilityMatrix[sid-1].utilityMatrix[tid-1][utilityMatrix[sid-1].item2transactionIdMap[item]] = double(quantity)*utility[item-1];
    }


    //Construct remaining utility matrix
    for(int i = 0; i < sequenceSize; i++)
    {
        double cumulateUtility = 0;
        for (int j = 0; j < utilityMatrix[i].transactionSize; j++)
        {
            for (int k = 0; k < utilityMatrix[i].itemSize; k++)
            {
                cumulateUtility += utilityMatrix[i].utilityMatrix[j][k];
            }
        }
        for (int j = 0; j < utilityMatrix[i].transactionSize; j++)
        {
            for (int k = 0; k < utilityMatrix[i].itemSize; k++)
            {
                cumulateUtility -= utilityMatrix[i].utilityMatrix[j][k];
                utilityMatrix[i].remainingUtilityMatrix[j][k] = cumulateUtility;
            }
        }
    }

    map<int, int> itemToSequence[itemSize];
    index = 0;
    for (int i = 0; i < itemSize; i++)
    {
        for (set<int>::iterator it=utilityChainCount[i].begin(); it != utilityChainCount[i].end(); ++it)
        {
            itemToSequence[i][*it] = index++;
        }
        index = 0;
    }

    //[item]
    UtilityChain *utilityChain = new UtilityChain[itemSize];
    for (int i = 0; i < itemSize; i++)
    {
        utilityChain[i] = UtilityChain(utilityChainCount[i].size(), itemToSequence[i]);
    }

    for(int i = 0; i < sequenceSize; i++)
    {
        for (map<int, int>::iterator it=utilityMatrix[i].item2transactionIdMap.begin(); it != utilityMatrix[i].item2transactionIdMap.end(); ++it)
        {
            //it->first-1: chain, it->second: matrix
            int tempUtilityChainSize = 0;
            for (int k = 0; k < utilityMatrix[i].transactionSize; k++) {
                if (utilityMatrix[i].utilityMatrix[k][it->second] != 0) {
                    tempUtilityChainSize++;
                }
            }
            index = utilityChain[it->first-1].SequenceId2IndexMap[i];
            utilityChain[it->first-1].chain[index] = new double *[tempUtilityChainSize];
            int index2 = 0;
            for (int k = 0; k < utilityMatrix[i].transactionSize; k++)
            {
                if (utilityMatrix[i].utilityMatrix[k][it->second] != 0)
                {
                    utilityChain[it->first-1].chain[index][index2] = new double[3]{0};
                    utilityChain[it->first-1].chain[index][index2][0] = k;
                    utilityChain[it->first-1].chain[index][index2][1] = utilityMatrix[i].utilityMatrix[k][it->second];
                    utilityChain[it->first-1].chain[index][index2][2] = utilityMatrix[i].remainingUtilityMatrix[k][it->second];
                    index2++;
                }
            }
            utilityChain[it->first-1].chainIndex[index] = index2;
        }
    }

    for (int k = 0; k < utilityChain[1557].chainIndex[2]; k++)
    {
        cout << utilityChain[1557].chain[2][k][0] << ", " << utilityChain[1557].chain[2][k][1];
    }


    return 0;
}