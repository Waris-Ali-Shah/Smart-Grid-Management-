#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <stdexcept>
using namespace std;
// --- CONSTANTS ---
const double RESIDENTIAL_RATE = 18.0;
const double INDUSTRIAL_RATE = 28.0;
const double INDUSTRIAL_FIXED = 500.0;
// --- UI HELPER ---
void pause() {
    cout << "\nPress Enter to Continue...";
    cin.ignore(10000, '\n');
    cin.get();
}
// --- 1. POWER STATION CLASSES ---
class PowerStation {
protected:
    int id; string name; double capacityMW; double currentOutputMW;
public:
    PowerStation(int id, string n, double cap) : id(id), name(n), capacityMW(cap), currentOutputMW(0) {
        if (cap <= 0) throw invalid_argument("Capacity must be greater than 0");
    }
    virtual ~PowerStation() {}
    virtual void generatePower() = 0;
    virtual void displayInfo() const {
        cout << left << setw(10) << id << setw(15) << getType() << setw(20) << name << setw(15) << capacityMW << "\n";
    }
    double getOutput() const { return currentOutputMW; }
    int getId() const { return id; }
    string getName() const { return name; }
    virtual string getType() const = 0;
    double getCapacity() const { return capacityMW; }
    void setCapacity(double cap) { if(cap>0) capacityMW = cap; }
};
class SolarPlant : public PowerStation {
private:
    double sunlightIntensity;
public:
    SolarPlant(int id, string n, double cap, double sunlight) : PowerStation(id, n, cap), sunlightIntensity(sunlight) {
        if (sunlight < 0.0 || sunlight > 1.0) throw invalid_argument("Sunlight must be 0.0-1.0");
    }
    void generatePower() override { currentOutputMW = capacityMW * sunlightIntensity; }
    string getType() const override { return "Solar"; }
};
class ThermalPlant : public PowerStation {
private:
    double fuelLevel;
public:
    ThermalPlant(int id, string n, double cap, double fuel) : PowerStation(id, n, cap), fuelLevel(fuel) {
        if (fuel < 0.0 || fuel > 1.0) throw invalid_argument("Fuel must be 0.0-1.0");
    }
    void generatePower() override { currentOutputMW = capacityMW * fuelLevel; }
    string getType() const override { return "Thermal"; }
};
// --- 2. CONSUMER CLASSES ---
class Consumer {
protected:
    int id; string name; double energyConsumedKWh; double currentLoadKW; int assignedTransformerId;
public:
    Consumer(int i, string n, double load) : id(i), name(n), energyConsumedKWh(0), currentLoadKW(load), assignedTransformerId(-1) {
        if (load <= 0) throw invalid_argument("Consumer load must be > 0");
    }
    virtual ~Consumer() {}
    virtual void displayInfo() const {
        string tId = (assignedTransformerId == -1) ? "None" : to_string(assignedTransformerId);
        cout << left << setw(10) << id << setw(15) << getType() << setw(20) << name << setw(15) << currentLoadKW << setw(15) << tId << "\n";
    }
    virtual double calculateBill() = 0;
    double getLoad() const { return currentLoadKW; }
    void setLoad(double l) { if(l>0) currentLoadKW = l; }
    void setAssignedTransformer(int tId) { assignedTransformerId = tId; }
    int getAssignedTransformer() const { return assignedTransformerId; }
    void consumeEnergy(double hours) { energyConsumedKWh += currentLoadKW * hours; }
    virtual string getType() const = 0;
    string getName() const { return name; }
    int getId() const { return id; }
};
class ResidentialConsumer : public Consumer {
public:
    ResidentialConsumer(int i, string n, double load) : Consumer(i, n, load) {}
    double calculateBill() override { return energyConsumedKWh * RESIDENTIAL_RATE; }
    string getType() const override { return "Residential"; }
};
class IndustrialConsumer : public Consumer {
public:
    IndustrialConsumer(int i, string n, double load) : Consumer(i, n, load) {}
    double calculateBill() override { return (energyConsumedKWh * INDUSTRIAL_RATE) + INDUSTRIAL_FIXED; }
    string getType() const override { return "Industrial"; }
};
// --- 3. TRANSFORMER CLASS ---
class Transformer {
private:
    int id; string location; double capacityKW; double currentLoadKW;
public:
    Transformer(int i, string loc, double cap) : id(i), location(loc), capacityKW(cap), currentLoadKW(0) {
        if (cap <= 0) throw invalid_argument("Capacity must be > 0");
    }
    ~Transformer() {}
    void addLoad(double load) { currentLoadKW += load; }
    void resetLoad() { currentLoadKW = 0; }
    string getStatus() const {
        if (capacityKW == 0) return "Healthy";
        double perc = (currentLoadKW / capacityKW) * 100.0;
        if (perc < 70.0) return "Healthy (<70%)";
        if (perc <= 89.99) return "Warning (70-89%)";
        if (perc <= 100.0) return "Critical (90-100%)";
        return "Overloaded (>100%)";
    }
    void displayInfo() const {
        cout << left << setw(10) << id << setw(20) << location << setw(15) << capacityKW << setw(15) << currentLoadKW << setw(20) << getStatus() << "\n";
    }
    bool isOverloaded() const { return currentLoadKW > capacityKW; }
    double getCapacity() const { return capacityKW; }
    void setCapacity(double cap) { if(cap>0) capacityKW = cap; }
    double getRemainingCapacity() const { return capacityKW - currentLoadKW; }
    double getLoad() const { return currentLoadKW; }
    int getId() const { return id; }
    string getLocation() const { return location; }
};
// --- 4. POWER GRID CLASS ---
class PowerGrid {
private:
    vector<PowerStation*> stations; vector<Consumer*> consumers; vector<Transformer*> transformers;
    double totalGeneratedPowerMW;
public:
    PowerGrid() : totalGeneratedPowerMW(0.0) {}
    ~PowerGrid() {
        for (auto p : stations) delete p;
        for (auto c : consumers) delete c;
        for (auto t : transformers) delete t;
    }
    bool isStationExists(int id) const { for(auto s : stations) if(s->getId() == id) return true; return false; }
    bool isConsumerExists(int id) const { for(auto c : consumers) if(c->getId() == id) return true; return false; }
    bool isTransformerExists(int id) const { for(auto t : transformers) if(t->getId() == id) return true; return false; }

    void addPowerStation(PowerStation* p) {
        if (isStationExists(p->getId())) { cout << "Error: ID " << p->getId() << " exists!\n"; delete p; return; }
        stations.push_back(p); cout << "Success: Power Station added!\n";
    }
    void registerConsumer(Consumer* c) {
        if (isConsumerExists(c->getId())) { cout << "Error: ID " << c->getId() << " exists!\n"; delete c; return; }
        consumers.push_back(c); cout << "Success: Consumer registered!\n";
    }
    void addTransformer(Transformer* t) {
        if (isTransformerExists(t->getId())) { cout << "Error: ID " << t->getId() << " exists!\n"; delete t; return; }
        transformers.push_back(t); cout << "Success: Transformer added!\n";
    }

    void viewPowerStations() const {
        if (stations.empty()) { cout << "No stations available.\n"; return; }
        cout << left << setw(10) << "ID" << setw(15) << "Type" << setw(20) << "Name" << setw(15) << "Capacity(MW)" << "\n";
        cout << "------------------------------------------------------------\n";
        for (auto s : stations) s->displayInfo();
    }
    void viewConsumers() const {
        if (consumers.empty()) { cout << "No consumers registered.\n"; return; }
        cout << left << setw(10) << "ID" << setw(15) << "Type" << setw(20) << "Name" << setw(15) << "Load(KW)" << setw(15) << "Trans ID" << "\n";
        cout << "---------------------------------------------------------------------------\n";
        for (auto c : consumers) c->displayInfo();
    }
    void viewTransformers() const {
        if (transformers.empty()) { cout << "No transformers available.\n"; return; }
        cout << left << setw(10) << "ID" << setw(20) << "Location" << setw(15) << "Cap(KW)" << setw(15) << "Load(KW)" << setw(20) << "Status" << "\n";
        cout << "--------------------------------------------------------------------------------\n";
        for (auto t : transformers) t->displayInfo();
    }

    void searchPowerStationProper(int id) const {
        for (auto s : stations) if (s->getId() == id) {
            cout << left << setw(10) << "ID" << setw(15) << "Type" << setw(20) << "Name" << setw(15) << "Capacity(MW)" << "\n";
            s->displayInfo(); return;
        }
        cout << "Error: Not found.\n";
    }
    void searchConsumerProper(int id) const {
        for (auto c : consumers) if (c->getId() == id) {
            cout << left << setw(10) << "ID" << setw(15) << "Type" << setw(20) << "Name" << setw(15) << "Load(KW)" << setw(15) << "Trans ID" << "\n";
            c->displayInfo(); return;
        }
        cout << "Error: Not found.\n";
    }
    void searchTransformerProper(int id) const {
        for (auto t : transformers) if (t->getId() == id) {
            cout << left << setw(10) << "ID" << setw(20) << "Location" << setw(15) << "Cap(KW)" << setw(15) << "Load(KW)" << setw(20) << "Status" << "\n";
            t->displayInfo(); return;
        }
        cout << "Error: Not found.\n";
    }

    void updateStationCapacity(int id, double newCap) {
        if(newCap<=0) { cout << "Error: Capacity > 0\n"; return; }
        for (auto s : stations) if (s->getId() == id) { s->setCapacity(newCap); cout << "Success: Updated.\n"; return; }
        cout << "Error: Not found.\n";
    }
    void updateConsumerLoad(int id, double newLoad) {
        if(newLoad<=0) { cout << "Error: Load > 0\n"; return; }
        for (auto c : consumers) if (c->getId() == id) { c->setLoad(newLoad); cout << "Success: Updated.\n"; return; }
        cout << "Error: Not found.\n";
    }
    void updateTransformerCapacity(int id, double newCap) {
        if(newCap<=0) { cout << "Error: Capacity > 0\n"; return; }
        for (auto t : transformers) if (t->getId() == id) { t->setCapacity(newCap); cout << "Success: Updated.\n"; return; }
        cout << "Error: Not found.\n";
    }

    void deleteStation(int id) {
        for (auto it = stations.begin(); it != stations.end(); ++it) {
            if ((*it)->getId() == id) { delete *it; stations.erase(it); cout << "Success: Deleted.\n"; return; }
        }
        cout << "Error: Not found.\n";
    }
    void deleteConsumer(int id) {
        for (auto it = consumers.begin(); it != consumers.end(); ++it) {
            if ((*it)->getId() == id) { delete *it; consumers.erase(it); cout << "Success: Deleted.\n"; return; }
        }
        cout << "Error: Not found.\n";
    }
    void deleteTransformer(int id) {
        for (auto it = transformers.begin(); it != transformers.end(); ++it) {
            if ((*it)->getId() == id) { delete *it; transformers.erase(it); cout << "Success: Deleted.\n"; return; }
        }
        cout << "Error: Not found.\n";
    }

    void generatePower() {
        totalGeneratedPowerMW = 0;
        for (auto p : stations) { p->generatePower(); totalGeneratedPowerMW += p->getOutput(); }
    }

    void monitorGrid() {
        generatePower();
        double totalDemandKW = 0;
        for(auto c : consumers) totalDemandKW += c->getLoad();
        double totalDemandMW = totalDemandKW / 1000.0;
        int overloadedCount = 0;
        for(auto t : transformers) if (t->isOverloaded()) overloadedCount++;

        string gridStatus = "NORMAL";
        if (totalDemandMW > totalGeneratedPowerMW) gridStatus = "CRITICAL (Shortage)";
        else if (overloadedCount > 0) gridStatus = "WARNING (Overloads)";
        cout << "\n========================================\n";
        cout << "        SMART GRID STATUS REPORT        \n";
        cout << "========================================\n";
        cout << left << setw(30) << "* Total Power Stations" << ": " << stations.size() << "\n";
        cout << left << setw(30) << "* Total Consumers" << ": " << consumers.size() << "\n";
        cout << left << setw(30) << "* Total Transformers" << ": " << transformers.size() << "\n";
        cout << left << setw(30) << "* Total Generated Power" << ": " << totalGeneratedPowerMW << " MW\n";
        cout << left << setw(30) << "* Total Consumer Demand" << ": " << totalDemandMW << " MW\n";
        if (totalDemandMW > totalGeneratedPowerMW)
            cout << left << setw(30) << "* Remaining Capacity" << ": 0 MW (Shortage: " << (totalDemandMW - totalGeneratedPowerMW) << " MW)\n";
        else
            cout << left << setw(30) << "* Remaining Capacity" << ": " << (totalGeneratedPowerMW - totalDemandMW) << " MW\n";
        cout << left << setw(30) << "* Overloaded Transformers" << ": " << overloadedCount << "\n";
        cout << left << setw(30) << "* Grid Status" << ": " << gridStatus << "\n";
        cout << "========================================\n";
    }
    void detectFaults() {
        generatePower();
        cout << "\n========================================\n";
        cout << "         FAULT DETECTION REPORT         \n";
        cout << "========================================\n";
        bool faultsFound = false;

        for (auto s : stations) {
            if (s->getOutput() == 0) {
                cout << "[FAULT] Station " << s->getId() << " (" << s->getName() << ") is offline or producing 0 MW.\n";
                faultsFound = true;
            }
        }

        for (auto t : transformers) {
            if (t->isOverloaded()) {
                cout << "[FAULT] Transformer " << t->getId() << " is overloaded (" << t->getLoad() << " KW / " << t->getCapacity() << " KW).\n";
                faultsFound = true;
            }
        }

        for (auto c : consumers) {
            if (c->getAssignedTransformer() == -1) {
                cout << "[FAULT] Consumer " << c->getId() << " (" << c->getName() << ") is NOT assigned to a transformer.\n";
                faultsFound = true;
            }
        }

        double totalDemandKW = 0;
        for (auto c : consumers) totalDemandKW += c->getLoad();
        double totalDemandMW = totalDemandKW / 1000.0;
        if (totalDemandMW > totalGeneratedPowerMW) {
            cout << "[CRITICAL] Grid power shortage of " << (totalDemandMW - totalGeneratedPowerMW) << " MW!\n";
            faultsFound = true;
        }

        if (!faultsFound) {
            cout << "No faults detected. The grid is operating flawlessly.\n";
        }
        cout << "========================================\n";
    }

    void dynamicLoadBalancing() {
        generatePower();
        if (transformers.empty() || consumers.empty()) { cout << "Error: Need transformers & consumers.\n"; return; }

        double totalDemandKW = 0;
        for (auto c : consumers) totalDemandKW += c->getLoad();
        double totalDemandMW = totalDemandKW / 1000.0;

        if (totalDemandMW > totalGeneratedPowerMW) {
            cout << "WARNING: Demand exceeds Generation! Power Shortage active.\n";
        }

        for (auto t : transformers) t->resetLoad();
        for (auto c : consumers) c->setAssignedTransformer(-1);

        int overloadedCount = 0;
        for (auto c : consumers) {
            Transformer* bestT = nullptr;
            double minLoad = 1e9;
            for (auto t : transformers) {
                if (t->getRemainingCapacity() >= c->getLoad() && t->getLoad() < minLoad) {
                    bestT = t; minLoad = t->getLoad();
                }
            }
            if (bestT) {
                bestT->addLoad(c->getLoad()); c->setAssignedTransformer(bestT->getId());
            } else {
                Transformer* fallbackT = transformers[0];
                for(auto t : transformers) if(t->getLoad() < fallbackT->getLoad()) fallbackT = t;
                fallbackT->addLoad(c->getLoad()); c->setAssignedTransformer(fallbackT->getId());
                cout << "WARNING: No space for Consumer " << c->getId() << ". Forced to Trans " << fallbackT->getId() << " (Overload!).\n";
            }
        }
        for (auto t : transformers) if (t->isOverloaded()) overloadedCount++;
        if (overloadedCount == 0) cout << "Success: Dynamic Load Balancing applied safely.\n";
        else cout << "WARNING: " << overloadedCount << " transformers overloaded after balancing.\n";
    }

    void generateBills() {
        if (consumers.empty()) { cout << "No consumers to bill.\n"; return; }
        cout << left << setw(10) << "ID" << setw(20) << "Name" << setw(15) << "Bill (Rs.)" << "\n";
        cout << "---------------------------------------------\n";
        for (auto c : consumers) {
            c->consumeEnergy(24);
            cout << left << setw(10) << c->getId() << setw(20) << c->getName() << setw(15) << fixed << setprecision(2) << c->calculateBill() << "\n";
        }
    }

    void saveData() const {
        ofstream out("grid_data.txt");
        if (!out) { cout << "Error writing file.\n"; return; }
        out << "===== POWER STATIONS =====\n" << stations.size() << "\n";
        for (auto s : stations) {
            out << "ID: " << s->getId() << "\nType: " << s->getType() << "\nName: " << s->getName() << "\nCapacity: " << s->getCapacity() << " MW\n==========================\n";
        }
        out << "===== CONSUMERS =====\n" << consumers.size() << "\n";
        for (auto c : consumers) {
            out << "ID: " << c->getId() << "\nType: " << c->getType() << "\nName: " << c->getName() << "\nLoad: " << c->getLoad() << " KW\n==========================\n";
        }
        out << "===== TRANSFORMERS =====\n" << transformers.size() << "\n";
        for (auto t : transformers) {
            out << "ID: " << t->getId() << "\nLocation: " << t->getLocation() << "\nCapacity: " << t->getCapacity() << " KW\n==========================\n";
        }
        cout << "Success: Data saved successfully to clean labeled format.\n";
    }

    void loadData() {
        ifstream in("grid_data.txt");
        if (!in) { cout << "No saved data found.\n"; return; }
        for(auto s: stations) delete s; stations.clear();
        for(auto c: consumers) delete c; consumers.clear();
        for(auto t: transformers) delete t; transformers.clear();

        string dummy; int num;
        getline(in, dummy);
        in >> num;
        for (int i=0; i<num; ++i) {
            int id; string type, name; double cap;
            in >> dummy >> id;
            in >> dummy >> type;
            in >> dummy >> name;
            in >> dummy >> cap >> dummy;
            in >> dummy;
            if(type=="Solar") stations.push_back(new SolarPlant(id, name, cap, 1.0));
            else stations.push_back(new ThermalPlant(id, name, cap, 1.0));
        }
        in >> dummy >> dummy >> dummy;
        in >> num;
        for (int i=0; i<num; ++i) {
            int id; string type, name; double load;
            in >> dummy >> id >> dummy >> type >> dummy >> name >> dummy >> load >> dummy >> dummy;
            if(type=="Residential") consumers.push_back(new ResidentialConsumer(id, name, load));
            else consumers.push_back(new IndustrialConsumer(id, name, load));
        }
        in >> dummy >> dummy >> dummy;
        in >> num;
        for (int i=0; i<num; ++i) {
            int id; string loc; double cap;
            in >> dummy >> id >> dummy >> loc >> dummy >> cap >> dummy >> dummy;
            transformers.push_back(new Transformer(id, loc, cap));
        }
        cout << "Success: Data loaded.\n";
    }
};

void showMenu() {
    cout << "\n========================================\n";
    cout << "  SMART POWER GRID MANAGEMENT SYSTEM  \n";
    cout << "========================================\n";
    cout << "1. Power Station Management (Add/View/Upd/Del)\n";
    cout << "2. Consumer Management (Add/View/Upd/Del)\n";
    cout << "3. Transformer Management (Add/View/Upd/Del)\n";
    cout << "4. Monitor Grid Status\n";
    cout << "5. Dynamic Load Balancing\n";
    cout << "6. Generate Bills\n";
    cout << "7. Fault Detection Report\n";
    cout << "8. Search Records\n";
    cout << "9. Save Data\n";
    cout << "10. Load Data\n";
    cout << "11. Exit\n";
    cout << "Enter your choice: ";
}

int main() {
    PowerGrid grid;
    int choice;
    do {
        showMenu();
        cin >> choice;
        if (cin.fail()) { cin.clear(); cin.ignore(10000, '\n'); cout << "Invalid input!\n"; pause(); continue; }

        switch (choice) {
            case 1: {
                int sub; cout << "1.Add 2.View 3.Update 4.Delete: "; cin >> sub;
                if (sub == 1) {
                    try {
                        int type, id; string name; double cap;
                        cout << "1.Solar 2.Thermal: "; cin >> type;
                        cout << "ID: "; cin >> id; cout << "Name: "; cin >> name; cout << "Capacity(MW): "; cin >> cap;
                        if (type == 1) { double sun; cout << "Sunlight(0-1): "; cin >> sun; grid.addPowerStation(new SolarPlant(id, name, cap, sun)); }
                        else if (type == 2) { double fuel; cout << "Fuel(0-1): "; cin >> fuel; grid.addPowerStation(new ThermalPlant(id, name, cap, fuel)); }
                    } catch(const exception& e) { cout << "Error: " << e.what() << "\n"; }
                } else if (sub == 2) grid.viewPowerStations();
                else if (sub == 3) { int id; double cap; cout << "ID: "; cin >> id; cout << "New Cap: "; cin >> cap; grid.updateStationCapacity(id, cap); }
                else if (sub == 4) { int id; cout << "ID: "; cin >> id; grid.deleteStation(id); }
                break;
            }
            case 2: {
                int sub; cout << "1.Add 2.View 3.Update 4.Delete: "; cin >> sub;
                if (sub == 1) {
                    try {
                        int type, id; string name; double load;
                        cout << "1.Res 2.Ind: "; cin >> type;
                        cout << "ID: "; cin >> id; cout << "Name: "; cin >> name; cout << "Load(KW): "; cin >> load;
                        if (type == 1) grid.registerConsumer(new ResidentialConsumer(id, name, load));
                        else if (type == 2) grid.registerConsumer(new IndustrialConsumer(id, name, load));
                    } catch(const exception& e) { cout << "Error: " << e.what() << "\n"; }
                } else if (sub == 2) grid.viewConsumers();
                else if (sub == 3) { int id; double load; cout << "ID: "; cin >> id; cout << "New Load: "; cin >> load; grid.updateConsumerLoad(id, load); }
                else if (sub == 4) { int id; cout << "ID: "; cin >> id; grid.deleteConsumer(id); }
                break;
            }
            case 3: {
                int sub; cout << "1.Add 2.View 3.Update 4.Delete: "; cin >> sub;
                if (sub == 1) {
                    try {
                        int id; string loc; double cap;
                        cout << "ID: "; cin >> id; cout << "Location: "; cin >> loc; cout << "Capacity(KW): "; cin >> cap;
                        grid.addTransformer(new Transformer(id, loc, cap));
                    } catch(const exception& e) { cout << "Error: " << e.what() << "\n"; }
                } else if (sub == 2) grid.viewTransformers();
                else if (sub == 3) { int id; double cap; cout << "ID: "; cin >> id; cout << "New Cap: "; cin >> cap; grid.updateTransformerCapacity(id, cap); }
                else if (sub == 4) { int id; cout << "ID: "; cin >> id; grid.deleteTransformer(id); }
                break;
            }
            case 4: grid.monitorGrid(); break;
            case 5: grid.dynamicLoadBalancing(); break;
            case 6: grid.generateBills(); break;
            case 7: grid.detectFaults(); break;
            case 8: {
                int sub, id; cout << "Search 1.Station 2.Consumer 3.Transformer: "; cin >> sub; cout << "ID: "; cin >> id;
                if(sub==1) grid.searchPowerStationProper(id); else if(sub==2) grid.searchConsumerProper(id); else if(sub==3) grid.searchTransformerProper(id);
                break;
            }
            case 9: grid.saveData(); break;
            case 10: grid.loadData(); break;
            case 11: cout << "Success: Exiting...\n"; break;
            default: cout << "Error: Invalid choice.\n";
        }
        if (choice != 11) pause();
    } while (choice != 11);
    return 0;
}
