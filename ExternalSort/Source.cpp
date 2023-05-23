#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <filesystem>
#if __cplusplus > 201402L
#include <filesystem>
#endif

// Функция для разбиения большого входного файла на более мелкие отсортированные фрагменты
void createSortedChunks(const std::string& inputFile, const std::string& tempDir, int chunkSize) {
    std::ifstream input(inputFile);
    std::string line;
    int chunkCount = 0;

    while (std::getline(input, line)) {
        std::vector<std::string> chunk;
        chunk.reserve(chunkSize);

        while (chunk.size() < chunkSize && !line.empty()) {
            chunk.push_back(line);
            if (!std::getline(input, line))
                break;
        }

     
        std::sort(chunk.begin(), chunk.end(), [](const std::string& a, const std::string& b)->bool {
            return std::lexicographical_compare(a.begin(), a.end(),
            b.begin(), b.end());
            });

        std::ofstream tempFile(tempDir + "/chunk_" + std::to_string(chunkCount));
        for (const std::string& str : chunk)
            tempFile << str << '\n';

        chunkCount++;
    }

    input.close();
}

// Функция для объединения отсортированной части в один выходной файл
void mergeSortedChunks(const std::string& tempDir, const std::string& outputFile) {
    std::vector<std::ifstream> chunkStreams;
    std::ofstream output(outputFile);

    // Открыть входные потоки для каждого отсортированной части
    for (int i = 0; ; i++) {
        std::string chunkFile = tempDir + "/chunk_" + std::to_string(i);
        std::ifstream chunkStream(chunkFile);

        if (!chunkStream)
            break;

        chunkStreams.push_back(std::move(chunkStream));
    }

    std::vector<std::string> currentLine(chunkStreams.size());
    std::vector<bool> isValidLine(chunkStreams.size(), true);
    for (size_t i = 0; i < chunkStreams.size(); i++)
        isValidLine[i] = !std::getline(chunkStreams[i], currentLine[i]).eof();

    while (true) {
        int smallestIdx = -1;
        std::string smallestStr;
        for (size_t i = 0; i < chunkStreams.size(); i++) {
            if (isValidLine[i]) {

                if(smallestIdx == -1 || std::lexicographical_compare(
                    currentLine[i].begin(), currentLine[i].end(), 
                    smallestStr.begin(), smallestStr.end())
                    ){
                    smallestIdx = i;
                    smallestStr = currentLine[i];
                }
            }
        }

        if (smallestIdx == -1)
            break;

        output << smallestStr << std::endl;

        if (!std::getline(chunkStreams[smallestIdx], currentLine[smallestIdx]))
            isValidLine[smallestIdx] = false;
    }

    for (std::ifstream& chunkStream : chunkStreams)
        chunkStream.close();
    output.close();
}

inline std::string getTime(
    std::chrono::steady_clock::time_point t1,
    std::chrono::steady_clock::time_point t2) {
    std::string s = std::to_string((long double)std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t2).count() / 1000).append(" seconds.");
    return s;
}

void clearDir(const std::string& tempDir) {
    for (int i = 0; ; i++) {
        std::string chunkFile = tempDir + "/chunk_" + std::to_string(i);
        int check = std::remove(chunkFile.c_str());
        if(check)
            break;

     
    }
}

void externalSort(const std::string& inputFile, const std::string& outputFile, const std::string& tempDir, int chunkSize) {
    clearDir(tempDir);
    auto timer1 = std::chrono::steady_clock::now();
    createSortedChunks(inputFile, tempDir, chunkSize);
    auto timer2 = std::chrono::steady_clock::now();
    std::cout << "Sorting time: " << getTime(timer2, timer1) << std::endl;
    mergeSortedChunks(tempDir, outputFile);
    auto timer3 = std::chrono::steady_clock::now();
    std::cout << "Merging time: " << getTime(timer3, timer2) << std::endl;
}

void executeProgram(const std::string& ifile, const std::string& ofile, const std::string& tdir) {
    std::string inputFile = ifile;  // Replace with your input file name
    std::string outputFile = ofile;  // Replace with your output file name
    std::string tempDir = tdir;  // Replace with your temporary directory name
    int chunkSize = 200000;  // кол-во строк в фрагменте

    auto timer1 = std::chrono::steady_clock::now();
    externalSort(inputFile, outputFile, tempDir, chunkSize);
    auto timer2 = std::chrono::steady_clock::now();
    std::cout << "Sorting completed successfully. Time: " << getTime(timer2, timer1) << std::endl;
}

int main() {


#ifdef _WIN32 && (__cplusplus > 201402L)
    std::filesystem::path cwd = std::filesystem::current_path();
    std::string p = cwd.string();
    executeProgram(p + "/input.txt", p + "/output.txt", p + "/temp");
#elif _WIN32 && (__cplusplus < 201402L)
    std::cout << "No c++17" << std::endl;// _WIN32
#endif

#if defined(__linux__) && (__cplusplus > 201402L)
    std::filesystem::path cwd = std::filesystem::current_path();
    std::string p = cwd.string();
    executeProgram(p + "/input.txt", p + "/output.txt", p + "/temp");
#elif defined(__linux__) && (__cplusplus < 201402L)
    std::string path{ "/media/sd0/ExternalSort/" };
    executeProgram(path + "input.txt", path + "output.txt", path + "temp");
#endif

    return 0;
}
