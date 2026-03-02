#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <string>
#include <direct.h>
#include <sys/stat.h>
#include <windows.h>

using namespace std;
using namespace chrono;

// Конфигурация тестирования
const vector<int> SIZES = {1000, 2000, 4000, 8000, 16000, 32000, 64000, 128000};
const int ATTEMPTS_PER_SIZE = 20;
const string RESULTS_DIR = "results";

// Функция для создания директории в Windows
bool createDirectory(const string& path) {
    return _mkdir(path.c_str()) == 0;
}

// Проверка существования директории
bool directoryExists(const string& path) {
    struct _stat info;
    return _stat(path.c_str(), &info) == 0 && (info.st_mode & _S_IFDIR);
}

// Класс для замера времени
class Timer {
private:
    high_resolution_clock::time_point start_time;
    
public:
    void start() {
        start_time = high_resolution_clock::now();
    }
    
    double getElapsedMilliseconds() {
        auto end_time = high_resolution_clock::now();
        duration<double, milli> diff = end_time - start_time;
        return diff.count();
    }
};

// Класс для сортировки пузырьком 
class BubbleSorter {
private:
    long long swap_count;      // количество обменов
    long long comparison_count; // количество сравнений
    long long pass_count;       // количество проходов
    
public:
    BubbleSorter() : swap_count(0), comparison_count(0), pass_count(0) {}
    
    // Оптимизированная сортировка пузырьком
    void sort(vector<double>& arr) {
        swap_count = 0;
        comparison_count = 0;
        pass_count = 0;
        
        int n = arr.size();
        bool swapped;
        
        for (int i = 0; i < n - 1; i++) {
            swapped = false;
            pass_count++;
            
            for (int j = 0; j < n - i - 1; j++) {
                comparison_count++;
                if (arr[j] > arr[j + 1]) {
                    swap(arr[j], arr[j + 1]);
                    swap_count++;
                    swapped = true;
                }
            }
            
            if (!swapped) {
                break;
            }
        }
    }
    
    // Геттеры
    long long getSwapCount() const { return swap_count; }
    long long getComparisonCount() const { return comparison_count; }
    long long getPassCount() const { return pass_count; }
    
    void resetCounters() {
        swap_count = 0;
        comparison_count = 0;
        pass_count = 0;
    }
};

// Генерация случайного массива
vector<double> generateRandomArray(int size) {
    random_device rd;
    mt19937 engine(rd());
    uniform_real_distribution<double> gen(-1.0, 1.0);
    
    vector<double> arr(size);
    for (auto& el : arr) {
        el = gen(engine);
    }
    return arr;
}

// Проверка сортировки
bool isSorted(const vector<double>& arr) {
    for (size_t i = 1; i < arr.size(); i++) {
        if (arr[i] < arr[i - 1]) {
            return false;
        }
    }
    return true;
}

// Создание директории для результатов
void createResultsDirectory() {
    if (directoryExists(RESULTS_DIR)) {
        cout << "[OK] Directory '" << RESULTS_DIR << "' already exists\n";
    } else if (createDirectory(RESULTS_DIR)) {
        cout << "[OK] Directory '" << RESULTS_DIR << "' created\n";
    } else {
        cerr << "[ERROR] Failed to create directory '" << RESULTS_DIR << "'\n";
    }
}

// Сохранение всех результатов в CSV
void saveAllResults(const vector<int>& sizes,
                    const vector<vector<double>>& all_times,
                    const vector<vector<long long>>& all_swaps,
                    const vector<vector<long long>>& all_comparisons,
                    const vector<vector<long long>>& all_passes) {
    
    string filename = RESULTS_DIR + "/all_results.csv";
    ofstream file(filename);
    
    if (!file.is_open()) {
        cerr << "[ERROR] Cannot open file: " << filename << endl;
        return;
    }
    
    file << "N,Attempt,Time_ms,Swaps,Comparisons,Passes\n";
    
    // Данные
    for (size_t i = 0; i < sizes.size(); i++) {
        for (size_t j = 0; j < all_times[i].size(); j++) {
            file << sizes[i] << ","
                 << j + 1 << ","
                 << fixed << setprecision(6) << all_times[i][j] << ","
                 << all_swaps[i][j] << ","
                 << all_comparisons[i][j] << ","
                 << all_passes[i][j] << "\n";
        }
    }
    
    file.close();
    cout << "[OK] All results saved: " << filename << endl;
}

// Сохранение статистики
void saveStatistics(const vector<int>& sizes,
                    const vector<vector<double>>& all_times,
                    const vector<vector<long long>>& all_swaps,
                    const vector<vector<long long>>& all_comparisons,
                    const vector<vector<long long>>& all_passes) {
    
    string filename = RESULTS_DIR + "/statistics.csv";
    ofstream file(filename);
    
    if (!file.is_open()) {
        cerr << "[ERROR] Cannot open statistics file: " << filename << endl;
        return;
    }
    
    file << "N,Best_Time_ms,Worst_Time_ms,Avg_Time_ms,"
         << "Avg_Swaps,Avg_Comparisons,Avg_Passes,Theoretical_O_n2\n";
    
    for (size_t i = 0; i < sizes.size(); i++) {
        // Вычисление статистики
        double min_time = *min_element(all_times[i].begin(), all_times[i].end());
        double max_time = *max_element(all_times[i].begin(), all_times[i].end());
        
        double sum_time = 0, sum_swaps = 0, sum_comparisons = 0, sum_passes = 0;
        for (size_t j = 0; j < all_times[i].size(); j++) {
            sum_time += all_times[i][j];
            sum_swaps += all_swaps[i][j];
            sum_comparisons += all_comparisons[i][j];
            sum_passes += all_passes[i][j];
        }
        
        double avg_time = sum_time / all_times[i].size();
        double avg_swaps = sum_swaps / all_swaps[i].size();
        double avg_comparisons = sum_comparisons / all_comparisons[i].size();
        double avg_passes = sum_passes / all_passes[i].size();
        
        // Теоретическая сложность O(n²) с коэффициентом для визуализации
        double theoretical = 0.0000005 * sizes[i] * sizes[i];
        
        file << sizes[i] << ","
             << fixed << setprecision(6) << min_time << ","
             << max_time << ","
             << avg_time << ","
             << avg_swaps << ","
             << avg_comparisons << ","
             << avg_passes << ","
             << theoretical << "\n";
    }
    
    file.close();
    cout << "[OK] Statistics saved: " << filename << endl;
}

// Тестирование одного размера
void testSize(int size, int attempts,
              vector<double>& times,
              vector<long long>& swaps,
              vector<long long>& comparisons,
              vector<long long>& passes) {
    
    BubbleSorter sorter;
    Timer timer;
    
    times.clear();
    swaps.clear();
    comparisons.clear();
    passes.clear();
    
    for (int attempt = 0; attempt < attempts; attempt++) {
        // Генерация массива
        vector<double> arr = generateRandomArray(size);
        
        // Сортировка с замером
        sorter.resetCounters();
        timer.start();
        sorter.sort(arr);
        double elapsed = timer.getElapsedMilliseconds();
        
        // Проверка
        if (!isSorted(arr)) {
            cerr << "\n[ERROR] Array is not sorted correctly!\n";
            return;
        }
        
        // Сохранение результатов
        times.push_back(elapsed);
        swaps.push_back(sorter.getSwapCount());
        comparisons.push_back(sorter.getComparisonCount());
        passes.push_back(sorter.getPassCount());
        
        // Индикатор прогресса
        if ((attempt + 1) % 5 == 0) {
            cout << "   Progress: " << attempt + 1 << "/" << attempts 
                 << " (last time: " << fixed << setprecision(3) 
                 << elapsed << " ms)\n";
        }
    }
}

int main() {
    cout << "========================================================\n";
    cout << "   Laboratory Work #1: Bubble Sort (Variant 1)        \n";
    cout << "========================================================\n\n";
    
    // Создание директории для результатов
    createResultsDirectory();
    
    cout << "\nTest Parameters:\n";
    cout << "   • Array sizes: ";
    for (int size : SIZES) cout << size << " ";
    cout << "\n   • Attempts per size: " << ATTEMPTS_PER_SIZE << "\n\n";
    
    // Векторы для хранения результатов
    vector<vector<double>> all_times(SIZES.size());
    vector<vector<long long>> all_swaps(SIZES.size());
    vector<vector<long long>> all_comparisons(SIZES.size());
    vector<vector<long long>> all_passes(SIZES.size());
    
    cout << "Starting tests...\n\n";
    
    // Основной цикл по размерам
    for (size_t i = 0; i < SIZES.size(); i++) {
        int current_size = SIZES[i];
        cout << ">> Testing N = " << current_size << "\n";
        
        testSize(current_size, ATTEMPTS_PER_SIZE,
                 all_times[i], all_swaps[i], 
                 all_comparisons[i], all_passes[i]);
        
        // Вывод средних результатов для текущего размера
        double avg_time = 0;
        for (double t : all_times[i]) avg_time += t;
        avg_time /= all_times[i].size();
        
        double avg_swaps = 0;
        for (long long s : all_swaps[i]) avg_swaps += s;
        avg_swaps /= all_swaps[i].size();
        
        cout << "   [OK] Average time: " << fixed << setprecision(3) << avg_time << " ms\n";
        cout << "   [OK] Average swaps: " << fixed << setprecision(0) << avg_swaps << "\n\n";
    }
    
    // Сохранение результатов
    cout << "Saving results...\n";
    saveAllResults(SIZES, all_times, all_swaps, all_comparisons, all_passes);
    saveStatistics(SIZES, all_times, all_swaps, all_comparisons, all_passes);
    
    cout << "\n Testing completed!\n";
    cout << "Results saved in 'results/' folder:\n";
    cout << "   - all_results.csv - all measurements\n";
    cout << "   - statistics.csv - statistics for graphs\n\n";
    
    system("pause");
    return 0;
}