#include <iostream>
#include <set>
#include <fstream>
#include <map>
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
//    int *utility = new int[5]{0};
    double *utility = new double[1559]{0};
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
    int sid, tid, item;
    double quantity;
    set<int> s;
    int tidSize = 0;
    int sidCount = -1;
    while (readData >> sid >> tid >> item >> quantity)
    {
//        cout << sid << ", " << tid << ", " << item << ", " << quantity << endl;
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
//            utilityTable[sid-2] = tempUtilityMatrix;
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

//    for(int i = 0; i < sequenceSize; i++)
//    {
//        for (int j = 0; j < utilityMatrix[i].transactionSize; j++)
//        {
//            for (int k = 0; k < utilityMatrix[i].itemSize; k++)
//            {
//                cout << utilityMatrix[i].utilityMatrix[j][k] << ", ";
//            }
//            cout << endl;
//        }
//        cout << endl;
//    }

    cout << utilityMatrix[0].transactionSize << ", " << utilityMatrix[0].itemSize << endl;
    for (int j = 0; j < utilityMatrix[0].transactionSize; j++)
        {
            for (int k = 0; k < utilityMatrix[0].itemSize; k++)
            {
                cout << utilityMatrix[0].utilityMatrix[j][k] << ", ";
            }
            cout << endl;
        }
    for (map<int, int>::iterator it=utilityMatrix[sid-1].item2transactionIdMap.begin(); it!=utilityMatrix[sid-1].item2transactionIdMap.end(); ++it)
    {
        cout << it->first << ": " << it->second << endl;
    }

    return 0;
}