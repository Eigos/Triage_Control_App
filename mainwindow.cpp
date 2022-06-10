#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    std::string currentPath = QCoreApplication::applicationDirPath().toStdString();
    std::string dbPath = currentPath + "/db/";
    std::string fileName = "data.json";
    std::string fullPath = dbPath + fileName;
    mFullPath = fullPath;

    ///New user

    NewUserHideAll();

    RegistrationShow();


    //Registration
    connect(ui->Button_NewUserNext, &QPushButton::released, this, &MainWindow::RegistrationNextButtonFunc);

    //Measurement
    const char* COM_PORT = "\\\\.\\COM5";
    const uint32_t BAUD = 115200;
    mSerialHandler = new SerialThread(COM_PORT, BAUD);

    connect(ui->Button_NewUser_Measurement_Next, &QPushButton::released, this, &MainWindow::MeasurementNextButtonFunc);
    connect(ui->Button_NewUser_Measurement_Back, &QPushButton::released, this, &MainWindow::MeasurementBackButtonFunc);
    connect(ui->Button_NewUser_Measurement_StartMeasure, &QPushButton::released, this, &MainWindow::MeasurementStartToMeasure);
    connect(mSerialHandler, &SerialThread::newData, this, &MainWindow::MeasurementUpdatePatientFromPort);

    //Symptom
    connect(ui->Button_NewUser_Symptom_Back, &QPushButton::released, this, &MainWindow::SymptomBackButtonFunc);
    connect(ui->Button_NewUser_Symptom_Next, &QPushButton::released, this, &MainWindow::SymptomNextButtonFunc);
    connect(ui->ComboBox_Symptom_List, &QComboBox::currentTextChanged, this, &MainWindow::SymptomSelected);
    connect(ui->Button_NewUser_Symptom_Add, &QPushButton::released, this, &MainWindow::SymptomAddButtonFunc);
    connect(ui->Button_NewUser_Symptom_Delete, &QPushButton::released, this, &MainWindow::SymptomDeleteButtonFunc);
    SymptomFillSymptomList();


    ///VIEW

    connect(ui->ListView_UserView_UserList, &QListWidget::currentRowChanged, this, &MainWindow::PickPatient);
    UpdateUserViewList();
}

MainWindow::~MainWindow()
{
    mSerialHandler->Stop();
    this->thread()->msleep(200);
    delete mSerialHandler;
    delete ui;
}

const bool MainWindow::IsNumeric(const std::string& str)
{

    for(uint32_t i = 0; i < str.size(); i++){
        if(!isdigit(str[i])){
            return false;
        }
    }

    return true;
}

std::vector<float> MainWindow::getNumberFromString(std::string s)
{

    std::vector<float> numbers;
    std::stringstream str_strm;
    str_strm << s; //convert the string s into stringstream
    std::string temp_str;
    float temp_float;
    while(!str_strm.eof()) {
        str_strm >> temp_str; //take words into temp_str one by one

        std::string numberStr;
        for(int i = 0;i < temp_str.length(); i++){
            if(std::isdigit(temp_str[i]) || temp_str[i] == '.'){
                numberStr.push_back(temp_str[i]);
            }
        }

        if(!numberStr.empty()){
            numbers.push_back(std::stof(numberStr));
        }

        temp_str = ""; //clear temp string
    }
    return {numbers};

}

void MainWindow::RegistrationNextButtonFunc()
{
    if(!IsNumeric(ui->TextEdit_NewUser_ID->toPlainText().toStdString())){
        qInfo() << "Unable to get registration ID";
        return;
    }

    std::string patientName = ui->TextEdit_NewUser_Name->toPlainText().toStdString();

    if(patientName.empty()){
        qInfo() << "Unable to get registration name";
        qInfo() << "Patient name cannot be empty";
        return;
    }

    mNewPatient.mID = std::stoi(ui->TextEdit_NewUser_ID->toPlainText().toStdString());
    mNewPatient.mName = patientName;

    RegistrationHide();
    MeasurementShow();
    ResultHide();
}

void MainWindow::RegistrationInputClear()
{
    ui->TextEdit_NewUser_ID->setText("");
    ui->TextEdit_NewUser_Name->setText("");
}

void MainWindow::RegistrationAssignCurrentPatientData()
{
    ui->TextEdit_NewUser_ID->setText(std::to_string(mNewPatient.mID).c_str());
    ui->TextEdit_NewUser_Name->setText(mNewPatient.mName.c_str());
}

void MainWindow::RegistrationHide()
{
    ui->GroupBox_NewUser_Registration->hide();
}

void MainWindow::RegistrationShow(){
    ui->GroupBox_NewUser_Registration->show();
}

void MainWindow::MeasurementNextButtonFunc()
{

    MeasurementHide();
    SymptomShow();
    //Update next page data
    SymptomAssignCurrentPatientData();

}

void MainWindow::MeasurementBackButtonFunc()
{
    RegistrationShow();
    MeasurementHide();
}

void MainWindow::MeasurementStartToMeasure()
{
    if(mSerialHandler->isRunning()){
        mSerialHandler->Stop();
        //disconnect(mSerialHandler);
    }
    else{

        mSerialHandler->start();
    }
}

void MainWindow::MeasurementUpdatePatientFromPort(std::string str)
{
    mPortCache.append(str);

    std::vector<std::string> keywords;
    keywords.push_back("Heart rate:");
    keywords.push_back("SpO2:");
    keywords.push_back("Temp C:");
    keywords.push_back("Blood Presure Data:");

    size_t pos = mPortCache.find("\n" , 1);

    while(pos != std::string::npos && !mPortCache.empty()){

        size_t pos = mPortCache.find("\n", 1);

        std::string subStr = mPortCache.substr(0, pos);

        mPortCache.erase(0, pos);

        for(int i = 0; i < keywords.size(); i++){
            pos = subStr.find(keywords[i]);


            if(pos != std::string::npos){

                switch (i) {
                //Heart rate
                case 0: {
                    std::vector<float> numbers = getNumberFromString(subStr);
                    if(numbers.size() != 0){
                        mNewPatient.mHeartRate = numbers[0];
                        qDebug() << "Heart Rate:" << numbers[0];
                    }
                    break;
                }

                    //SpO2
                case 1: {
                    std::vector<float> numbers = getNumberFromString(subStr.substr(subStr.find(":")));
                    if(numbers.size() != 0){
                        mNewPatient.mSpO2 = numbers[0];
                        qDebug() << "SpO2:" << numbers[0];
                    }
                    break;
                }

                    //Temp C
                case 2: {
                    std::vector<float> numbers = getNumberFromString(subStr.substr(subStr.find(":")));
                    if(numbers.size() != 0){
                        mNewPatient.mTemp = numbers[0];
                        qDebug() << "Temp C:" << numbers[0];
                    }
                    break;
                }

                    //Blood Pressure Data
                case 3:{

                    float val1;
                    float val2;
                    float val3;

                    size_t startPos = subStr.find("h") + 1;
                    size_t endPos = subStr.find("/", startPos);
                    std::vector<float> numbers;
                    numbers = getNumberFromString(subStr.substr(startPos, endPos-startPos));
                    if(numbers.size() != 0){
                        val1 = numbers[0];
                        mNewPatient.mPressureData.mHighPresure = val1;
                    }

                    startPos = endPos + 1;
                    endPos = subStr.find("/", startPos + 1);
                    numbers = getNumberFromString(subStr.substr(startPos, endPos-startPos));
                    if(numbers.size() != 0){
                        val2 = numbers[0];
                        mNewPatient.mPressureData.mLowPresure = val2;
                    }

                    startPos = endPos + 1;
                    numbers = getNumberFromString(subStr.substr(startPos));
                    if(numbers.size() != 0){
                        val3 = numbers[0];
                        mNewPatient.mPressureData.mHeartRate = val3;
                    }

                    qDebug() << "Pressure data -> Val1:" << val1 << "Val2" << val2 << "Val3" << val3;

                    break;
                }

                }

                break;
            }


        }


    }

    //Update Ui
    MeasurementAssignCurrentPatientData();
}

void MainWindow::MeasurementAssignCurrentPatientData()
{
    ui->Label_NewUser_Measurement_HeartRateResult->setText(std::to_string(mNewPatient.mHeartRate).c_str());
    ui->Label_NewUser_Measurement_SPO2Result->setText(std::to_string(mNewPatient.mSpO2).c_str());
    ui->Label_NewUser_Measurement_TemperatureResult->setText(std::to_string(mNewPatient.mTemp).c_str());
    ui->Label_NewUser_Measurement_BloodPressureResult->setText(mNewPatient.mPressureData.getString().c_str());
}

void MainWindow::MeasurementHide()
{
    ui->GroupBox_NewUser_Measurement->hide();
}

void MainWindow::MeasurementShow()
{
    ui->GroupBox_NewUser_Measurement->show();
}

void MainWindow::SymptomNextButtonFunc()
{
    std::vector<std::string> symptomList;

    for(uint32_t i= 0; i < ui->ListWidget_NewUser_Symptom_List->count(); i++){
        symptomList.push_back(ui->ListWidget_NewUser_Symptom_List->item(i)->text().toStdString());
    }

    bool red = false;
    bool yellow = false;
    bool green = false;

    for(uint32_t i = 0; i < symptomList.size(); i++){
        if(symptomList[i].find("RED") != std::string::npos){
            red = true;
            break;
        }
        else if(symptomList[i].find("Yellow") != std::string::npos){
            yellow = true;
        }else if(symptomList[i].find("Green") != std::string::npos){
            green = true;
        }
    }

    if(red == true){
        mNewPatient.mSymptom = "RED";
    }else if(yellow){
        mNewPatient.mSymptom = "YELLOW";
    }else if(green){
        mNewPatient.mSymptom = "GREEN";
    }else{
        mNewPatient.mSymptom = "NoSymptom";
    }

    uint32_t listCount = ui->ListWidget_NewUser_Symptom_List->count();

    for(uint32_t i = 0; i < listCount; i++){
        ui->ListWidget_NewUser_Symptom_List->setCurrentRow(0);
        QListWidgetItem *it = ui->ListWidget_NewUser_Symptom_List->takeItem(ui->ListWidget_NewUser_Symptom_List->currentRow());
        delete it;
    }

    ResultAssignCurrentPatientData();
    SaveToJsonFile();
    UpdateUserViewList();

    SymptomHide();
    RegistrationShow();
    ResultShow();
}

void MainWindow::SymptomBackButtonFunc()
{
    SymptomHide();
    MeasurementShow();
}

void MainWindow::SymptomAssignCurrentPatientData()
{
    ui->Label_NewUser_Symptom_HeartRateResult->setText(std::to_string(mNewPatient.mHeartRate).c_str());
    ui->Label_NewUser_Symptom_SPO2Result->setText(std::to_string(mNewPatient.mSpO2).c_str());
    ui->Label_NewUser_Symptom_TemperatureResult->setText(std::to_string(mNewPatient.mTemp).c_str());
    ui->Label_NewUser_Symptom_BloodPressureResult->setText(mNewPatient.mPressureData.getString().c_str());
}

void MainWindow::SymptomSelected(const QString& newSymptom)
{
    mNewPatient.mSymptom = newSymptom.toStdString();
}

void MainWindow::SymptomFillSymptomList()
{
    ui->ComboBox_Symptom_List->addItem("RED - AMBULANCE");
    ui->ComboBox_Symptom_List->addItem("RED - STRETCHER");
    ui->ComboBox_Symptom_List->addItem("RED - SEVERE CHEST PAIN");
    ui->ComboBox_Symptom_List->addItem("RED - ELECTRIC SHOCK");
    ui->ComboBox_Symptom_List->addItem("RED - PATIENT WITH CLOSED CONSCIOUSNESS");
    ui->ComboBox_Symptom_List->addItem("RED - SERIOUS RESPIRATORY DIFFICULTY");
    ui->ComboBox_Symptom_List->addItem("RED - EPILEPSY CRISIS");
    ui->ComboBox_Symptom_List->addItem("RED - AIRWAY OBSTRUCTION");
    ui->ComboBox_Symptom_List->addItem("RED - ACUTE LOSS OF POWER, PARLIAMENT");
    ui->ComboBox_Symptom_List->addItem("RED - FEVER AND SLEEP");
    ui->ComboBox_Symptom_List->addItem("RED - VIOLENT BEHAVIORS");
    ui->ComboBox_Symptom_List->addItem("RED - DRUG USE");
    ui->ComboBox_Symptom_List->addItem("RED - GENERAL STATUS DISORDER");

    ui->ComboBox_Symptom_List->addItem("Yellow - SERIOUS HYPERTENSION");
    ui->ComboBox_Symptom_List->addItem("Yellow - CHEST PAIN");
    ui->ComboBox_Symptom_List->addItem("Yellow - SHORTNESS OF BREATH ");
    ui->ComboBox_Symptom_List->addItem("Yellow - HYPERGLYCEMIA");
    ui->ComboBox_Symptom_List->addItem("Yellow - HYPOGLYCEMIA");
    ui->ComboBox_Symptom_List->addItem("Yellow - BLOODY VOMITING");
    ui->ComboBox_Symptom_List->addItem("Yellow - BLACK STOCKS");
    ui->ComboBox_Symptom_List->addItem("Yellow - KIDNEY FAILURE");
    ui->ComboBox_Symptom_List->addItem("Yellow - HYPOTENSION");
    ui->ComboBox_Symptom_List->addItem("Yellow - FLUSH-RHYTHM DISORDER");
    ui->ComboBox_Symptom_List->addItem("Yellow - STOVE POISONING FROM FOOT");
    ui->ComboBox_Symptom_List->addItem("Yellow - NOSE BLEEDING");
    ui->ComboBox_Symptom_List->addItem("Yellow - PATIENT WITH STRESS AND RISK OF DAMAGE");
    ui->ComboBox_Symptom_List->addItem("Yellow - severe abdominal pain");
    ui->ComboBox_Symptom_List->addItem("Yellow - RESPIRATORY DIFFICULTY");
    ui->ComboBox_Symptom_List->addItem("Yellow - BLEEDING PATIENT USING KUMADIN");
    ui->ComboBox_Symptom_List->addItem("Yellow - HEADACHE(SERIOUS)");
    ui->ComboBox_Symptom_List->addItem("Yellow - ASTHMA, BRONCHITIS CRISIS");

    ui->ComboBox_Symptom_List->addItem("Green - HEADACHE");
    ui->ComboBox_Symptom_List->addItem("Green - FLU, WEAKNESS");
    ui->ComboBox_Symptom_List->addItem("Green - nausea-vomiting");
    ui->ComboBox_Symptom_List->addItem("Green - COLD, COLD");
    ui->ComboBox_Symptom_List->addItem("Green - DIARRHEA");
    ui->ComboBox_Symptom_List->addItem("Green - ALLERGY");
    ui->ComboBox_Symptom_List->addItem("Green - itch, rash");
    ui->ComboBox_Symptom_List->addItem("Green - LEG SHALLOW (CHRONIC)");
    ui->ComboBox_Symptom_List->addItem("Green - GENERAL BODY PAIN");
    ui->ComboBox_Symptom_List->addItem("Green - BURN IN URINE");
    ui->ComboBox_Symptom_List->addItem("Green - FREQUENT URINATION");
    ui->ComboBox_Symptom_List->addItem("Green - HIGH FEVER");
    ui->ComboBox_Symptom_List->addItem("Green - COUGH");
    ui->ComboBox_Symptom_List->addItem("Green - NON-TRAAUMA JOINT PAIN ");
    ui->ComboBox_Symptom_List->addItem("Green - INJECTION");
    ui->ComboBox_Symptom_List->addItem("Green - SORROW SEPARATION");
    ui->ComboBox_Symptom_List->addItem("Green - BACKACHE");
    ui->ComboBox_Symptom_List->addItem("Green - HEADACHE(MILF)");
    ui->ComboBox_Symptom_List->addItem("Green - EYE BURRING");
    ui->ComboBox_Symptom_List->addItem("Green - EYES ITCH-RED");
    ui->ComboBox_Symptom_List->addItem("Green - EARACHE");
    ui->ComboBox_Symptom_List->addItem("Green - TENSION HEIGHT");
    ui->ComboBox_Symptom_List->addItem("Green - WHAT AĞRISI");

}

void MainWindow::SymptomAddButtonFunc()
{
    ui->ListWidget_NewUser_Symptom_List->addItem(ui->ComboBox_Symptom_List->currentText());
}

void MainWindow::SymptomDeleteButtonFunc()
{
    QListWidgetItem *it = ui->ListWidget_NewUser_Symptom_List->takeItem(ui->ListWidget_NewUser_Symptom_List->currentRow());
    delete it;
}

void MainWindow::SymptomHide()
{
    ui->GroupBox_NewUser_Symptom->hide();
}

void MainWindow::SymptomShow()
{
    ui->GroupBox_NewUser_Symptom->show();
}

void MainWindow::NewUserHideAll()
{
    RegistrationHide();
    MeasurementHide();
    SymptomHide();
    ResultHide();
}

void MainWindow::NewUserShowAll()
{
    RegistrationShow();
    MeasurementShow();
    SymptomShow();
    ResultShow();
}

void MainWindow::ResultAssignCurrentPatientData()
{
    ui->Label_NewUser_Result_HeartRateResult->setText(std::to_string(mNewPatient.mHeartRate).c_str());
    ui->Label_NewUser_Result_SPO2Result->setText(std::to_string(mNewPatient.mSpO2).c_str());
    ui->Label_NewUser_Result_TemperatureResult->setText(std::to_string(mNewPatient.mTemp).c_str());
    ui->Label_NewUser_Result_BloodPressureResult->setText(mNewPatient.mPressureData.getString().c_str());
    ui->Label_NewUser_Result_SymptomResult->setText(mNewPatient.mSymptom.c_str());
}

void MainWindow::ResultHide()
{
    ui->GroupBox_NewUser_Result->hide();
}

void MainWindow::ResultShow()
{
    ui->GroupBox_NewUser_Result->show();
}

void MainWindow::UpdateUserViewList()
{
    UpdatePatientList();
    ClearView();

    for (Patient patient : mPatientList) {
        ui->ListView_UserView_UserList->addItem(patient.mName.c_str());
    }
}

void MainWindow::UpdatePatientList()
{
    if(!QFile(mFullPath.c_str()).exists()){
        qDebug() << "File does not exist!";
        return;
    }

    auto x = LoadFromJsonFile(mFullPath);
    mPatientList = JsonArrayToPatient(x);
}

void MainWindow::PickPatient(int row)
{
    if(mPatientList.size() < row){
        return;
    }

    Patient selectedPatient = mPatientList[row];

    UpdateViewResult(selectedPatient);
}

void MainWindow::UpdateViewResult(Patient patient)
{

    ui->Label_UserView_NameResult->setText(patient.mName.c_str());
    ui->Label_UserView_IDResult->setText(std::to_string(patient.mID).c_str());
    ui->Label_UserView_HeartRateResult->setText(std::to_string(patient.mHeartRate).c_str());
    ui->Label_UserView_SPO2Result->setText(std::to_string(patient.mSpO2).c_str());
    ui->Label_UserView_TemperatureResult->setText(std::to_string(patient.mTemp).c_str());
    ui->Label_UserView_BloodPressureResult->setText(patient.mPressureData.getString().c_str());
    ui->Label_UserView_SymptomResult->setText(patient.mSymptom.c_str());
}

void MainWindow::ClearView()
{
    while(ui->ListView_UserView_UserList->item(0) != nullptr){
        ui->ListView_UserView_UserList->takeItem(0);
    }

}

void MainWindow::SaveToJsonFile()
{

    std::string currentPath = QCoreApplication::applicationDirPath().toStdString();
    std::string dbPath = currentPath + "/db/";
    std::string fileName = "data.json";
    std::string fullPath = dbPath + fileName;

    QJsonObject jsonPatientObject;

    jsonPatientObject.insert("ID", mNewPatient.mID);
    jsonPatientObject.insert("Name", mNewPatient.mName.c_str());
    jsonPatientObject.insert("HeartRate", mNewPatient.mHeartRate);
    jsonPatientObject.insert("SpO2", mNewPatient.mSpO2);
    jsonPatientObject.insert("Temp", mNewPatient.mTemp);
    jsonPatientObject.insert("Symptom", mNewPatient.mSymptom.c_str());

    QJsonObject jsonPatitentPressureData;
    jsonPatitentPressureData.insert("HeartRate", mNewPatient.mPressureData.mHeartRate);
    jsonPatitentPressureData.insert("LowPressure", mNewPatient.mPressureData.mLowPresure);
    jsonPatitentPressureData.insert("HighPressure", mNewPatient.mPressureData.mHighPresure);

    jsonPatientObject.insert("Pressure", jsonPatitentPressureData);

    QJsonObject obj;

    QJsonArray existingFile = LoadFromJsonFile(fullPath);

    existingFile.append(jsonPatientObject);

    obj.insert("PatientData", existingFile);

    QJsonDocument document;
    document.setObject(obj);
    QByteArray bytes = document.toJson( QJsonDocument::Indented );

    if(!QDir(dbPath.c_str()).exists()){
        QDir(dbPath.c_str()).mkpath(dbPath.c_str());
    }

    QFile file(fullPath.c_str());

    if( file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate) )
    {
        QTextStream iStream( &file );
        //iStream.setCodec( "utf-8" );
        iStream << bytes;
        file.close();
    }
    else
    {
        qDebug() << "file open failed: " << fullPath.c_str();
    }

}


QJsonArray MainWindow::LoadFromJsonFile(std::string& filePath)
{
    QFile file(filePath.c_str());

    if(!file.open(QIODevice::ReadOnly)) {
        qDebug() << file.errorString();


        return QJsonArray();
    }

    QByteArray fileBytes = file.readAll();

    QJsonParseError jsonError;

    QJsonDocument jsonDocument = QJsonDocument::fromJson(fileBytes, &jsonError);

    if(jsonError.error != QJsonParseError::NoError){
        qDebug() << "fromJson failed: " << jsonError.errorString();

        file.close();
        return QJsonArray();
    }

    if(!jsonDocument.object().contains("PatientData")){
        qDebug() << "json file does not contain 'PatientData'! ";

        file.close();
        return QJsonArray();
    }

    file.close();

    return jsonDocument.object().value("PatientData").toArray();
}

std::vector<Patient> MainWindow::JsonArrayToPatient(QJsonArray& array)
{
    std::vector<Patient> patientList;

    for(uint64_t i = 0; i < array.size(); i++){
        QJsonObject patientObject = array[i].toObject();

        Patient patient;

        if(patientObject.contains("Name")){
            patient.mName = patientObject.value("Name").toString().toStdString().c_str();
        }
        if(patientObject.contains("ID")){
            patient.mID = patientObject.value("ID").toInt();
        }
        if(patientObject.contains("HeartRate")){
            patient.mHeartRate = patientObject.value("HeartRate").toDouble();
        }
        if(patientObject.contains("SpO2")){
            patient.mSpO2 = patientObject.value("SpO2").toDouble();
        }
        if(patientObject.contains("Temp")){
            patient.mTemp = patientObject.value("Temp").toDouble();
        }

        if(patientObject.contains("Symptom")){
            patient.mSymptom = patientObject.value("Symptom").toString().toStdString();
        }

        if(patientObject.contains("Pressure")){
            QJsonObject pressureObject = patientObject.value("Pressure").toObject();

            if(pressureObject.contains("HeartRate")){
                patient.mPressureData.mHeartRate = pressureObject.value("HeartRate").toDouble();
            }
            if(pressureObject.contains("HighPressure")){
                patient.mPressureData.mHighPresure = pressureObject.value("HighPressure").toDouble();
            }
            if(pressureObject.contains("LowPressure")){
                patient.mPressureData.mLowPresure = pressureObject.value("LowPressure").toDouble();
            }

        }

        patientList.push_back(patient);
    }

    return {patientList};
}





















