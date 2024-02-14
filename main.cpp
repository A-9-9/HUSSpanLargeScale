#include <iostream>
#include <set>
#include <fstream>
#include <map>
#include <string>
using namespace std;
class UtilityChain
{
public:
    int sequenceCount;
    int *chainIndex;
    //[sequence][tid][tid, acu, ru]
    double ***chain;
    std::map<int, int> SequenceId2IndexMap;
    double utility;
    double PEU;
    double *PEUList;
    string pattern;
    UtilityChain(){};
    UtilityChain(int sequenceCount, std::map<int, int> SequenceId2IndexMap, int pattern)
    {
        this->sequenceCount = sequenceCount;
        chainIndex = new int[sequenceCount]{0};
        chain = new double**[sequenceCount];
        this->SequenceId2IndexMap=SequenceId2IndexMap;
        utility=0;
        PEU=0;
        PEUList = new double[sequenceCount];
        this->pattern= to_string(pattern);
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

UtilityChain I(UtilityChain utilityChain, int extensionItem, UtilityMatrix *utilityMatrix){
//    UtilityChain(int sequenceCount, std::map<int, int> SequenceId2IndexMap, int pattern);
//    int sequenceCount;
//    int *chainIndex;
//    //[sequence][tid][tid, acu, ru]
//    double ***chain;
//    std::map<int, int> SequenceId2IndexMap;
//    for (map<int, int>::iterator it=utilityMatrix[2].item2transactionIdMap.begin(); it!=utilityMatrix[2].item2transactionIdMap.end(); ++it)
//    {
//        cout << it->first << ": " << it->second << endl;
//    }
//    for (int i = 0; i < utilityChain.sequenceCount; i++)
//    {
//        for (int j = 0; j < utilityChain.chainIndex[i]; j++)
//        {
//            cout << utilityChain.chain[i][j][0] << ", ";
//        }
//        cout << endl;
//    }
    int seqSize = 2;
    int sequenceCount = 0;
    int *chainIndex = new int[seqSize];
    double ***chain = new double**[seqSize];
    map<int, int> chainItemToIndex;
    for (map<int, int>::iterator it=utilityChain.SequenceId2IndexMap.begin(); it != utilityChain.SequenceId2IndexMap.end(); ++it)
    {
        //it->second index
        //it->first-1 item
        int size = 0;
        for (int j = 0; j < utilityChain.chainIndex[it->second]; j++)
        {
            int tidIndexOnUM = utilityChain.chain[it->second][j][0];
            if (utilityMatrix[it->first].item2transactionIdMap.count(extensionItem+1))
            {
                int itemIndexOnUM = utilityMatrix[it->first].item2transactionIdMap[extensionItem+1];
                int tt = utilityMatrix[it->first].utilityMatrix[tidIndexOnUM][itemIndexOnUM];
                if (utilityMatrix[it->first].utilityMatrix[tidIndexOnUM][itemIndexOnUM] != 0)
                {
                    size++;
                }
            }
        }
        if (size != 0)
        {
            double **temp = new double*[size];
            int index = 0;
            for (int j = 0; j < utilityChain.chainIndex[it->second]; j++)
            {
                int tidIndexOnUM = utilityChain.chain[it->second][j][0];
                int itemIndexOnUM = utilityMatrix[it->first].item2transactionIdMap[extensionItem+1];
                if (utilityMatrix[it->first].utilityMatrix[tidIndexOnUM][itemIndexOnUM] != 0)
                {
                    temp[index] = new double[3];
                    temp[index][0] = utilityChain.chain[it->second][j][0];
                    temp[index][1] = utilityChain.chain[it->second][j][1]+utilityMatrix[it->first].utilityMatrix[tidIndexOnUM][itemIndexOnUM];
                    temp[index][2] = utilityMatrix[it->first].remainingUtilityMatrix[tidIndexOnUM][itemIndexOnUM];
//                    cout << temp[index][0] << ", " << temp[index][1] << ", " << temp[index][2] << endl;
                    index++;
                }
            }

            if (sequenceCount >= seqSize)
            {
                int newSize = seqSize*2;
                double ***tempPtr = new double**[newSize];
                int *tempChainIndex = new int[newSize];
                for (int j = 0; j < seqSize; j++)
                {
                    tempPtr[j] = chain[j];
                    tempChainIndex[j] = chainIndex[j];
                }
                delete [] chain;
                delete [] chainIndex;
                chain = tempPtr;
                chainIndex = tempChainIndex;
                seqSize = newSize;
            }
            chainIndex[sequenceCount] = index;
            chainItemToIndex[it->first] = sequenceCount;
            chain[sequenceCount++] = temp;
        }
    }

    for (int i = 0; i < sequenceCount; i++)
    {
        for (int j = 0; j < chainIndex[i]; j++)
        {
            cout << chain[i][j][0] << ", " << chain[i][j][1] << ", " << chain[i][j][2] << endl;
        }
        cout << endl;
    }

    for (map<int, int>::iterator it=chainItemToIndex.begin(); it!=chainItemToIndex.end(); ++it)
    {
        cout << it->first << ": " << it->second << endl;
    }
//    cout << endl << sequenceCount;
//    cout << endl << chainIndex[0];
    UtilityChain a = UtilityChain();
    a.sequenceCount=sequenceCount;
    a.chainIndex=chainIndex;
    a.chain=chain;
    a.SequenceId2IndexMap=chainItemToIndex;
    double tempUtility = 0;
    double *tempPEUList = new double[a.sequenceCount];
    double totalPEU = 0;
    int index = 0;
    for (int j = 0; j < a.sequenceCount; j++)
    {
        double temp = 0;
        double tempPEU = 0;
        for (int k = 0; k < a.chainIndex[j]; k++)
        {
            temp = max(temp, a.chain[j][k][1]);
            if (a.chain[j][k][2]>0)
            {
                tempPEU = max(tempPEU, a.chain[j][k][1] + a.chain[j][k][2]);
            }
        }
        tempUtility += temp;
        tempPEUList[index++] = tempPEU;
        totalPEU += tempPEU;
    }
    a.utility = tempUtility;
    a.PEUList = tempPEUList;
    a.PEU = totalPEU;
    a.pattern = utilityChain.pattern+ to_string(extensionItem+1);
    return a;
};

int main() {
    ifstream readUtility;
//    readUtility.open("C:\\Users\\Ken\\CLionProjects\\HUSSpanLargeData\\foodmartData\\foodmart_until1998_utb.txt");
    readUtility.open("C:\\Users\\Ken\\CLionProjects\\HUSSpanLargeData\\paperData\\originalUtility.txt");
    ifstream readData;
//    readData.open("C:\\Users\\Ken\\CLionProjects\\HUSSpanLargeData\\foodmartData\\foodmart_until1998.txt");
    readData.open("C:\\Users\\Ken\\CLionProjects\\HUSSpanLargeData\\paperData\\originalData.txt");
    ifstream readData2;
//    readData2.open("C:\\Users\\Ken\\CLionProjects\\HUSSpanLargeData\\foodmartData\\foodmart_until1998.txt");
    readData2.open("C:\\Users\\Ken\\CLionProjects\\HUSSpanLargeData\\paperData\\originalData.txt");
    if (!readUtility.is_open() || !readData.is_open() || !readData2.is_open()) {
        cerr << "Failed to read data.\n";
        return 1;
    }

    //Construct external utility
    int itemSize = 5;
    int *utility = new int[itemSize]{0};
//    double *utility = new double[1559]{0};
    int itemInput;
    double utilityInput;
    while (readUtility >> itemInput >> utilityInput)
    {
        utility[itemInput-1] = utilityInput;
    }

    //Construct utility matrix
    int sequenceSize = 5;
//    int sequenceSize = 8842;
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
        utilityChain[i] = UtilityChain(utilityChainCount[i].size(), itemToSequence[i], i+1);
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

    /*double tempUtility = 0;
    double *PEUList = new double[utilityChain[0].sequenceCount];
    double totalPEU = 0;
    index = 0;
    for (int i = 0; i < utilityChain[0].sequenceCount; i++)
    {
        double temp = 0;
        double tempPEU = 0;
        for (int j = 0; j < utilityChain[0].chainIndex[i]; j++)
        {
            temp = max(temp, utilityChain[0].chain[i][j][1]);
            if (utilityChain[0].chain[i][j][2]>0)
            {
                tempPEU = max(tempPEU, utilityChain[0].chain[i][j][1] + utilityChain[0].chain[i][j][2]);
            }
        }
        tempUtility += temp;
        PEUList[index++] = tempPEU;
        totalPEU += tempPEU;
    }*/
    for (int i = 0; i < itemSize; i++)
    {
        double tempUtility = 0;
        double *tempPEUList = new double[utilityChain[i].sequenceCount];
        double totalPEU = 0;
        index = 0;
        for (int j = 0; j < utilityChain[i].sequenceCount; j++)
        {
            double temp = 0;
            double tempPEU = 0;
            for (int k = 0; k < utilityChain[i].chainIndex[j]; k++)
            {
                temp = max(temp, utilityChain[i].chain[j][k][1]);
                if (utilityChain[i].chain[j][k][2]>0)
                {
                    tempPEU = max(tempPEU, utilityChain[i].chain[j][k][1] + utilityChain[i].chain[j][k][2]);
                }
            }
            tempUtility += temp;
            tempPEUList[index++] = tempPEU;
            totalPEU += tempPEU;
        }
        utilityChain[i].utility = tempUtility;
        utilityChain[i].PEUList = tempPEUList;
        utilityChain[i].PEU = totalPEU;
    }

//    for (int i = 0; i < itemSize; i++)
//    {
//        cout << utilityChain[i].pattern << ": " << utilityChain[i].utility << ", " << utilityChain[i].PEU << endl;
//    }



//    for (int j = 0; j < utilityMatrix[2].itemSize; j++)
//    {
//        for (int k = 0; k < utilityMatrix[2].transactionSize; k++)
//        {
//
//            cout << utilityMatrix[2].utilityMatrix[k][j] << ", ";
//        }
//        cout << endl;
//    }


//    for (map<int, int>::iterator it=utilityChain[2].SequenceId2IndexMap.begin(); it!=utilityChain[2].SequenceId2IndexMap.end(); ++it)
//    {
//        //for each tid
//        for (int i = 0; i < utilityChain[2].chainIndex[it->second]; i++)
//        {
//            int tidd = utilityChain[2].chain[it->second][i][0];
//            int itemIndex = utilityMatrix[it->first].item2transactionIdMap[2];
//
//            for (int j = itemIndex+1; j < utilityMatrix[it->first].itemSize; j++)
//            {
//                int ttt = utilityMatrix[it->first].utilityMatrix[tidd][j];
//                cout << ttt;
//            }
//            utilityMatrix[it->first].utilityMatrix[tidd][itemIndex];
//        }
////        cout << it->first << ":" << it->second << ", " << endl;
//    }

//    for (map<int, int>::iterator it=utilityMatrix[2].item2transactionIdMap.begin(); it!=utilityMatrix[2].item2transactionIdMap.end(); ++it)
//    {
//        cout << it->first << ": " << it->second << endl;
//    }

    I(utilityChain[0], 2, utilityMatrix);
    return 0;
}
