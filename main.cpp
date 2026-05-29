#include <fstream>
#include <vector>
#include <string>

using namespace std;

// удаляем пробелы и переносы строк
string clean(string s) {
    string res;
    for (char c : s) {
        if (c != ' ' && c != '\n' && c != '\t' && c != '\r') {
            res += c;
        }
    }
    return res;
}

// кошелёк
vector<pair<int, int>> parseWallet(string json) {
    vector<pair<int, int>> wallet;
    
    // ищем [ [
    size_t start = json.find("[[");
    if (start == string::npos) return wallet;
    
    // ищем ] ]
    size_t end = json.find("]]", start);
    if (end == string::npos) return wallet;
    
    // вырезаем внутренность
    string data = json.substr(start + 2, end - start - 2);
    
    // разбираем пары
    for (size_t i = 0; i < data.length(); i++) {
        if (data[i] == '[') {
            i++;
            string valStr;
            string cntStr;
            
            // читаем число до запятой
            while (i < data.length() && data[i] != ',') {
                valStr += data[i];
                i++;
            }
            i++; // пропускаем запятую
            
            // читаем число до ]
            while (i < data.length() && data[i] != ']') {
                cntStr += data[i];
                i++;
            }
            
            int val = stoi(valStr);
            int cnt = stoi(cntStr);
            wallet.push_back({val, cnt});
        }
    }
    return wallet;
}

// amount
int parseAmount(string json) {
    size_t pos = json.find("\"amount\"");
    if (pos == string::npos) return 0;
    
    pos = json.find(":", pos);
    if (pos == string::npos) return 0;
    
    pos++;
    while (pos < json.size() && !isdigit(json[pos])) pos++;
    
    int res = 0;
    while (pos < json.size() && isdigit(json[pos])) {
        res = res * 10 + (json[pos] - '0');
        pos++;
    }
    return res;
}

// стратегия
string parseStrategy(string json) {
    size_t pos = json.find("\"strategy\"");
    if (pos == string::npos) return "";
    
    pos = json.find(":", pos);
    if (pos == string::npos) return "";
    
    pos = json.find("\"", pos);
    if (pos == string::npos) return "";
    pos++;
    
    string res;
    while (pos < json.size() && json[pos] != '"') {
        res += json[pos];
        pos++;
    }
    return res;
}

// MAX
vector<pair<int, int>> solveMAX(vector<pair<int, int>> w, int need) {
    // сортировка по убыванию
    for (int i = 0; i < (int)w.size() - 1; i++) {
        for (int j = 0; j < (int)w.size() - i - 1; j++) {
            if (w[j].first < w[j + 1].first) {
                swap(w[j], w[j + 1]);
            }
        }
    }
    
    vector<pair<int, int>> res;
    int rem = need;
    
    for (int i = 0; i < (int)w.size(); i++) {
        int val = w[i].first;
        int cnt = w[i].second;
        int take = cnt;
        if (val * take > rem) take = rem / val;
        if (take > 0) {
            res.push_back({val, take});
            rem -= take * val;
        }
    }
    
    if (rem != 0) res.clear();
    return res;
}

// MIN
vector<pair<int, int>> solveMIN(vector<pair<int, int>> w, int need) {
    // сортировка по возрастанию
    for (int i = 0; i < (int)w.size() - 1; i++) {
        for (int j = 0; j < (int)w.size() - i - 1; j++) {
            if (w[j].first > w[j + 1].first) {
                swap(w[j], w[j + 1]);
            }
        }
    }
    
    vector<pair<int, int>> res;
    int rem = need;
    
    for (int i = 0; i < (int)w.size(); i++) {
        int val = w[i].first;
        int cnt = w[i].second;
        int take = cnt;
        if (val * take > rem) take = rem / val;
        if (take > 0) {
            res.push_back({val, take});
            rem -= take * val;
        }
    }
    
    if (rem != 0) res.clear();
    return res;
}

// UNIFORM - рекурсия
void dfs(vector<pair<int, int>>& w, int idx, int rem,
         vector<int>& cur, vector<int>& best, int& bestDiff) {
    if (idx == (int)w.size()) {
        if (rem == 0) {
            int mx = 0, mn = 999999999;
            for (int i = 0; i < (int)cur.size(); i++) {
                if (cur[i] > mx) mx = cur[i];
                if (cur[i] < mn) mn = cur[i];
            }
            int diff = mx - mn;
            if (diff < bestDiff) {
                bestDiff = diff;
                best = cur;
            }
        }
        return;
    }
    
    int val = w[idx].first;
    int cnt = w[idx].second;
    int maxTake = cnt;
    if (val * maxTake > rem) maxTake = rem / val;
    
    for (int t = 0; t <= maxTake; t++) {
        cur[idx] = t;
        dfs(w, idx + 1, rem - t * val, cur, best, bestDiff);
    }
}

vector<pair<int, int>> solveUNIFORM(vector<pair<int, int>> w, int need) {
    // сортировка по возрастанию
    for (int i = 0; i < (int)w.size() - 1; i++) {
        for (int j = 0; j < (int)w.size() - i - 1; j++) {
            if (w[j].first > w[j + 1].first) {
                swap(w[j], w[j + 1]);
            }
        }
    }
    
    vector<int> cur(w.size(), 0);
    vector<int> best(w.size(), 0);
    int bestDiff = 999999999;
    
    dfs(w, 0, need, cur, best, bestDiff);
    
    vector<pair<int, int>> res;
    for (int i = 0; i < (int)w.size(); i++) {
        if (best[i] > 0) {
            res.push_back({w[i].first, best[i]});
        }
    }
    return res;
}

// запись в файл
void save(vector<pair<int, int>>& r) {
    ofstream f("output.json");
    f << "[\n";
    f << "  {\n";
    f << "    \"dispense\": [";
    
    for (size_t i = 0; i < r.size(); i++) {
        if (i > 0) f << ", ";
        f << "[" << r[i].first << ", " << r[i].second << "]";
    }
    
    f << "]\n";
    f << "  }\n";
    f << "]\n";
    f.close();
}

int main() {
    ifstream file("input.json");
    if (!file.is_open()) {
        vector<pair<int, int>> e;
        save(e);
        return 1;
    }
    
    string s;
    string line;
    while (getline(file, line)) {
        s += line;
    }
    file.close();
    
    string cleanJson = clean(s);
    
    vector<pair<int, int>> w = parseWallet(cleanJson);
    int need = parseAmount(cleanJson);
    string strat = parseStrategy(cleanJson);
    
    vector<pair<int, int>> res;
    
    if (strat == "MAX") {
        res = solveMAX(w, need);
    } else if (strat == "MIN") {
        res = solveMIN(w, need);
    } else if (strat == "UNIFORM") {
        res = solveUNIFORM(w, need);
    }
    
    save(res);
    
    return 0;
}
