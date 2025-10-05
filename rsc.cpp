// g++ -std=gnu++17 points_spacing.cpp -O2 && ./a.out
#include <bits/stdc++.h>
using namespace std;

// Part 1 - Define a data structure to hold a set of points. 
// How would you initialize this data structure? 
// What about file storage? 
// What if points need the ability to have additional attributes?


// Point - represents each point as an object that holds:
// Its location on the number line (a number, like 5.0).
// Optional attributes (label, timestamp, ect).
struct Point {
    // main attribute
    double x{};
    // optional attributes
    unordered_map<string, string> attrs;
};

// Pointset - A collection of points stored in an array.
struct PointSet {
    vector<Point> pts;

    // Initializer: PointSet ps{{ {1.0, {{"label","A"}}}, {2.5,{}} }};
    PointSet() = default;
    PointSet(initializer_list<Point> init) : pts(init) {}

    // Sort by coordinate
    void sortByX() {
        sort(pts.begin(), pts.end(), [](const Point& a, const Point& b){ return a.x < b.x; });
    }

    // Save to a simple CSV
    // first column is x, then optional k=v attributes.
    void saveCSV(const string& path) const {
        ofstream f(path);
        // check for ability for filing writing
        if (!f) {
            throw runtime_error("Cannot open file for writing: " + path);
        } 
        // loop over & write
        for (auto& p : pts) {
            f << std::setprecision(17) << p.x;
            for (auto& [k, v] : p.attrs) {
                f << "," << k << "=" << v;
            }
            f << "\n";
        }
    }

    // Load from the same CSV format.
    static PointSet loadCSV(const string& path) {
        ifstream f(path);
        // check for ability to read file
        if (!f) {
            throw runtime_error("Cannot open file for reading: " + path);
        }
        PointSet ps;
        string line;
        // loop over all lines and build Pointset
        while (getline(f, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string token;

            // First token is x (up to first comma or end)
            size_t comma = line.find(',');
            Point p;
            if (comma == string::npos) {
                p.x = stod(line);
            } else {
                p.x = stod(line.substr(0, comma));
                // Parse attributes as k=v pairs separated by commas
                string rest = line.substr(comma + 1);
                stringstream s2(rest);
                while (getline(s2, token, ',')) {
                    size_t eq = token.find('=');
                    if (eq != string::npos) {
                        string k = token.substr(0, eq);
                        string v = token.substr(eq + 1);
                        p.attrs[k] = v;
                    }
                }
            }
            ps.pts.push_back(std::move(p));
        }
        return ps;
    }
};

// Find if there's a value ~ target within eps using lower_bound on sorted array
static bool existsNear(const vector<double>& xs, size_t startIdx, double target, double eps) {
    auto it = lower_bound(xs.begin() + startIdx, xs.end(), target - eps);
    if (it == xs.end()) { 
        return false;
    }
    return fabs(*it - target) <= eps;
}

// Deduplicate very-close doubles by keeping the first in order
static vector<double> dedupClose(vector<double> xs, double eps) {
    if (xs.empty()) {
        return xs;
    } 
    sort(xs.begin(), xs.end());
    vector<double> out;
    out.reserve(xs.size());
    out.push_back(xs[0]);
    for (size_t i = 1; i < xs.size(); ++i) {
        if (fabs(xs[i] - out.back()) > eps) {
            out.push_back(xs[i]);
        }
    }
    return out;
}

// Part 2 - Min-distance filter 
// Keep only one point from any cluster of points closer than minDist.
// Strategy: sort points, then iterate and keep only those at least minDist apart.

PointSet filterByMinDistance(const PointSet& input, double minDist) {
    // Validate input
    if (minDist < 0) {
        throw invalid_argument("minDist must be non-negative");
    }

    PointSet out;

    // Edge case: empty input
    if (input.pts.empty()){
        return out;
    } 
    // Sort input by x
    vector<Point> v = input.pts;
    sort(v.begin(), v.end(), [](const Point& a, const Point& b){ return a.x < b.x; });

    // memory reserve
    out.pts.reserve(v.size());

    // always keep the first point
    out.pts.push_back(v[0]);

    // Iterate and keep points that are at least minDist apart from the last kept
    double lastKept = v[0].x;
    for (size_t i = 1; i < v.size(); ++i) {
        if (fabs(v[i].x - lastKept) >= minDist) {
            out.pts.push_back(v[i]);
            lastKept = v[i].x;
        }
        // else: skip this point; it's too close to the last kept
    }
    return out;
}

// Part 3 - Regular spacing detection
//
// Check whether there exist >= minCount points forming an arithmetic progression
// with given spacing 'd' (within a tolerance 'eps').
//
// Strategy:
// 1) Extract sorted unique coordinates (dedup by eps).
// 2) For each candidate start x[i], look for x[i] + d, x[i] + 2d, ... by binary search
//    within eps. Stop early if a step is missing. If count >= minCount: true.
//
bool hasRegularSpacing(const PointSet& input,
                       double spacing,
                       size_t minCount = 3,
                       double eps = 1e-6) {
    if (spacing <= 0.0 || minCount < 3 || input.pts.size() < minCount) return false;

    // Extract coordinates and dedup tiny differences
    vector<double> xs;
    xs.reserve(input.pts.size());
    for (auto& p : input.pts) xs.push_back(p.x);
    xs = dedupClose(xs, eps);
    if (xs.size() < minCount) return false;

    // For each possible start, try to walk forward by spacing
    for (size_t i = 0; i < xs.size(); ++i) {
        size_t count = 1;
        double nextVal = xs[i] + spacing;
        size_t searchFrom = i + 1;

        while (true) {
            if (!existsNear(xs, searchFrom, nextVal, eps)) break;
            // Found next step, bump count and predict the next:
            ++count;
            nextVal += spacing;
        }

        if (count >= minCount) return true;
    }
    return false;
}

// ===================== Demo / Example usage =====================

int main() {
    // Part 1 - Initialize
    PointSet ps{
        {0.05, {{"label","a"}}},
        {1.0,  {}},
        {2.01, {}},
        {3.0,  {}},
        {3.02, {}},
        {5.0,  {}},
        {5.01, {}},
        {7.0,  {}},
        {9.0,  {}}
    };

    // Part 2 - Filter points closer than 0.05 apart
    double minDist = 0.05; // treat anything closer than this as duplicates
    PointSet filtered = filterByMinDistance(ps, minDist);

    // Print filtered x's
    cout << "Filtered points (minDist=" << minDist << "):\n";
    for (auto& p : filtered.pts) cout << fixed << setprecision(3) << p.x << " ";
    cout << "\n";

    // Part 3 - Regular spacing checks
    // Check for spacing 2.0 among >=3 points (allow small epsilon due to float noise)
    double d = 2.0, eps = 1e-3;
    bool ok = hasRegularSpacing(filtered, d, /*minCount=*/3, eps);
    cout << "Has regular spacing d=" << d << " among >=3 points? "
         << (ok ? "YES" : "NO") << "\n";

    // Another check: spacing 4.0
    d = 4.0;
    ok = hasRegularSpacing(filtered, d, /*minCount=*/3, eps);
    cout << "Has regular spacing d=" << d << " among >=3 points? "
         << (ok ? "YES" : "NO") << "\n";

    // Notes on file storage:
    // filtered.saveCSV("points.csv");
    // auto loaded = PointSet::loadCSV("points.csv");

    return 0;
}
