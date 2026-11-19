#ifndef CAMION_H
#define CAMION_H
#include <QString>
#include <QMessageBox>
#include <QVariant>
#include <QMetaType>
#include <QSqlQuery>
#include <QSqlQueryModel>

#include <QSqlError>
#include <QPrinter>
#include <QPainter>
#include <QFileDialog>
#include <QtCharts/QChart>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QChartView>
#include <QVBoxLayout>
#include <QDialog>
#include <algorithm>
#include <QQuickItem>
#include <QObject>
#include <QQuickWidget>

#include <QGeoServiceProvider>
#include <QGeoCodingManager>
#include <QGeoCodeReply>
#include <QGeoLocation>
#include <QGeoAddress>
#include <QGeoCoordinate>

using namespace std;
class Camion{
private:
    //attributs
    int id_camion;
    QString matricule;
    float capacite_max;
    float remplissage;
    float consomation;
    QString status;
    bool camEditMode;
    bool trieAsc;
public:
    // Constructeurs
    Camion();
    Camion(QString,float,float,float,QString);

    // getters & setters
    int getId_camion()const{return id_camion;}
    QString getMatricule()const{return matricule;}
    float getCapacite_max()const{return capacite_max;}
    float getRemplissage()const{return remplissage;}
    float getConsomation()const{return consomation;}
    QString getStatus()const{return status;}
    bool getCamEditMode()const{return camEditMode;}
    bool getTrieAsc()const{return trieAsc;}
    void setId_camion(int id_camion){this->id_camion = id_camion;}
    void setMatricule(QString matricule){this->matricule = matricule;}
    void setCapacite_max(float capacite_max){this->capacite_max = capacite_max;}
    void setRemplissage(float remplissage){this->remplissage =remplissage;}
    void setConsomation(float consomation){this->consomation= consomation;}
    void setStatus(QString status){this->status = status;}
    void setCamEditMode(bool camEditMode){this->camEditMode = camEditMode;}
    void setTrieAsc(bool trieAsc){this->trieAsc = trieAsc;}

    // Fonction Crud
    bool addCamion()const;
    QSqlQueryModel * showCamion()const;
    bool editCamion()const;
    bool deleteCamion(QString)const;

    // chercher un Camion par matricule
    bool fetchCamionByMatricule(QString matricule);
    bool fetchCamionById(int id);
    bool camControleSaisie()const;

    // Metiers Simples
    QSqlQueryModel * camRechMat(QString Matricule)const;
    QSqlQueryModel * camTrierStatus(QString Matricule)const;
    void camExportPDF(QSqlQueryModel *model)const;
    void afficherStatistiques()const;

    /// Metier Avancé Carte
    void camMapZoom(QQuickWidget * mapWidget,bool i)const;
    void camMapCentrer(QQuickWidget* mapWidget)const;
    void camMapShowPC(QQuickWidget * mapWidget);
    void camConvSauvCoord(QString adresse);
    void camConvModifCoord(int id, QString adresse);

};

#endif // CAMION_H
