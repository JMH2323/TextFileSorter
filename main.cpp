
#include <filesystem>
#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
#include <ctime>
#include <utility>
#include <vector>
#include <future>

// Simplify Namespaces.
using namespace std;
namespace fs = std::filesystem;



////// Definitions, Declarations

// Enable or Disable Multi-threading outputs for testing purposes.
#define MULTITHREADED_ENABLED 1

enum class ESortType { AlphAsc, AlphDesc, LastLetterAsc };

class IStringComparer {
public:
    virtual bool IsFirstAboveSecond(string firstString, string secondString) = 0;
};

class AlphAscStrComp : public IStringComparer {
public:
    bool IsFirstAboveSecond(string firstString, string secondString) override;
};

class AlphDescStrComp : public IStringComparer {
public:
    bool IsFirstAboveSecond(string firstString, string secondString) override;
};

class LastLetterAscStrComp : public IStringComparer {
public:
    bool IsFirstAboveSecond(string firstString, string secondString) override;

};


////// Function Prototypes
void singleThreading(const vector<string>& fileList, ESortType sortType, const string& outputName);
void multiThreading(vector<string> fileList, ESortType sortType, const string& outputName);
vector<string> ReadFile(const string& fileName);
vector<string> MergeSortWrapper(vector<string> listToSort, ESortType sortType);
void WriteAndPrint(const vector<string>& finalList, const string& outputName, int clockCounter);


////// Main
int main() {

    // Enumerate the directory for input files.
    vector<string> fileList;
    string inputDirectoryPath = "../InputText";
    for (const auto & entry : fs::directory_iterator(inputDirectoryPath)) {
        if (!fs::is_directory(entry)) {
            fileList.push_back(entry.path().string());
        }
    }

    // Start sorting sectioned by single threading and multi-threading.
    singleThreading(fileList, ESortType::AlphAsc, "AlphabeticalAscendingTextOutput");
    singleThreading(fileList, ESortType::AlphDesc, "AlphabeticalDescendingTextOutput");
    singleThreading(fileList, ESortType::LastLetterAsc, "LastLetterAscendingTextOutput");
#if MULTITHREADED_ENABLED
    multiThreading(fileList, ESortType::AlphAsc, "MultiAscTextOutput");
    multiThreading(fileList, ESortType::AlphDesc, "MultiDescTextOutput");
    multiThreading(fileList, ESortType::LastLetterAsc, "MultiLastLetterTextOutput");
#endif

    // Wait
    cout << endl << "Done...";
    getchar();
    return 0;
}


////// Single Threaded Sorting
void singleThreading(const vector<string>& fileList, ESortType sortType, const string& outputName) {

    // Use clocks to measure speed and efficiency.
    clock_t startTime = clock();
    vector<string> finalList;

    for (const auto & i : fileList) {
        vector<string> fileStringList = ReadFile(i);
        finalList.insert(finalList.end(), fileStringList.begin(), fileStringList.end());
    }

    // Sort the results and call time.
    finalList = MergeSortWrapper(finalList, sortType);
    clock_t endTime = clock();

    // Write the results.
    WriteAndPrint(finalList, outputName, endTime - startTime);
}


////// Multi-Threaded Sorting
void multiThreading(vector<string> fileList, ESortType sortType, const string& outputName) {

    // Use clocks to measure speed and efficiency.
    clock_t startTime = clock();
    vector<string> finalList;

    // Create vector of shared pointers and futures to track tasks and completion.
    vector<future<vector<string>>> futures(fileList.size());
    vector<shared_ptr<atomic<bool>>> done(fileList.size());

    for (size_t i = 0; i < fileList.size(); ++i) {

        // Initially set to not done. shared_ptr ensures it is alive for lambda operation.
        done[i] = make_shared<atomic<bool>>(false);
        // Pass the file and doneFlag to lambda for reading.
        futures[i] = async(launch::async, [](const string& file, const shared_ptr<atomic<bool>>& doneFlag) {
            auto result = ReadFile(file);


            // Set the done flag to true once ReadFile is done.
            *doneFlag = true;
            return result;
        }, fileList[i], done[i]);
    }
    bool allDone;
    do {
        // Yield the current thread to allow others to run.
        this_thread::yield();
        allDone = true;
        for (const auto& d : done) {
            allDone &= *d;
        }
    } while (!allDone);

    // When done, gather the results.
    for (auto& f : futures) {
        auto result = f.get();
        finalList.insert(finalList.end(), result.begin(), result.end());
    }

    // Sort the final results and call time.
    finalList = MergeSortWrapper(finalList, sortType);
    clock_t endTime = clock();

    // Write the results.
    WriteAndPrint(finalList, outputName, endTime - startTime);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// File Processing
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ContainsSpecial(const string& str) {
    for (char ch : str) {
        // Included std to acknowledge this comes from the standard library.
        if (std::isdigit(ch) || !std::isalnum(ch)) {
            return true;
        }
    }
    return false;
}

vector<string> ReadFile(const string& fileName) {
    vector<string> listOut;
    ifstream fileIn(fileName);

    // Checks for if the file is currently open.
    if (!fileIn.is_open()) {
        cout << "Unable to open file, please close input files: " << fileName << endl;
        return listOut;
    }

    // Checks if we can open the file at all.
    if (!fileIn) {
        cout << "Unable to open file\n";
        return listOut;
    }

    string line;
    while (getline(fileIn, line)) {
        // Skip empty lines.
        if (!line.empty()) {
            // Check for special characters or numbers.
            if (ContainsSpecial(line)) {
                cerr << "ERROR: special characters or numbers: " << line << " in file: " << fileName << endl;
                cerr << line << " has been removed" << endl;
                continue;
            }

            // Emplace over push back for good practice in optimizing speed.
            listOut.emplace_back(line);
        }
    }

    fileIn.close();
    return listOut;
}



////// Sorting two words methods
bool AlphAscStrComp::IsFirstAboveSecond(string firstString, string secondString) {
    unsigned int i = 0;
    while (i < firstString.length() && i < secondString.length()) {
        if (firstString[i] < secondString[i])
            return true;
        else if (firstString[i] > secondString[i])
            return false;
        ++i;
    }
    return (firstString.length() < secondString.length());
}

// Descending String comparer in same format.
bool AlphDescStrComp::IsFirstAboveSecond(string firstString, string secondString) {
    unsigned int i = 0;
    while (i < firstString.length() && i < secondString.length()) {
        if (firstString[i] > secondString[i])
            return true;
        else if (firstString[i] < secondString[i])
            return false;
        ++i;
    }
    return (secondString.length() < firstString.length());
}

// Last Letter comparer in different format.
bool LastLetterAscStrComp::IsFirstAboveSecond(string firstString, string secondString) {

        // Start from the end and work to the front. Loop is designed to rely on return statements.
    for (auto reverseIt1 = firstString.rbegin(), reverseIt2 = secondString.rbegin(); ;
         ++reverseIt1, ++reverseIt2) {

        // If we reach the beginning of either string, the "prefix" goes first.
        if (reverseIt1 == firstString.rend()) return true;
        if (reverseIt2 == secondString.rend()) return false;

        // If characters are not equal, return comparison result
        if (*reverseIt1 != *reverseIt2) return *reverseIt1 < *reverseIt2;
    }
}


////// MergeSorting Algorithm
void merge(vector<string>& originVec, int upper, int mid, int lower, IStringComparer* stringComparer) {

    int i, j, k, upperSize, lowerSize;

    // Size of upper and lower sub-arrays.
    upperSize = mid - upper + 1;
    lowerSize = lower - mid;

    // Temporary vectors to store upper side and lower side.
    vector<string> upArray(upperSize), lowArray(lowerSize);

    // Fill the upper sub-array.
    for(i = 0; i < upperSize; i++)
        upArray[i] = originVec[upper + i];

    // Fill the lower sub-array.
    for(j = 0; j < lowerSize; j++)
        lowArray[j] = originVec[mid + 1 + j];

    // Reset indices. Since "upper" is actually our left side (we are reading "top down"),
    // we set k to equal upper, or the smallest index.
    i = 0; j = 0; k = upper;

    // Merge the temporary arrays to the real array.
    while(i < upperSize && j < lowerSize) {
        // Here, we invert comparison from traditional Merge Sort methodology to match our...
        // string comparison methods.
        if(stringComparer->IsFirstAboveSecond(upArray[i], lowArray[j]))
            // If upper array element is first, it is placed in the original array,
            // and we move to the next element in the upper array.
            originVec[k] = upArray[i++];
        else
            // Same logic as above.
            originVec[k] = lowArray[j++];

        // k iterates through the whole vector, indicating our point in the original.
        k++;
    }
    // For any extra element in the upper array.
    while(i < upperSize) {
        originVec[k] = upArray[i++];
        k++;
    }
    // For any extra element in the lower array.
    while(j < lowerSize) {
        originVec[k] = lowArray[j++];
        k++;
    }
}

void MergeSort(vector<string>& originVec, int upper, int lower, IStringComparer* stringComparer){

    // Base Case. if this is false, subarray has 0-1 elements or is sorted.
    if (upper < lower) {
        // Calculate middle index.
        int mid = upper + (lower - upper) / 2;

        // Sort first and second halves through recursive calls.
        MergeSort(originVec, upper, mid, stringComparer);
        MergeSort(originVec, mid + 1, lower, stringComparer);

        // After halves are sorted, merge morphs the halves into a single sorted array.
        merge(originVec, upper, mid, lower, stringComparer);
    }
}

vector<string> MergeSortWrapper(vector<string> listToSort, ESortType sortType){
    // Similar switch statement to create stringSorter object depending on the Sort type needed.
    IStringComparer* stringSorter;
    switch(sortType) {
        case ESortType::AlphAsc:
            stringSorter = new AlphAscStrComp();
            break;
        case ESortType::AlphDesc:
            stringSorter = new AlphDescStrComp();
            break;
        case ESortType::LastLetterAsc:
            stringSorter = new LastLetterAscStrComp();
            break;
        default:
            cerr << "ERROR: Unknown sort type in MergeSortWrapper. defaulting to AlphAsc" << endl;
            stringSorter = new AlphAscStrComp();
            break;
    }

    // After finding the correct sorting method, we pass the list and the method to MergeSort.
    MergeSort(listToSort, 0, listToSort.size() - 1, stringSorter);

    // Delete the object to avoid memory leaks.
    delete stringSorter;
    return listToSort;
}


////// Output
void WriteAndPrint(const vector<string>& finalList, const string& outputName, int clockCounter) {

    // Track the times for testing
    cout << endl << outputName << "\t- Time Taken (clocks): " << clockCounter << endl;

    // Output directory and file pathing.
    const std::string outputDirectory = "../OutputText/";
    std::string filePath = outputDirectory + outputName + ".txt";

    ofstream fileOut(filePath, ofstream::trunc);
    for (const auto & i : finalList) {
        fileOut << i << endl;
    }
    fileOut.close();
}
