#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <locale>
#include <sstream>

#include "patient.h"
#include "serialthread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    const bool IsNumeric(const std::string& str);

    std::vector<float> getNumberFromString(std::string s);

private:
    Ui::MainWindow *ui;

    ///---------NEW USER VARIABLES---------///

    //Current new patient
    Patient mNewPatient;

    //To handle serial I/O operations
    SerialThread* mSerialHandler = nullptr;
    QThread mSerialThread;
    std::string mPortCache;

    //Open/Close measurement
    bool isMeasuring = false;

    ///---------NEW USER METHODS---------///

    //Registration
    void RegistrationNextButtonFunc();
    void RegistrationInputClear();
    void RegistrationAssignCurrentPatientData();
    void RegistrationHide();
    void RegistrationShow();

    //Measurement
    void MeasurementNextButtonFunc();
    void MeasurementBackButtonFunc();
    void MeasurementStartToMeasure(); // Start / Stop button
    void MeasurementUpdatePatientFromPort(std::string str); //Updates patient data from incoming port data
    void MeasurementClearData();
    void MeasurementAssignCurrentPatientData();
    void MeasurementHide();
    void MeasurementShow();

    //Symptom
    void SymptomNextButtonFunc();
    void SymptomBackButtonFunc();
    void SymptomAssignCurrentPatientData(); // Update "Symptom" view with patient data
    void SymptomSelected(const QString&);
    void SymptomFillSymptomList();
    void SymptomAddButtonFunc();
    void SymptomDeleteButtonFunc();
    void SymptomHide();
    void SymptomShow();

    //Result
    void ResultAssignCurrentPatientData(); // Update "Result" view with patient data
    void ResultHide();
    void ResultShow();

    //General methods
    void NewUserHideAll();
    void NewUserShowAll();


    ///---------USER VIEW VARIABLES---------///
    std::vector<Patient> mPatientList;

    ///---------USER VIEW METHODS---------///

    void UpdateUserViewList();
    void UpdatePatientList();
    void PickPatient(int row);
    void UpdateViewResult(Patient patient);
    void ClearView();


    ///---------OTHER METHODS---------///
    void SaveToJsonFile();
    QJsonArray LoadFromJsonFile(std::string& filePath);
    std::vector<Patient> JsonArrayToPatient(QJsonArray& array);

    ///---------OTHER VARIABLES---------///
    std::string mFullPath;


};
#endif // MAINWINDOW_H
