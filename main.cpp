#include <iostream>
#include <set>
#include <fstream>
#include <map>
#include <string>
#include <chrono>

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
    int lastPattern;
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
        this->lastPattern = pattern;
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
    a.pattern = utilityChain.pattern + "," + to_string(extensionItem+1);
    a.lastPattern = extensionItem+1;

    return a;
};

UtilityChain S(UtilityChain utilityChain, int extensionItem, UtilityMatrix *utilityMatrix){
    int seqSize = 2;
    int sequenceCount = 0;
    int *chainIndex = new int[seqSize];
    double ***chain = new double**[seqSize];
    map<int, int> chainItemToIndex;
    for (map<int, int>::iterator it=utilityChain.SequenceId2IndexMap.begin(); it != utilityChain.SequenceId2IndexMap.end(); ++it)
    {
        int size = 0;
        int tidIndexOnUM = utilityChain.chain[it->second][0][0]+1;
        if (!utilityMatrix[it->first].item2transactionIdMap.count(extensionItem+1))
        {
            continue;
        }
        for (int j = tidIndexOnUM; j < utilityMatrix[it->first].transactionSize; j++)
        {
            if (utilityMatrix[it->first].utilityMatrix[j][utilityMatrix[it->first].item2transactionIdMap[extensionItem+1]] != 0)
            {
                size++;
            }
        }
        if (size != 0)
        {
            double **temp = new double*[size];
            int index = 0;
            for (int j = tidIndexOnUM; j < utilityMatrix[it->first].transactionSize; j++)
            {
                if (utilityMatrix[it->first].utilityMatrix[j][utilityMatrix[it->first].item2transactionIdMap[extensionItem+1]] != 0)
                {
                    temp[index] = new double[3];
                    temp[index][0] = j;
                    double tempUtility = 0;
                    for (int k = 0; k < utilityChain.chainIndex[it->second]; k++)
                    {
                        if (utilityChain.chain[it->second][k][0] >= j)
                        {
                            break;
                        }
                        tempUtility = max(tempUtility, utilityChain.chain[it->second][k][1]);
                    }
                    temp[index][1] = tempUtility+utilityMatrix[it->first].utilityMatrix[j][utilityMatrix[it->first].item2transactionIdMap[extensionItem+1]];
                    temp[index][2] = utilityMatrix[it->first].remainingUtilityMatrix[j][utilityMatrix[it->first].item2transactionIdMap[extensionItem+1]];
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
    a.pattern = utilityChain.pattern +"}{"+ to_string(extensionItem+1);
    a.lastPattern = extensionItem+1;

    return a;
}

void HUSSpan(UtilityMatrix *utilityMatrix, double threshold, UtilityChain utilityChain, ofstream& outFile)
{
    if (utilityChain.PEU < threshold)
    {
        return;
    }
    set<int> iList;
    set<int> iItemList;
    map<int, int> iRSU;
    set<int> sList;
    set<int> sItemList;
    map<int, int> sRSU;
    for (map<int, int>::iterator it=utilityChain.SequenceId2IndexMap.begin(); it != utilityChain.SequenceId2IndexMap.end(); ++it)
    {
        //i-candidate
        for (int i = 0; i < utilityChain.chainIndex[it->second]; i++)
        {
            int tidIndexOnUM = utilityChain.chain[it->second][i][0];
            for (int j = utilityMatrix[it->first].item2transactionIdMap[utilityChain.lastPattern]+1; j < utilityMatrix[it->first].itemSize; j++)
            {
                if (utilityMatrix[it->first].utilityMatrix[tidIndexOnUM][j] != 0)
                {
                    for (map<int, int>::iterator iterator=utilityMatrix[it->first].item2transactionIdMap.begin(); iterator!=utilityMatrix[it->first].item2transactionIdMap.end(); ++iterator)
                    {
                        if (iterator->second==j)
                        {
                            iList.insert(iterator->first);
                            if (!iRSU.count(iterator->first))
                                iRSU[iterator->first] = 0;
                        }
                    }
                }
            }
        }
        for (map<int, int>::iterator it2=utilityMatrix[it->first].item2transactionIdMap.begin(); it2 != utilityMatrix[it->first].item2transactionIdMap.end(); ++it2)
        {
            if (iList.count(it2->first))
            {
                iRSU[it2->first] += utilityChain.PEUList[it->second];
            }
        }
        for (set<int>::iterator iterator=iList.begin(); iterator!=iList.end(); ++iterator)
        {
            iItemList.insert(*iterator);
        }
        iList.clear();

        //s-candidate
        int tempTid = utilityChain.chain[it->second][0][0]+1;
        for (int i = tempTid; i < utilityMatrix[it->first].transactionSize; i++)
        {
            for (map<int, int>::iterator iterator=utilityMatrix[it->first].item2transactionIdMap.begin(); iterator!=utilityMatrix[it->first].item2transactionIdMap.end(); ++iterator)
            {
                if (utilityMatrix[it->first].utilityMatrix[i][iterator->second] != 0)
                {
                    sList.insert(iterator->first);
                    if (!sRSU.count(iterator->first))
                    {
                        sRSU[iterator->first] = 0;
                    }
                }
            }
        }

        for (map<int, int>::iterator it2=utilityMatrix[it->first].item2transactionIdMap.begin(); it2 != utilityMatrix[it->first].item2transactionIdMap.end(); ++it2)
        {
            if (sList.count(it2->first))
            {
                sRSU[it2->first] += utilityChain.PEUList[it->second];
            }
        }

        for (set<int>::iterator iterator=sList.begin(); iterator!=sList.end(); ++iterator)
        {
            sItemList.insert(*iterator);
        }
        sList.clear();
    }

    //RSU pruning;
    for (map<int, int>::iterator iterator=iRSU.begin(); iterator!=iRSU.end(); ++iterator)
    {
        if (iterator->second>=threshold)
        {
            UtilityChain tPrime = I(utilityChain, iterator->first-1, utilityMatrix);
            if (tPrime.utility >= threshold)
            {
                outFile << "{" << tPrime.pattern << "}= " << tPrime.utility << ", " << tPrime.PEU << endl;
                cout << "{" << tPrime.pattern << "}= " << tPrime.utility << ", " << tPrime.PEU << endl;
            }
            HUSSpan(utilityMatrix, threshold, tPrime, outFile);
            for (int i = 0; i < tPrime.sequenceCount; i++)
            {
                for (int j = 0; j < tPrime.chainIndex[i]; j++)
                {
                    delete [] tPrime.chain[i][j];
                }
                delete [] tPrime.chain[i];
            }
            delete [] tPrime.chain;
            delete [] tPrime.PEUList;
            delete [] tPrime.chainIndex;
        }
    }

    for (map<int, int>::iterator iterator=sRSU.begin(); iterator!=sRSU.end(); ++iterator)
    {
        if (iterator->second>=threshold)
        {
            UtilityChain tPrime = S(utilityChain, iterator->first-1, utilityMatrix);
            if (tPrime.utility >= threshold)
            {
                outFile << "{" << tPrime.pattern << "}= " << tPrime.utility << ", " << tPrime.PEU << endl;
                cout << "{" << tPrime.pattern << "}= " << tPrime.utility << ", " << tPrime.PEU << endl;
            }
            HUSSpan(utilityMatrix, threshold, tPrime, outFile);
            for (int i = 0; i < tPrime.sequenceCount; i++)
            {
                for (int j = 0; j < tPrime.chainIndex[i]; j++)
                {
                    delete [] tPrime.chain[i][j];
                }
                delete [] tPrime.chain[i];
            }
            delete [] tPrime.chain;
            delete [] tPrime.PEUList;
            delete [] tPrime.chainIndex;
        }
    }
}

int main() {
    ifstream readUtility;
    readUtility.open("C:\\Users\\Ken\\CLionProjects\\HUSSpanLargeData\\foodmartData\\foodmart_until1998_utb.txt");
//    readUtility.open("C:\\Users\\Ken\\CLionProjects\\HUSSpanLargeData\\paperData\\originalUtility.txt");
//    readUtility.open("C:\\Users\\mio\\CLionProjects\\untitled4\\paperData\\originalUtility.txt");
//    readUtility.open("C:\\Users\\mio\\CLionProjects\\untitled4\\foodmartData\\foodmart_until1998_utb.txt");
    ifstream readData;
    readData.open("C:\\Users\\Ken\\CLionProjects\\HUSSpanLargeData\\foodmartData\\foodmart_until1998.txt");
//    readData.open("C:\\Users\\Ken\\CLionProjects\\HUSSpanLargeData\\paperData\\originalData.txt");
//    readData.open("C:\\Users\\mio\\CLionProjects\\untitled4\\paperData\\originalData.txt");
//    readData.open("C:\\Users\\mio\\CLionProjects\\untitled4\\foodmartData\\foodmart_until1998.txt");
    ifstream readData2;
    readData2.open("C:\\Users\\Ken\\CLionProjects\\HUSSpanLargeData\\foodmartData\\foodmart_until1998.txt");
//    readData2.open("C:\\Users\\Ken\\CLionProjects\\HUSSpanLargeData\\paperData\\originalData.txt");
//    readData2.open("C:\\Users\\mio\\CLionProjects\\untitled4\\paperData\\originalData.txt");
//    readData2.open("C:\\Users\\mio\\CLionProjects\\untitled4\\foodmartData\\foodmart_until1998.txt");
    if (!readUtility.is_open() || !readData.is_open() || !readData2.is_open()) {
        cerr << "Failed to read data.\n";
        return 1;
    }

    //Construct external utility
//    int itemSize = 5;
//    double *utility = new double[itemSize]{0};
    int itemSize = 1559;
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

    double totalUtility = 0;
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
        totalUtility += cumulateUtility;
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

    UtilityChain *utilityChain = new UtilityChain[itemSize];
    for (int i = 0; i < itemSize; i++)
    {
        utilityChain[i] = UtilityChain(utilityChainCount[i].size(), itemToSequence[i], i+1);
    }

    for(int i = 0; i < sequenceSize; i++)
    {
        for (map<int, int>::iterator it=utilityMatrix[i].item2transactionIdMap.begin(); it != utilityMatrix[i].item2transactionIdMap.end(); ++it)
        {
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

    ofstream outFile;
    outFile.open("C:\\Users\\Ken\\CLionProjects\\HUSSpanLargeScale\\test3.txt");
    if (!outFile.is_open())
    {
        cout << "Unable to write file" << endl;
        return -1;
    }
    double thresholdRatio = 0.001721;
    double threshold = thresholdRatio*totalUtility;
    cout << "Threshold ratio: " << thresholdRatio << ", Threshold utility: " << threshold << endl;

    chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();
    for (int i = 0; i < itemSize; i++)
    {
        if (utilityChain[i].utility >= threshold)
        {
            outFile << "{" << utilityChain[i].pattern << "}= " << utilityChain[i].utility << ", " << utilityChain[i].PEU << endl;
            cout << "{" << utilityChain[i].pattern << "}= " << utilityChain[i].utility << ", " << utilityChain[i].PEU << endl;
        }
        HUSSpan(utilityMatrix, threshold, utilityChain[i], outFile);
    }
    chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
    chrono::milliseconds duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "Execution time: " << duration.count() << " milliseconds" << endl;

    outFile.close();

    //memory release
    delete [] utility;
    for (int i = 0; i < sequenceSize; i++)
    {
        for (int j = 0; j < utilityMatrix[i].transactionSize; j++)
        {
            delete [] utilityMatrix[i].utilityMatrix[j];
            delete [] utilityMatrix[i].remainingUtilityMatrix[j];
        }
        delete [] utilityMatrix[i].utilityMatrix;
        delete [] utilityMatrix[i].remainingUtilityMatrix;
    }
    delete [] utilityMatrix;
//    delete [] utilityChain;
    return 0;
}